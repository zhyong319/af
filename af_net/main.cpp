#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<libgen.h>
#include<string.h>
#include<signal.h>
#include<time.h>
#include<sys/types.h>
#include<sys/stat.h>
#include"mail.h"
#include"rconf.h"
#include"socknet.h"
#include"worksys.h"
#include"mytype.h"
#include"sql.h"

#define TIMELOOP 10

char mailTo[10][50];
char webURL[3][255];
char gatewayip[16];
char sendtime[10];

void proc_handle(int)
{
	alarm(600);
	signal(SIGALRM, proc_handle);
	int step = 0;
	int netflag; //�����־����ӦΪ��
	int sockfd;
	int timeout;
	//  7			6			5			4				3				2			1			0				
	// hwflag    gateway     internet1   internet2		internet3			X	        X           X
	while (step < 5)
	{
		switch (step)
		{
		case 0://���Ӳ�����ӳ���
			netflag = 0; //�����־λ
			if (IsHardWareOK())
			{
				step++;
				netflag += 1 << 7; //д����ر�־
			}
			else //Ӳ��δ����׼��
			{
				netflag &= ~(1 << 7);
				step = 5; //�������
				//////ֱ��д��SQL����
				mysql_wralog((char*)"HW not ready", (char*)(""), (char*)(""));
			}
			break;


		case 1: //�������״̬����
			sockfd = 0;
			timeout = 0;
			sockfd = ConnectHost(gatewayip);
			if (sockfd < 0) //����ʧ�ܣ�ֱ�����ݿ�д���������,
			{
				netflag &= ~(1 << 6);
				printf("Action:Check local net please!\n");
				step = 5; //�������
				//�������ӳ���д��SQL���������Ϣ
				mysql_wralog((char*)"Y", (char*)("GateWay Connect failse"), (char*)(""));
				//sleep(TIMELOOP);
				break;
			}
			timeout = GetHttpRequest(sockfd, gatewayip);
			if (timeout >= 0)	//��������������NETFLAG��־λ������ת����һ���������
			{
				step++;
				netflag += 1 << 6;
				break;
			}
			else// ����ֱ��д��SQL���ݣ����������Ϣ 
			{
				step = 5; //�������
				//sleep(TIMELOOP);
				mysql_wralog((char*)"Y", (char*)("GateWay NO-response"), (char*)(""));
			}
			break;

		case 2://����һ������WEB����������
			sockfd = 0;
			timeout = 0;
			sockfd = ConnectHost(webURL[0]);
			if (sockfd < 0) //����ʧ��,���ı�־λ������ڶ�WEB���������
			{
				printf("Action: Connect host %s error!\n", webURL[0]);
				netflag &= ~(1 << 5);
				step++; //�����Լ�,����ڶ����������
				break;
			}
			timeout = GetHttpRequest(sockfd, webURL[0]);
			if (timeout >= 0)	//��������������NETFLAG��־λ��
			{
				step = 5; //�������
				netflag += 1 << 5;
				//printf("web1 status:  OK\n");
				//��������������NETFLAG��־λ�����ݿ�д���������
				mysql_wralog((char*)"Y", (char*)("Y"), (char*)("Y"));
				//sleep(TIMELOOP);
			}
			else// ����ת����һ���裬�������web������
			{
				step = 3;
				netflag &= ~(1 << 5);
			}
			break;

		case 3://���ڶ�������WEB����������
			sockfd = 0;
			timeout = 0;
			sockfd = ConnectHost(webURL[1]);
			if (sockfd < 0) //����ʧ��,���ı�־λ���������WEB���������
			{
				printf("Action: Connect host %s error!\n", webURL[1]);
				netflag &= ~(1 << 4);
				step++; //�����Լ�,����������������
				break;
			}
			timeout = GetHttpRequest(sockfd, webURL[1]);
			if (timeout >= 0)	//��������������NETFLAG��־λ��
			{
				step = 5; //�������
				netflag += 1 << 4;
				//printf("web2 status:  OK\n");
				//��������������NETFLAG��־λ�����ݿ�д���������
				mysql_wralog((char*)"Y", (char*)("Y"), (char*)("Y"));
				//sleep(TIMELOOP);
			}
			else// ����ת����һ���裬�������web������
			{
				step = 3;
				netflag &= ~(1 << 4);
			}

			break;

		case 4: //������������WEB����������
			sockfd = 0;
			timeout = 0;
			sockfd = ConnectHost(webURL[2]);
			if (sockfd < 0) //����ʧ��,���ı�־λ����λ���裬д������SQL����
			{
				step = 5; //�������
				printf("Action: Connect host %s error!\n", webURL[2]);
				netflag &= ~(1 << 3);
				//�쳣��д��SQL�����������Ϣ
				mysql_wralog((char*)"Y", (char*)("Y"), (char*)("Web server NOT connect"));
				break;
			}
			timeout = GetHttpRequest(sockfd, webURL[2]);
			if (timeout >= 0)	//��������������NETFLAG��־λ��
			{
				step = 5; //�������
				netflag += 1 << 4;
				//printf("web3 status:  OK\n");
				//��������������NETFLAG��־λ�����ݿ�д���������
				mysql_wralog((char*)"Y", (char*)("Y"), (char*)("Y"));

			}
			else// ����ת������0���������
			{
				step = 5; //�������
				netflag &= ~(1 << 3);
				//�����쳣������NETFLAG��־λ�����ݿ�д���������
				mysql_wralog((char*)"Y", (char*)("Y"), (char*)("Web server NO-response"));
				//sleep(TIMELOOP);
			}
			break;

		default:
			break;
		}

	}
}

