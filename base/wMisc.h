
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_MISC_H_
#define _W_MISC_H_

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include "wType.h"

//计算数组中个体的个数
#define count_of(entry_v) (sizeof(entry_v) / sizeof((entry_v)[0]))

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

//获取毫秒级时间
unsigned long long GetTickCount();

//linux没有这个函数,不好说什么时候就用到了
void itoa(unsigned long val, char *buf, unsigned radix);

#endif
