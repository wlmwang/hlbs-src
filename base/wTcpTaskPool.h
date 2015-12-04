
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_TCP_TASK_POOL_H_
#define _W_TCP_TASK_POOL_H_

#include <vector>

class wTcpTaskPool
{
	public:
		wTcpTaskPool();
		~wTcpTaskPool();
		
	private:
		vector<wTcpTask *> mTaskQueue;
};

#endif