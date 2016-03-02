
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_CONFIG_H_
#define _W_CONFIG_H_

#include "wType.h"
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

template <typename T>
wConfig<T>::wConfig()
{
	Initialize();
}

template <typename T>
wConfig<T>::~wConfig() 
{
	SAFE_DELETE(mProcTitle);
}

template <typename T>
void wConfig<T>::Initialize()
{
	mShowVer = 0;
	mDaemon = 0;
	mHost = 0;
	mPort = 0;
	mSignal = NULL;
	mProcTitle = NULL;

	InitErrLog();
}

template <typename T>
int wConfig<T>::GetOption(int argc, const char *argv[])
{
	char *p;
	int  i;

	for (i = 1; i < argc; i++) 
	{
		p = (char *) argv[i];
		if (*p++ != '-') 
		{
			LOG_ERROR(ELOG_KEY, "invalid option: \"%s\"", argv[i]);
			return -1;
		}

		while (*p) 
		{
			switch (*p++) 
			{
			case '?':
			case 'v':
				mShowVer = 1;
				break;

			case 'd':
				mDaemon = 1;
				break;
				
			case 'h':
				if (*p) 
				{
					mHost = p;
					goto next;
				}

				if (argv[++i]) 
				{
					mHost = (char *) argv[i];
					goto next;
				}
				
				LOG_ERROR(ELOG_KEY, "option \"-h\" requires ip address");
				return -1;
				
			case 'p':
				if (*p) 
				{
					mPort = atoi(p);
					goto next;
				}

				if (argv[++i]) 
				{
					mPort = atoi(argv[i]);
					goto next;
				}
				
				LOG_ERROR(ELOG_KEY, "option \"-h\" requires port number");
				return -1;

			case 's':
				if (*p) 
				{
					mSignal = (char *) p;
					goto next;
				}

				if (argv[++i]) 
				{
					mSignal = (char *) argv[i];
					goto next;
				}
				
				LOG_ERROR(ELOG_KEY, "option \"-h\" requires signal number");
				return -1;	
			default:
				LOG_ERROR(ELOG_KEY, "invalid option: \"%c\"", *(p - 1));
				return -1;
			}
		}

	next:
		continue;
	}

	mProcTitle = new wProcTitle(argc, argv);
	mProcTitle->InitSetproctitle();
	return 0;
}

#endif