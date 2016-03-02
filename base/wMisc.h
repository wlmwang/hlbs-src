
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_MISC_H_
#define _W_MISC_H_

#include <sstream>
#include <vector>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "wType.h"
#include "wCommand.h"

inline const char* IP2Text(long ip)
{
	in_addr in;
	in.s_addr = ip;

	return inet_ntoa(in);
}

inline long Text2IP(const char* ipstr)
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
inline unsigned long long GetTickCount()    //clock_t
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (unsigned long long)tv.tv_sec * 1000 + (unsigned long long)tv.tv_usec / 1000;
}

//long -> string
inline string Itos(const long &i)
{
    string sTmp;
    stringstream sRet(sTmp);
    sRet << i;
    return sRet.str();
}

inline void Strlow(u_char *dst, u_char *src, size_t n)
{
    while (n) 
    {
        *dst = tolower(*src);
        dst++;
        src++;
        n--;
    }
}

//long -> char*
void itoa(unsigned long val, char *buf, unsigned radix);

u_char *Cpystrn(u_char *dst, u_char *src, size_t n);

//分隔字符串
vector<string> Split(string sStr, string sPattern, bool bRepeat = true);

//两个数最大公约数
int Gcd(int a, int b);
//n个数最大公约数
int Ngcd(int *arr, int n);

//变为守护进程
int InitDaemon(const char *filename);

#endif
