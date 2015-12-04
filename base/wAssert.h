
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

/**
 *  实现一个带日志输出以及程序跳转控制的assert方法
 */

#ifndef _W_ASSERT_H_
#define _W_ASSERT_H_

#include "wLog.h"

#define W_ASSERT(a, b) \
	if(!(a)) \
	{  \
		LOG_ERROR("default", "ASSERT %s failed, %s, %s, %d", #a, __FILE__, __FUNCTION__, __LINE__); \
		b; \
	}

#endif
