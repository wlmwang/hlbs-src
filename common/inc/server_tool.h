#ifndef _SERVER_TOOL_H_
#define _SERVER_TOOL_H_

#include <sys/time.h>
#include <stdlib.h>

// 获取毫秒级时间
unsigned long long GetTickCount();

// linux没有这个函数,不好说什么时候就用到了
void itoa(unsigned long val, char *buf, unsigned radix);

#endif
