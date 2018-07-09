
#ifndef __MAIL_H_
#define __MAIL_H_

extern int mailSockfd;

//连接smtp主机
//参数： char型主机域名
extern int connectHost(const char *smtpaddr);

//接收SMTP主机响应信息
extern int getResponse();

//登陆SMTP服务器
//参数：  username :base64编码  password：base64编码
extern int login(char *username, char *password);

//发送邮件，并关闭连接 mailSockfd

extern int sendmail(char* from, char * to, char * subject, char * context);

//测试函数
extern int testmail(void);


//发送邮件到指定的收件人，
//参数  from:发件人   TO:收件人  SUBJECT：主题  CONTEXT:正文
extern int writemailto(char* from, char * to, char * subject, char * context);

#endif
