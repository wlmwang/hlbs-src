
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
		int mShowVersionFlag;
		int mDaemonFlag;	//是否启动为守护进程
		int mInitFlag;		//这个参数用于以后服务器在崩溃后被拉起
		
		char* mHost;
		int mPort;

		wConfig()
		{
			mShowVersionFlag = 0;
			mDaemonFlag = 0;
			mInitFlag = 0;
			mHost = 0;
			mPort = 0;
		}
		
		virtual ~wConfig() {}
		
		virtual int ParseLineConfig(int argc, const char *argv[])
		{
			char *p;
			int  i;

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
						mShowVersionFlag = 1;
						break;

					case 'd':
						mDaemonFlag = 1;
						break;
						
					case 'r':
						mInitFlag = 1;
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