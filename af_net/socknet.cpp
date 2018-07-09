#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<string.h>
#include<pthread.h>
#include<time.h>
#include"worksys.h"

int IsIPV4(char* purl)
{
	size_t length = strlen(purl);
	for (size_t ii = 0; ii < length; ii++)
	{
		if ((*(purl + ii) >= 'A') && (*(purl + ii) <= 'z')) return 0;
	}
	return 1;
}

int ConnectHost(char* purl)
{
	timespec starttime, endtime;
	
	clock_gettime(CLOCK_MONOTONIC, &starttime);
	int sockfd;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	hostent* phostent;
	sockaddr_in addr_in;
	addr_in.sin_family = AF_INET;
	addr_in.sin_port = htons(80);
	if (IsIPV4(purl)) //是IPV4格式
	{
		int status = inet_aton(purl, &addr_in.sin_addr);//转换IP地址为网络字节顺序
		if (status)
		{
			//printf("\nIP=%s\n", purl, purl); //如何转换没有出错，则打印IP信息
		}
		else //转换出错后
		{
			printf("Convert the internet host address from IPv4 numbers-and-dots false!\n");
			writelog2file("Convert the internet host address from IPv4 numbers-and-dots false!\n");
			close(sockfd);
			exit(-1);
		}

		/*printf("\nConnect %s ......", purl);
		fflush(stdout);*/

	}
	else //是域名格式
	{
		phostent = gethostbyname(purl); //解析域名，获取IP地址
		if ((void*)phostent == NULL)//error
		{
			printf("\nAction: Get host IP address false! Please check domain and try again\n");
			writelog2file("\nAction: Get host IP address false! Please check domain and try again\n");
			close(sockfd);
			exit(-1);
		}
	//	printf("\nURL=%s	IP=%s\n", purl, inet_ntoa(*(in_addr*)phostent->h_addr_list[0])); //打印域名转IP信息
	
	/*printf("\nConnect %s ......", inet_ntoa(*(in_addr*)phostent->h_addr_list[0]));
	fflush(stdout);	*/	
	addr_in.sin_addr = *(in_addr*)phostent->h_addr_list[0];
	}
	int status = connect(sockfd, (sockaddr*)&addr_in, sizeof(sockaddr));
	if (status < 0)
	{
		//printf("\nAction: Connect error!\n");
		close(sockfd);
		clock_gettime(CLOCK_MONOTONIC, &endtime);
		//printf("Connect Timeout=%d ms\n", ((endtime.tv_sec - starttime.tv_sec) * 1000) + ((endtime.tv_nsec - starttime.tv_nsec) / 1000000));
		//exit(-1);
		return status;
	}
	//printf("Done!\n");
	clock_gettime(CLOCK_MONOTONIC, &endtime);
	//printf("Connect Timeout=%d ms\n", ((endtime.tv_sec-starttime.tv_sec)*1000) + ((endtime.tv_nsec-starttime.tv_nsec)/1000000));
	//fflush(stdout);
	return sockfd;
}

int CloseHost(int sockfd)
{
	close(sockfd);
	return 0;
}

int GetHttpRequest(int sockfd, char* phost)
{
	timespec starttime, endtime;
	long timeout; //毫秒
	clock_gettime(CLOCK_MONOTONIC, &starttime);
	char recvbuff[40960];
	char sendbuff[1024];
	memset(recvbuff, 0, sizeof(recvbuff));
	memset(sendbuff, 0, sizeof(sendbuff));
	sprintf(sendbuff, "\
GET %s HTTP/1.1\r\n\
Host:%s\r\n\
User-Agent:Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:60.0) Gecko/20100101 Firefox/60.0\r\n\
Accept:text\r\n\
Accept-Language:zh-CN\r\n\
Accept-Encoding:gzip,deflate,br\r\n\
Connection:keep-alive\r\n\
Upgrade-Insecure-Requests:1\r\n\
Cache-Control:max-age=0,no-cache\r\n\
Pragma:no-cache\r\n\
\r\n", phost, phost);
	/*printf("\nSend data to server with GET  ......");
	fflush(stdout);*/
	ssize_t sendbyte = send(sockfd, sendbuff, sizeof(sendbuff), 0);
	if (sendbyte < 0)// 发送出错
	{
		printf("\nWARNING: Send error!\n");
		CloseHost(sockfd);
		return -1;
	}
	//printf("Done!\n");
	//printf("\nWait data from server ......");
	//fflush(stdout);
	ssize_t recvbyte = recv(sockfd, recvbuff, sizeof(recvbuff), 0);
	if (recvbyte < 0) //接收出错
	{
		printf("\nWARNING: Recv error!\n");
		CloseHost(sockfd);
		return -1;
	}
	//printf("Done!\n");
	CloseHost(sockfd);
	//printf("\nSocket was closed!\n");
	clock_gettime(CLOCK_MONOTONIC, &endtime);
	timeout = ((endtime.tv_sec - starttime.tv_sec) * 1000) + ((endtime.tv_nsec - starttime.tv_nsec) / 1000000); //计算所用的时间
	//printf("Get http repeate Timeout=%d ms\n", ((endtime.tv_sec - starttime.tv_sec) * 1000) + ((endtime.tv_nsec - starttime.tv_nsec) / 1000000));
	return (int)timeout;
}