int loadconf(char* currentworkpath)
{
	int reFlag = ChangeWorkPath(currentworkpath);//�л�����Ŀ¼
	if (reFlag != 0)
	{
		printf("WARNING: Change work directory failse!\n");
		return -1;
	}
	int fileStatus = IsFileStatusOK((char*)"./af.conf");//����ļ���дȨ���Ƿ�����
	if (fileStatus != 0)// { daemon(1, 0); }
	{
		printf("WARNING: file \'af.conf\' not exsit, please check file exsit and can be read!\n");
		exit(-1);
	}
	int status = 0;
	memset(gatewayip, 0, sizeof(gatewayip));
	status = GetProfileString((char*)"./af.conf", (char*)"gateway", (char*)"ip", gatewayip);//��ȡ����IP��ַ
	if (status == -1) printf("Action: Read af.conf gateway filed has a error! Please check it and try again!\n");

	memset(sendtime, 0, sizeof(sendtime));
	status = GetProfileString((char*)"./af.conf", (char*)"sendreport", (char*)"time", sendtime);//��ȡ����IP��ַ
	if (status == -1) printf("Action: Read af.conf sendreport filed has a error! Please check it and try again!\n");

	for (int ii = 0; ii < 3; ii++) //��ȡwebURL��ַ
	{
		char urlstrtmp[10];
		memset(urlstrtmp, 0, sizeof(urlstrtmp));
		sprintf(urlstrtmp, "url%d", ii);
		memset(webURL[ii], 0, sizeof(webURL[ii]));
		status = GetProfileString((char*)"./af.conf", (char*)"webURL", urlstrtmp, webURL[ii]);
		if (status == -1) printf("Action: Read af.conf webURL filed has a error! Please check it and try again!\n");
	}


	for (int ii = 0; ii < 10; ii++)//��ȡ�ʼ���ַ
	{
		char mailstrtmp[10];
		memset(mailstrtmp, 0, sizeof(mailstrtmp));
		sprintf(mailstrtmp, "mail%d", ii);
		memset(mailTo[ii], 0, sizeof(mailTo[ii]));
		status = GetProfileString((char*)"./af.conf", (char*)"mail", mailstrtmp, mailTo[ii]);
		if (status == -1) printf("Action: Read af.conf mail filed has a error! Please check it and try again!\n");
	}


	return 0;
}

