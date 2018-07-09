#ifndef __WORKSYS__H
#define __WORKSYS__H

/*更改当前的工作目录*/
extern int ChangeWorkPath(char* pSrc);


/*检查文件是否有权限正常打开、读*/
extern int IsFileStatusOK(char* filename);


/*检查硬件是否连接正常,未连接返回0， 连接返回1*/
extern bool IsHardWareOK(void);


extern int writelog2file(const char *format, ...);

#endif