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
	int netflag; //网络标志（对应为）
	int sockfd;
	int timeout;
	//  7			6			5			4				3				2			1			0				
	// hwflag    gateway     internet1   internet2		internet3			X	        X           X
	while (step < 5)
	{
		switch (step)
		{
		case 0://检查硬件连接程序
			netflag = 0; //清零标志位
			if (IsHardWareOK())
			{
				step++;
				netflag += 1 << 7; //写入相关标志
			}
			else //硬件未做好准备
			{
				netflag &= ~(1 << 7);
				step = 5; //结束检查
				//////直接写入SQL数据
				mysql_wralog((char*)"HW not ready", (char*)(""), (char*)(""));
			}
			break;


		case 1: //检查网关状态程序
			sockfd = 0;
			timeout = 0;
			sockfd = ConnectHost(gatewayip);
			if (sockfd < 0) //连接失败，直接数据库写入相关数据,
			{
				netflag &= ~(1 << 6);
				printf("Action:Check local net please!\n");
				step = 5; //结束检查
				//网关连接出错，写入SQL报告故障信息
				mysql_wralog((char*)"Y", (char*)("GateWay Connect failse"), (char*)(""));
				//sleep(TIMELOOP);
				break;
			}
			timeout = GetHttpRequest(sockfd, gatewayip);
			if (timeout >= 0)	//网络正常，更改NETFLAG标志位，继续转到下一步继续检查
			{
				step++;
				netflag += 1 << 6;
				break;
			}
			else// 出错，直接写入SQL数据，报告故障信息 
			{
				step = 5; //结束检查
				//sleep(TIMELOOP);
				mysql_wralog((char*)"Y", (char*)("GateWay NO-response"), (char*)(""));
			}
			break;

		case 2://检查第一个外网WEB服务器程序
			sockfd = 0;
			timeout = 0;
			sockfd = ConnectHost(webURL[0]);
			if (sockfd < 0) //连接失败,更改标志位，进入第二WEB服务器检查
			{
				printf("Action: Connect host %s error!\n", webURL[0]);
				netflag &= ~(1 << 5);
				step++; //步骤自加,进入第二次外网检查
				break;
			}
			timeout = GetHttpRequest(sockfd, webURL[0]);
			if (timeout >= 0)	//网络正常，更改NETFLAG标志位，
			{
				step = 5; //结束检查
				netflag += 1 << 5;
				//printf("web1 status:  OK\n");
				//网络正常，更改NETFLAG标志位，数据库写入相关数据
				mysql_wralog((char*)"Y", (char*)("Y"), (char*)("Y"));
				//sleep(TIMELOOP);
			}
			else// 出错，转到下一步骤，继续检查web服务器
			{
				step = 3;
				netflag &= ~(1 << 5);
			}
			break;

		case 3://检查第二个外网WEB服务器程序
			sockfd = 0;
			timeout = 0;
			sockfd = ConnectHost(webURL[1]);
			if (sockfd < 0) //连接失败,更改标志位，进入第三WEB服务器检查
			{
				printf("Action: Connect host %s error!\n", webURL[1]);
				netflag &= ~(1 << 4);
				step++; //步骤自加,进入第三次外网检查
				break;
			}
			timeout = GetHttpRequest(sockfd, webURL[1]);
			if (timeout >= 0)	//网络正常，更改NETFLAG标志位，
			{
				step = 5; //结束检查
				netflag += 1 << 4;
				//printf("web2 status:  OK\n");
				//网络正常，更改NETFLAG标志位，数据库写入相关数据
				mysql_wralog((char*)"Y", (char*)("Y"), (char*)("Y"));
				//sleep(TIMELOOP);
			}
			else// 出错，转到下一步骤，继续检查web服务器
			{
				step = 3;
				netflag &= ~(1 << 4);
			}

			break;

		case 4: //检查第三个外网WEB服务器程序
			sockfd = 0;
			timeout = 0;
			sockfd = ConnectHost(webURL[2]);
			if (sockfd < 0) //连接失败,更改标志位，置位步骤，写入最终SQL数据
			{
				step = 5; //结束检查
				printf("Action: Connect host %s error!\n", webURL[2]);
				netflag &= ~(1 << 3);
				//异常，写入SQL，报告故障信息
				mysql_wralog((char*)"Y", (char*)("Y"), (char*)("Web server NOT connect"));
				break;
			}
			timeout = GetHttpRequest(sockfd, webURL[2]);
			if (timeout >= 0)	//网络正常，更改NETFLAG标志位，
			{
				step = 5; //结束检查
				netflag += 1 << 4;
				//printf("web3 status:  OK\n");
				//网络正常，更改NETFLAG标志位，数据库写入相关数据
				mysql_wralog((char*)"Y", (char*)("Y"), (char*)("Y"));

			}
			else// 出错，转到步骤0，继续检查
			{
				step = 5; //结束检查
				netflag &= ~(1 << 3);
				//网络异常，更改NETFLAG标志位，数据库写入相关数据
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
	int reFlag = ChangeWorkPath(currentworkpath);//切换工作目录
	if (reFlag != 0)
	{
		printf("WARNING: Change work directory failse!\n");
		return -1;
	}
	int fileStatus = IsFileStatusOK((char*)"./af.conf");//检查文件读写权限是否正常
	if (fileStatus != 0)// { daemon(1, 0); }
	{
		printf("WARNING: file \'af.conf\' not exsit, please check file exsit and can be read!\n");
		exit(-1);
	}
	int status = 0;
	memset(gatewayip, 0, sizeof(gatewayip));
	status = GetProfileString((char*)"./af.conf", (char*)"gateway", (char*)"ip", gatewayip);//获取网关IP地址
	if (status == -1) printf("Action: Read af.conf gateway filed has a error! Please check it and try again!\n");

	memset(sendtime, 0, sizeof(sendtime));
	status = GetProfileString((char*)"./af.conf", (char*)"sendreport", (char*)"time", sendtime);//获取网关IP地址
	if (status == -1) printf("Action: Read af.conf sendreport filed has a error! Please check it and try again!\n");

	for (int ii = 0; ii < 3; ii++) //获取webURL地址
	{
		char urlstrtmp[10];
		memset(urlstrtmp, 0, sizeof(urlstrtmp));
		sprintf(urlstrtmp, "url%d", ii);
		memset(webURL[ii], 0, sizeof(webURL[ii]));
		status = GetProfileString((char*)"./af.conf", (char*)"webURL", urlstrtmp, webURL[ii]);
		if (status == -1) printf("Action: Read af.conf webURL filed has a error! Please check it and try again!\n");
	}


	for (int ii = 0; ii < 10; ii++)//获取邮件地址
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

void sendreport(void) //通过MAIL发送查询结果,并生成邮件内容
{
	int localbreak_t = GetTodayLocalBreakCount();
	int netbreak_t = GetTodayInternetBreakCount();
	int total_t = 0;//总
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
	sprintf(subject, "酒店网络连接质量结果报告(%d年%d月%d日)", ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday);
	sprintf(context, "截止至邮件发送时间，当日内网故障 %d 次，外网故障 %d 次，中断时间累计 %d 分钟。\r\n\
+---------------------------------------------------------------------------------------+\r\n\
提示：\r\n\
1.软件基于Ubuntu + MYSQL平台\r\n\
2.本次查询结果由系统自动生成并发送，如有疑问或获取当日详细记录，请回复邮件或与周勇联系（18981573198)\r\n\
3.检测机制，每10分钟轮询一次，检测内容包括（硬件是否连接正常、网关连接是否正常、外网连接是否正常），并将详细的检测结果记录于MYSQL中以备查\r\n\
4.本结果仅测试服务器端的光纤线路---->联通DNS---->机房干线---->163、baidu、sohu网页服务器的连接情况，不能全面表明仙人湖酒店其他HTTH光纤情况是否也如此，请知晓。\r\n\
+---------------------------------------------------------------------------------------+", localbreak_t, netbreak_t, total_t);
	
	for (int ii = 0; ii < 10; ii++)
	{
		if(strlen(mailTo[ii]) !=0 ) //如果邮件地址不为空，则向该地址发送邮件
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
		printf("创建守护进行失败！！！\n");
		writelog2file("创建守护进行失败！！！\n");
		exit(-1); //创建守护进行失败！
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
		strftime(strtime, 10, "%H:%M:%S", ptm);//格式化时间字符串
		if (strcmp(strtime, sendtime) == 0)
		{
			int localbreak_t = GetTodayLocalBreakCount();
			int netbreak_t = GetTodayInternetBreakCount();
			if ((localbreak_t > 0) || (netbreak_t > 0)) //当日有中断记录才发送邮件！
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