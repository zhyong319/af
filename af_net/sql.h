#include<mysql/mysql.h>
#include"mytype.h"
#ifndef __SQL__H__
#define __SQL__H__

extern int mysql_wralog(const char* pHWstatus, const char* pGateway, const char* pInternet);

extern int GetTodayLocalBreakCount(void);

extern int GetTodayInternetBreakCount(void);
#endif