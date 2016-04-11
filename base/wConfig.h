
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _W_CONFIG_H_
#define _W_CONFIG_H_

#include "wCore.h"
#include "wLog.h"
#include "wMisc.h"
#include "wSingleton.h"
#include "wProcTitle.h"
#include "tinyxml.h"	//lib tinyxml

template <typename T>
class wConfig : public wSingleton<T>
{
	public:
		wConfig();
		void Initialize();
		virtual ~wConfig();
		
		/**
		 * 初始化基础日志对象
		 * @return 
		 */
		int InitErrLog()
		{
			return INIT_ROLLINGFILE_LOG(ELOG_KEY, ELOG_FILE, ELOG_LEVEL, ELOG_FSIZE, ELOG_BACKUP);
		}

		virtual int GetOption(int argc, const char *argv[]);

	public:
		int mShowVer;	//版本信息
		int mDaemon;	//是否启动为守护进程
		char *mSignal;	//信号字符串
		char *mHost;
		int mPort;

		wProcTitle *mProcTitle;		//进程标题
};

#include "wConfig.inl"

#endif