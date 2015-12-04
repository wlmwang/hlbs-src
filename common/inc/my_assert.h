//--------------------------------------------------
// 本文件用于实现一个带日志输出以及程序跳转控制的assert方法
//-------------------------------------------------- 

#ifndef _MY_ASSERT_H_
#define _MY_ASSERT_H_

#include "log.h"

#define MY_ASSERT(a, b) \
	if(!(a)) \
	{  \
		LOG_ERROR("default", "ASSERT %s failed, %s, %s, %d", #a, __FILE__, __FUNCTION__, __LINE__); \
		b; \
	}

#endif
