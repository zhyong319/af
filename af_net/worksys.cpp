#include<unistd.h>
#include<libgen.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/types.h>
#include <errno.h>
#include<string.h>
#include<stdarg.h>
#include<time.h>
//int plogfile; //写入日志信息的文件FD

int ChangeWorkPath(char* pSrc)
{
	int flag = 0;
	flag = chdir(dirname(pSrc));
	//printf("path= %s,	flag=%d\n", pSrc, flag);
	return flag;
}

bool IsHardWareOK(void)
{
	char* pChar = (char*)"/sys/class/net/eth0/carrier";
	int fd_tmp = open(pChar, O_RDONLY);
	if (fd_tmp < 0)
	{
		printf(strerror(errno));
		return false;
	}
	char buff[4096];
	memset(buff, 0, sizeof(buff));
	read(fd_tmp, buff, sizeof(buff));
	close(fd_tmp);
	if (buff[0] == '1') return true;
	else return false;

}

int IsFileStatusOK(char* filename)
{
	//return value = 0x03(file not exsit and read not allow ), 0x02(read not allow), 0x01(file not exsit), 0x00(OK)
	int flag = 0;
	// 0    0   0   0   0   0   0	0   /value
	// 7    6	5	4	3	2	1	0   /bit
	//							读 存在
	if ((access(filename, F_OK)) != -1)
	{
		//printf("文件 xrh.conf 存在.\n");
		flag &= ~(1 << 0);
	}
	else
	{
		//printf("文件xrh.conf 不存在!\n");
		flag |= 1 << 0;
	}

	if (access(filename, R_OK) != -1)
	{
		//printf("xrh.conf 有可读权限\n");
		flag &= ~(1 << 1);
	}
	else
	{
		//	printf("xrh.conf 不可读.\n");
		flag |= 1 << 1;
	}


	return flag;
}


int writelog2file(const char *format, ...)
{
	char strtime[24];
	memset(strtime, 0, sizeof(strtime));
	time_t timer;
	struct tm* ptm;
	time(&timer);
	ptm = localtime(&timer);
	strftime(strtime, sizeof(strtime), "%Y/%m/%d %H:%M:%S    ", ptm);//格式化时间字符串

	char buff[4096];
	strcpy(buff, strtime);
	strcat(buff, format);
	FILE* pfile;
	pfile = fopen((char*)"./af.log", (char*)"a+");
	if (pfile == NULL)
	{
		printf("Action: Open af.log file failse!\n");
		exit(-1);
	}

	va_list ap;
	int retval;
	va_start(ap, format);
	retval = vfprintf(pfile, buff, ap);
	va_end(ap);


	fclose(pfile);
	return retval;
}