void sendreport(void) //ͨ��MAIL���Ͳ�ѯ���,�������ʼ�����
{
	int localbreak_t = GetTodayLocalBreakCount();
	int netbreak_t = GetTodayInternetBreakCount();
	int total_t = 0;//��
	if (netbreak_t > localbreak_t) total_t = netbreak_t*10;
	else total_t = localbreak_t * 10;
	time_t timer;
	struct tm* ptm;
	time(&timer);
	ptm = localtime(&timer);
	char subject[255];
	char context[4096];
	memset(subject, 0, sizeof(subject));
	memset(context, 0, sizeof(context));
	sprintf(subject, "�Ƶ��������������������(%d��%d��%d��)", ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday);
	sprintf(context, "��ֹ���ʼ�����ʱ�䣬������������ %d �Σ��������� %d �Σ��ж�ʱ���ۼ� %d ���ӡ�\r\n\
+---------------------------------------------------------------------------------------+\r\n\
��ʾ��\r\n\
1.�������Ubuntu + MYSQLƽ̨\r\n\
2.���β�ѯ�����ϵͳ�Զ����ɲ����ͣ��������ʻ��ȡ������ϸ��¼����ظ��ʼ�����������ϵ��18981573198)\r\n\
3.�����ƣ�ÿ10������ѯһ�Σ�������ݰ�����Ӳ���Ƿ��������������������Ƿ����������������Ƿ���������������ϸ�ļ������¼��MYSQL���Ա���\r\n\
4.����������Է������˵Ĺ�����·---->��ͨDNS---->��������---->163��baidu��sohu��ҳ���������������������ȫ��������˺��Ƶ�����HTTH��������Ƿ�Ҳ��ˣ���֪����\r\n\
+---------------------------------------------------------------------------------------+", localbreak_t, netbreak_t, total_t);
	
	for (int ii = 0; ii < 10; ii++)
	{
		if(strlen(mailTo[ii]) !=0 ) //����ʼ���ַ��Ϊ�գ�����õ�ַ�����ʼ�
		{
			int mailstatus = writemailto((char*)"58180698@qq.com", mailTo[ii], subject, context);
			if (mailstatus < 0) printf("Action: send mail to $s failse!\n", mailTo[ii]);
			sleep(5);

		}
	}
}

int main(int argc, char* argv[])
{
	
	loadconf(argv[0]);
	if (daemon(1, 0) == -1)
	{
		printf("�����ػ�����ʧ�ܣ�����\n");
		writelog2file("�����ػ�����ʧ�ܣ�����\n");
		exit(-1); //�����ػ�����ʧ�ܣ�
	}
	printf("Action:	Tracer process is running ......(if has any error infomation can be display on this tty)\n");
	writelog2file((char*)"Action:	Tracer process is running ......(if has any error infomation can be display on this tty)\n");
	alarm(5);
	signal(SIGALRM, proc_handle);
	while (1)
	{
		char strtime[10];
		time_t timer;
		struct tm* ptm;
		time(&timer);
		ptm = localtime(&timer);
		strftime(strtime, 10, "%H:%M:%S", ptm);//��ʽ��ʱ���ַ���
		if (strcmp(strtime, sendtime) == 0)
		{
			int localbreak_t = GetTodayLocalBreakCount();
			int netbreak_t = GetTodayInternetBreakCount();
			if ((localbreak_t > 0) || (netbreak_t > 0)) //�������жϼ�¼�ŷ����ʼ���
			{
				sendreport();
				printf("\n\n\nAction: Report was send with mail!\n");
				writelog2file("%s Action: Report was send with mail!\n");
				sleep(60);
				
			}
		}
		sleep(1);
	}
	
	return 0;
}