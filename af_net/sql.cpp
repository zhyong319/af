#include<mysql/mysql.h>
#include<stdio.h>
#include<time.h>
#include<string.h>
#include"mytype.h"
#include"worksys.h"

int mysql_wralog(const char* pHWstatus, const char* pGateway, const char* pInternet)
{
	MYSQL m_sql;
	if (!mysql_init(&m_sql))
	{
		printf("data base init false!\n");
		return 1;
	}
	if (!mysql_real_connect(&m_sql, "localhost", "root", "", "unicom", 0, NULL, 0))
	{
		printf("mysql connect false!\n");
		writelog2file("mysql connect false!\n");
		return 2;
	}
	char buff[200];
	int spCount = sprintf(buff, "INSERT INTO undata(time,HWstatus,GWstatus,NETstatus) values(now(),'%s','%s','%s');", pHWstatus, pGateway, pInternet);
	if (spCount == -1)
	{
		printf("sprintf funcation false!\n");
		writelog2file("sprintf funcation false!\n");
		return 4;
	}
	if (mysql_query(&m_sql, buff) != 0)
	{
		printf("Excute sql command false!\n");
		writelog2file("Excute sql command false!\n");
		return 3;
	}
	mysql_close(&m_sql);

	return 0;

}


int GetTodayInternetBreakCount(void)
{
	time_t tNow;
	time(&tNow);
	struct tm* ptnow = localtime(&tNow);
	MYSQL_RES* result;
	MYSQL m_sql;
	int count = 0;
	if (!mysql_init(&m_sql))
	{
		printf("WARNING: data base init false!\n");
		writelog2file("WARNING: data base init false!\n");
		return -1;
	}
	mysql_errno(&m_sql);
	mysql_real_connect(&m_sql, "localhost", "root", "", "unicom", 0, NULL, 0);
	char buff[200];
	memset(buff, 0, sizeof(buff));
	
	sprintf(buff, "select * from undata where (time>='%02d-%02d-%02d 00:00:00') and (time<='%02d-%02d-%02d 23:59:59') and HWstatus='Y' and GWstatus='Y' and NETstatus<>'Y';", \
		(ptnow->tm_year) + 1900, (ptnow->tm_mon) + 1, ptnow->tm_mday, (ptnow->tm_year) + 1900, (ptnow->tm_mon) + 1, ptnow->tm_mday);
	if (mysql_query(&m_sql, buff) != 0)
	{
		printf("Excute sql command false!\n");
		writelog2file("Excute sql command false!\n");
		return -1;
	}
	else //正确执行
	{
		result = mysql_store_result(&m_sql);
		if (result)
		{
			mysql_num_fields(result);
			//printf("Row Count: %d\n", result->row_count);
			count = (int)result->row_count;
		}

	}
	mysql_free_result(result);
	mysql_close(&m_sql);
	return count;
}


int GetTodayLocalBreakCount(void)
{
	time_t tNow;
	time(&tNow);
	struct tm* ptnow = localtime(&tNow);
	MYSQL_RES* result;
	MYSQL m_sql;
	int count = 0;
	if (!mysql_init(&m_sql))
	{
		printf("WARNING: data base init false!\n");
		writelog2file("WARNING: data base init false!\n");
		return -1;
	}
	mysql_errno(&m_sql);
	mysql_real_connect(&m_sql, "localhost", "root", "", "unicom", 0, NULL, 0);
	char buff[200];
	sprintf(buff, "select * from undata where (time>='%02d-%02d-%02d 00:00:00') and (time<='%02d-%02d-%02d 23:59:59') and HWstatus<>'Y' and GWstatus<>'Y';", \
		(ptnow->tm_year) + 1900, (ptnow->tm_mon) + 1, ptnow->tm_mday, (ptnow->tm_year) + 1900, (ptnow->tm_mon) + 1, ptnow->tm_mday);
	if (mysql_query(&m_sql, buff) != 0)
	{
		printf("Excute sql command false!\n");
		writelog2file("Excute sql command false!\n");
		return -1;
	}
	else //正确执行
	{
		result = mysql_store_result(&m_sql);
		if (result)
		{
			mysql_num_fields(result);
			//printf("Row Count: %d\n", result->row_count);
			count = (int)result->row_count;
		}

	}
	mysql_free_result(result);
	mysql_close(&m_sql);
	return count;
}



