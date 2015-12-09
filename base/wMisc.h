
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_MISC_H_
#define _W_MISC_H_

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include "wCommand.h"
#include "wType.h"

//计算数组中个体的个数
#define countOf(entry_v) (sizeof(entry_v) / sizeof((entry_v)[0]))

inline const char* IP2Text(DWORD ip)
{
	in_addr in;
	in.s_addr = ip;

	return inet_ntoa(in);
}

inline DWORD Text2IP(const char* ipstr)
{
	return inet_addr(ipstr);
}

inline unsigned int HashString(const char* s)
{
	unsigned int hash = 5381;
	while (*s)
	{
		hash += (hash << 5) + (*s ++);
	}
	return hash & 0x7FFFFFFF;
}

//获取毫秒级时间
inline unsigned long long GetTickCount()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (unsigned long long)tv.tv_sec * 1000 + (unsigned long long)tv.tv_usec / 1000;
}

//变为守护进程
int InitDaemon(const char *filename);

//linux没有这个函数,不好说什么时候就用到了
void itoa(unsigned long val, char *buf, unsigned radix);

inline char * EncodeCmd(wCommand *pCmd,int *iLen)
{
	int iSize = sizeof(*pCmd);
	iLen = iSize + sizeof(int);
	char *pBuffer = new char[iLen];
	*(int*)pBuffer = iSize;
	strncpy(pBuffer, (char*)pCmd, iSize);
	return pBuffer;
}
#endif
