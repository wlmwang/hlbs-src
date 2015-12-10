
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
#include "wMisc.h"
#include "wLog.h"

#include "AgentServer.h"
#include "AgentConfig.h"

/**
 * 对于SIGUSR1信号的处理，用于服务器关闭
 * @param nSigVal [信号量]
 */
void Sigusr1Handle(int nSigVal)
{
	AgentServer::Instance()->SetStatus();
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
	//初始化配置
	AgentConfig *pConfig = AgentConfig::Instance();
	if(pConfig == NULL) 
	{
		LOG_ERROR("default", "Get RouterConfig instance failed");
		exit(1);
	}
	pConfig->ParseLineConfig(argc, argv);

	pConfig->ParseBaseConfig();

	//初始化日志
	InitailizeLog();

	if (pConfig->mDaemonFlag) 
	{
		//初始化守护进程
		const char *pFilename = "./agent_server.lock";

		if (InitDaemon(pFilename) < 0)
		{
			LOG_ERROR("default", "Create daemon failed!");
			exit(1);
		}
	}

	//获取服务器实体
    AgentServer *pServer = AgentServer::Instance();
	if(pServer == NULL) 
	{
		LOG_ERROR("default", "Get AgentServer instance failed");
		exit(1);
	}

	//准备工作
	pServer->PrepareStart(pConfig->mIPAddr, pConfig->mPort);

	//消息处理函数的注册
	signal(SIGUSR1, Sigusr1Handle);
	signal(SIGUSR2, Sigusr2Handle);
	
	//服务器开始运行
	LOG_INFO("default", "AgentServer start succeed");
	pServer->Start();

	LOG_SHUTDOWN_ALL;

	return 0;
}
