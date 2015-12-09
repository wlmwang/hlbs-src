
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_CONFIG_H_
#define _W_CONFIG_H_

#include <string.h>
#include <iostream>

#include "tinyxml.h"	//lib tinyxml
#include "wType.h"
#include "wLog.h"
#include "wSingleton.h"

template <typename T>
class wConfig : public wSingleton<T>
{
	public:
		int mDaemonFlag;	//是否启动为守护进程
		int mInitFlag;		//这个参数用于以后服务器在崩溃后被拉起
		
		wConfig()
		{
			mDaemonFlag = 0;
			mInitFlag = 0;
		}

		virtual void ParseLineConfig(int argc, const char *argv[])
		{
			//启动参数 -d -r
			for (int i = 1; i < argc; i++) 
			{
				if (strcasecmp((const char *)argv[i], "-D") == 0) 
				{
					mDaemonFlag = 1;
				}

				if (strcasecmp((const char *)argv[i], "-R") == 0)
				{
					mInitFlag = 0;
				}
			}
		}
};

#endif