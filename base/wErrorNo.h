
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_ERRNO_H_
#define _W_ERRNO_H_

enum SERVER_ERROR_NO
{
	SEN_EXECUTE_SQL_FAILED			= -1,		// SQL执行失败
	SEN_GET_SQL_RESULT_FAILED		= -2,		// 获取SQL结果失败
	SEN_LUA_RUN_FAILED				= -3,		// lua执行失败
	SEN_NO_SUCH_PUSH				= -4,		// 没有什么push
	SEN_NO_SUCH_PHONE				= -5,		// 没有这个手机
};

#endif
