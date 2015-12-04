
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "wType.h"
#include "wLog.h"
#include "wFunction.h"

#include "RouterServer.h"
#include "RouterConfig.h"

/**
 * 对于SIGUSR1信号的处理，用于服务器关闭
 * @param nSigVal [信号量]
 */
void Sigusr1Handle(int nSigVal)
{
	RouterServer::Instance()->SetStatus();
	signal(SIGUSR1, Sigusr1Handle);
}

/**
 * 对于SIGUSR2信号的处理，暂时没什么用，保留
 * @param nSigVal [信号量]
 */
void Sigusr2Handle(int nSigVal)
{
	signal(SIGUSR2, Sigusr2Handle);
}

/**
 *  初始化日志
 */
void InitailizeLog()
{
	//...
}

/**
 * 入口函数
 * @param  argc [参数个数]
 * @param  argv [参数字符]
 * @return      [int]
 */
int main(int argc, const char *argv[])
{
	//是否启动为守护进程
	int nDaemonFlag = 0;
	
	//这个参数用于以后服务器在崩溃后被拉起
	int nInitFlag = 1;
	
	//启动参数 -d -r
	for (int i = 1; i < argc; i++) 
	{
		if (strcasecmp((const char *)argv[i], "-D") == 0) 
		{
			nDaemonFlag = 1;
		}

		if (strcasecmp((const char *)argv[i], "-R") == 0)
		{
			nInitFlag = 0;
		}
		//...
	}
	
	if (nDaemonFlag) 
	{
		//初始化守护进程
		const char *pFilename = "./router_server.lock";
		InitDaemon(pFilename);
	}
	//初始化配置
	RouterConfig *pConfig = RouterConfig::Instance();
	if(pConfig == NULL) 
	{
		LOG_ERROR("default", "Get RouterConfig instance failed");
		exit(1);
	}
	pConfig->ParseBaseConfig();
	pConfig->ParseRtblConfig();
	
	//初始化日志
	InitailizeLog();
	
	//获取服务器实体
    RouterServer *pServer = RouterServer::Instance();
	if(pServer == NULL) 
	{
		LOG_ERROR("default", "Get RouterServer instance failed");
		exit(1);
	}

	//准备工作
	pServer->PrepareStart(pConfig->mExIPAddr, pConfig->mExPort, pConfig->mBacklog);

	//消息处理函数的注册
	signal(SIGUSR1, Sigusr1Handle);
	signal(SIGUSR2, Sigusr2Handle);
	
	//服务器开始运行
	LOG_INFO("default", "RouterServer start succeed");
	pServer->Start();

	LOG_SHUTDOWN_ALL;

	return 0;
}
