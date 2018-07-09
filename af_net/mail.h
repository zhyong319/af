
#ifndef __MAIL_H_
#define __MAIL_H_

extern int mailSockfd;

//����smtp����
//������ char����������
extern int connectHost(const char *smtpaddr);

//����SMTP������Ӧ��Ϣ
extern int getResponse();

//��½SMTP������
//������  username :base64����  password��base64����
extern int login(char *username, char *password);

//�����ʼ������ر����� mailSockfd

extern int sendmail(char* from, char * to, char * subject, char * context);

//���Ժ���
extern int testmail(void);


//�����ʼ���ָ�����ռ��ˣ�
//����  from:������   TO:�ռ���  SUBJECT������  CONTEXT:����
extern int writemailto(char* from, char * to, char * subject, char * context);

#endif
