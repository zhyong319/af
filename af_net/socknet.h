#ifndef __SOCKNET__H
#define __SOCKNET__H

/*连接主机，purl:域名  或者  IPv4*/
/*连接超时的时间默认约 129 秒*/
/*返回值为正常为套接字，出错后返回值<0*/
extern int ConnectHost(char* purl);

/*关闭套接字  sockfd: 打开的套接字*/
extern int CloseHost(int sockfd);

/*获取服务器的响应   sockfd: 创建的套接字   phost 域名（用于填充GET请求HOST健值）*/
/*服务器响应正常返回花费的时间， 否则返回-1*/
extern int GetHttpRequest(int sockfd, char* phost);

/*判断是字符串是否是IPV4还是域名   不是返回0， 是返回1*/
extern int IsIPV4(char* purl);

#endif