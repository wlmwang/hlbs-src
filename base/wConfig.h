
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
#include "wMisc.h"
#include "wSingleton.h"

template <typename T>
class wConfig : public wSingleton<T>
{
	public:
		int mProcess;	//进程类型
		int mShowVer;	//版本信息
		int mDaemon;	//是否启动为守护进程
		char *mSignal;	//信号字符串
		char *mHost;
		int mPort;

		int mArgc;
		char **mOsArgv;		//原生参数
		char *mOsArgvLast;	//原始argv和environ的总体大小

		char **mOsEnv;	//原生环境变量
		int mMoveEnv;	//是否移动了environ

		string *mArgv;	//堆上参数

		wConfig()
		{
			mProcess = 0;
			mShowVer = 0;
			mDaemon = 1;
			mHost = 0;
			mPort = 0;
			mArgc = 0;
			mSignal = NULL;
			mMoveEnv = 0;
		}
		
		virtual ~wConfig() 
		{
			SAFE_DELETE_VEC(mArgv);
			if (mMoveEnv == 1)
			{
				SAFE_DELETE(environ);
				environ = mOsEnv;
			}
		}
		
		void SaveArgv(int argc, char *const *argv)
		{
			mArgc = argc;

			mArgv = new string[argc + 1];
			for(int i = 0; i < mArgc; i++)
			{
				mArgv[i] = argv[i];
			}

			mOsArgv = argv;
			mOsEnv = environ;
		}

		/**
		 *  移动**environ到堆上，为进程标题做准备。计算**environ指针结尾地址。
		 *  tips：*argv[]与**environ两个变量所占的内存是连续的，并且是**environ紧跟在*argv[]后面
		 */
		int InitSetproctitle()
		{
			u_char      *p;
			size_t size = 0;
			int   i;

			size = 0;
			//environ字符串总长度
		    for (i = 0; environ[i]; i++) 
		    {
		        size += strlen(environ[i]) + 1;		
		    }

		    p = new char[size];
		    if (p == NULL) 
		    {
		        return -1;
		    }

		    mOsArgvLast = mOsArgv[0];  //argv开始地址

		    //argv字符总长度
		    for (i = 0; mOsArgv[i]; i++)
		     {
		        if (mOsArgvLast == mOsArgv[i]) 
		        {
		            mOsArgvLast = mOsArgv[i] + strlen(mOsArgv[i]) + 1;
		        }
		    }

		    //移动**environ到堆上
		    for (i = 0; environ[i]; i++) 
		    {
		        if (mOsArgvLast == environ[i]) 
		        {
		            size = strlen(environ[i]) + 1;
		            mOsArgvLast = environ[i] + size;

		            Cpystrn(p, (u_char *) environ[i], size);
		            environ[i] = (char *) p;
		            p += size;
		        }
		    }

		    mOsArgvLast--;     //是原始argv和environ的总体大小。去除结尾一个NULL字符
		    mMoveEnv = 1;
		    return -1;
		}

		//设置标题
		void Setproctitle(char *title)
		{
		    u_char     *p;

		    mOsArgv[1] = NULL;

		    p = Cpystrn((u_char *) mOsArgv[0], (u_char *) "disvr: ", mOsArgvLast - mOsArgv[0]);

		    p = Cpystrn(p, (u_char *) title, mOsArgvLast - (char *) p);


		    //在原始argv和environ的连续内存中，将修改了的进程名字之外的内存全部清零
		    if (mOsArgvLast - (char *) p) 
		    {
		        memset(p, SETPROCTITLE_PAD, mOsArgvLast - (char *) p);
		    }
		}

		virtual int ParseLineConfig(int argc, const char *argv[])
		{
			char *p;
			int  i;

			SaveArgv(argc, argv);	//保存参数

			for (i = 1; i < argc; i++) 
			{
				p = (char *) argv[i];
				if (*p++ != '-') 
				{
					LOG_ERROR("default", "invalid option: \"%s\"", argv[i]);
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
						
						LOG_ERROR("default", "option \"-h\" requires ip address");
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
						
						LOG_ERROR("default", "option \"-h\" requires port number");
						return -1;

					case 's':
						if (*p) 
						{
							mSignal = (char *) p;
							goto next;
						}

						if (argv[++i]) 
						{
							mSignal = argv[i];

			                if (strcmp(ngx_signal, "stop") == 0 || strcmp(ngx_signal, "quit") == 0)
			                {
			                    mProcess = PROCESS_SIGNALLER;  //该进程只为发送信号而运行
			                    goto next;
			                }
						}
						
						LOG_ERROR("default", "option \"-h\" requires signal number");
						return -1;	
					default:
						LOG_ERROR("default", "invalid option: \"%c\"", *(p - 1));
						return -1;
					}
				}

			next:
				continue;
			}
			return 0;
		}

};

#endif