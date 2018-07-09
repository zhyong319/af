#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include<unistd.h>
#include<netdb.h>
#include"worksys.h"

#define SOCKET_ERROR -1

int mailSockfd;
//char name[64] = "NTgxODA2OTg=";//base64
//char passwd[64] = "dXhlcXRobGFmbmVxYmdlZg==";//base64

int connectHost(const char *smtpaddr)
{
	char buffer[4096];
	hostent* host;
	host = gethostbyname(smtpaddr);
	if (!host)
	{
		printf("domain failse!\n");
		writelog2file("domain failse!\n");
		return false;
	}

	struct sockaddr_in servaddr;
	mailSockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (mailSockfd < 0)
	{
		printf("Create socket error!\n");
		writelog2file("Create socket error!\n");
		return -1;
	}
	
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(25);
	servaddr.sin_addr = *(in_addr*)*host->h_addr_list;

	if (connect(mailSockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		printf("Connect failed... \n");
		writelog2file("Connect failed... \n");
		return -1;
	}
	char   str[32];
	memset(str, 0, sizeof(str));
	printf("smtp server IP:%s\n", inet_ntop(AF_INET, smtpaddr, str, sizeof(str)));
	writelog2file("smtp server IP:%s\n", inet_ntop(AF_INET, smtpaddr, str, sizeof(str)));
	printf("Connect to %s...... \n",smtpaddr);
	writelog2file("Connect to %s...... \n",smtpaddr);

	memset(buffer, 0, sizeof(buffer));

	if (recv(mailSockfd, buffer, sizeof(buffer), 0) < 0)
	{
		printf("receive failed... \n");
		writelog2file("receive failed... \n");
		return -1;
	}
	else
	{
		printf("Server response: %s\n", buffer);
		writelog2file("Server response: %s\n", buffer);
	}
	return mailSockfd;
}


int getResponse()
{
	char buffer[4096];
	memset(buffer, 0, sizeof(buffer));
	ssize_t ret = recv(mailSockfd, buffer, 1024, 0);
	if (ret == SOCKET_ERROR)
	{
		printf("receive nothing\n");
		writelog2file("receive nothing\n");
		return -1;
	}
	buffer[ret] = '\0';

	if (*buffer == '5')
	{
		printf("command error! code:%s\n",buffer);
		writelog2file("command error! code:%s\n",buffer);
		return -1;
	}

	printf("recieved buffer:%s\n", buffer);
	writelog2file("recieved buffer:%s\n", buffer);

	return 0;
}

int login(char *username, char *password)
{
	ssize_t ret;
	printf("login.....mailSockfd: %d\n", mailSockfd);
	writelog2file("login.....mailSockfd: %d\n", mailSockfd);
	char ch[100];
	if (username == (char*)"" || password == (char*)"")
		return -1;

	char* send_data = (char*)"HELO UbuntuHost\r\n"; 
	printf("Client Send %s\n", send_data);
	writelog2file("Client Send %s\n", send_data);
	ret = send(mailSockfd, send_data, strlen(send_data), 0);
	if (ret == SOCKET_ERROR)
	{
		close(mailSockfd);
		return -1;
	}
	if (getResponse() < 0)
		return -1;

	send_data = (char*)"AUTH LOGIN\r\n";
	printf("Client Send %s\n", send_data);
	writelog2file("Client Send %s\n", send_data);
	ret = send(mailSockfd, send_data, strlen(send_data), 0);
	if (ret == SOCKET_ERROR)
	{
		close(mailSockfd);
		return -1;
	}
	if (getResponse() < 0)
		return -1;
	sprintf(ch, "%s\r\n", username);
	printf("%s\n", ch);
	writelog2file("%s\n", ch);
	ret = send(mailSockfd, (char *)ch, strlen(ch), 0);
	if (ret == SOCKET_ERROR)
	{
		close(mailSockfd);
		return -1;
	}

	if (getResponse() < 0)
		return -1;

	sprintf(ch, "%s\r\n", password);
	printf("%s\n", ch);
	writelog2file("%s\n", ch);
	ret = send(mailSockfd, (char *)ch, strlen(ch), 0);
	if (ret == SOCKET_ERROR)
	{

		close(mailSockfd);
		return -1;
	}

	if (getResponse() < 0)
		return -1;

	return 0;

}

int sendmail(char* from, char * to, char * subject, char * context)
{
	ssize_t ret;
	char buffer[4096];
	char From[128];
	char To[128];
	char Context[409600]; //Context
	if (from == (char*)"" || to == (char*)"" || subject == (char*)"" || context == (char*)"")
	{
		printf("arguments error!\n");
		writelog2file("arguments error!\n");
		return -1;
	}
	sprintf(From, "MAIL FROM:<%s>\r\n", from);//填充 MAIL FROM 内容
	printf("Client Send %s\n", From);
	writelog2file("Client Send %s\n", From);
	if ((ret = send(mailSockfd, From, strlen(From), 0)) == SOCKET_ERROR) // 发送 MAIL FROM 指令
	{
		close(mailSockfd);
		return -1;
	}
	if (getResponse() < 0)
		return -1;

	sprintf(To, "RCPT TO:<%s>\r\n", to); //填充 RCPT TO 内容
	printf("Client Send %s\n", To);
	writelog2file("Client Send %s\n", To);
	if ((ret = send(mailSockfd, To, strlen(To), 0)) == SOCKET_ERROR) // 发送 RCPT TO 指令
	{
		close(mailSockfd);
		return -1;
	}
	if (getResponse() < 0)
		return -1;

	char* send_data = (char*)"DATA\r\n";  
	printf("Client Send %s\n", send_data);
	writelog2file("Client Send %s\n", send_data);
	if ((ret = send(mailSockfd, send_data, strlen(send_data), 0)) == SOCKET_ERROR)  // 发送 DATA 指令
	{
		close(mailSockfd);
		return -1;
	}
	if (getResponse() < 0)
		return -1;

	memset(Context, 0, sizeof(Context));
	sprintf(Context, "From:%s\r\nTo:%s\r\nSubject:%s\r\n\r\n%s\r\n", from, to, subject, context); //填充邮件正文内容
	strcat(Context, (char*)"\r\n.\r\n");
	if ((ret = send(mailSockfd, Context, strlen(Context), 0)) == SOCKET_ERROR) //发送邮件正文内容
	{
		printf("Send mail context failse!\n");
		writelog2file("Send mail context failse!\n");
		close(mailSockfd);
		return -1;
	}

	printf("\ncontext mail byte:%d byte\n", ret);
	writelog2file("\ncontext mail byte:%d byte\n", ret);
	memset(buffer, '\0', sizeof(buffer));

	if (getResponse() < 0)
		return -1;

	printf("SMTP host status: quit\n");
	writelog2file("SMTP host status: quit\n");
	if ((ret = send(mailSockfd, (char*)"QUIT\r\n", strlen("QUIT\r\n"), 0)) == SOCKET_ERROR) //发送QUIT指令，退出
	{
		close(mailSockfd);
		return -1;
	}
	if (getResponse() < 0)
		return -1;

	printf("Send Mail Successful..!\n");
	writelog2file("Send Mail Successful..!\n");
	return 0;
}


int testmail(void)   //exzample
{
	char from[128] = "58180698@qq.com";
	char to[128] = "zhyong319@163.com";

	char subject[512] = "Hello world";
	char context[6000] = "这是一封中文邮件,\r\n通过LINUX系统发送\r\nHello world, Hello mail，\r\n编码测ABCDEFGHIJKLMNOPQRSTUVWXYZ试\r\n";
	char server[56] = "smtp.qq.com";

	char name[64] = "NTgxODA2OTg=";//base64
	char passwd[64] = "dXhlcXRobGFmbmVxYmdlZg==";//base64

	if (connectHost(server)<0)
	{
		printf("ACTION: Can Not CONNECT smtp.qq.com\n");
		writelog2file("ACTION: Can Not CONNECT smtp.qq.com\n");
		return -1;
	}

	if (login(name, passwd) < 0)
	{
		fprintf(stderr, "ACTION: Can Not LOGIN !\n");
		writelog2file("ACTION: Can Not LOGIN !\n");
		return -1;
	}
	sendmail(from, to, subject, context);
	return 0;
}

int writemailto(char* from, char * to, char * subject, char * context)   //写邮件，登陆、并发送
{
	/*char from[128] = "58180698@qq.com";
	char to[128] = "zhyong319@163.com";

	char subject[512] = "Hello world";
	char context[6000] = "这是一封中文邮件,\r\n通过LINUX系统发送\r\nHello world, Hello mail，\r\n编码测ABCDEFGHIJKLMNOPQRSTUVWXYZ试\r\n";*/
	char server[56] = "smtp.qq.com";

	char name[64] = "NTgxODA2OTg=";//base64
	char passwd[64] = "dXhlcXRobGFmbmVxYmdlZg==";//base64

	if (connectHost(server)<0)
	{
		printf("ACTION: Can Not CONNECT smtp.qq.com\n");
		writelog2file("ACTION: Can Not CONNECT smtp.qq.com\n");
		return -1;
	}

	if (login(name, passwd) < 0)
	{
		fprintf(stderr, "ACTION: Can Not LOGIN !\n");
		writelog2file("ACTION: Can Not LOGIN !\n");
		return -1;
	}
	sendmail(from, to, subject, context);
	return 0;
}
