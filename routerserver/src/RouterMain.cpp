
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "wType.h"
#include "wMisc.h"
#include "wLog.h"
#include "wSignal.h"
#include "RouterServer.h"
#include "RouterConfig.h"

/**
 * 对于SIGUSR1信号的处理，用于服务器关闭
 * @param nSigVal [信号量]
 */
void Sigusr1Func(int nSigVal)
{
	RouterServer::Instance()->SetStatus();
}

/**
 * 对于SIGUSR2信号的处理，暂时没什么用，保留
 * @param nSigVal [信号量]
 */
void Sigusr2Func(int nSigVal)
{
	//
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
	RouterConfig *pConfig = RouterConfig::Instance();
	if(pConfig == NULL) 
	{
		cout << "Get RouterConfig instance failed" << endl;
		exit(1);
	}
	
	if (pConfig->GetOption(argc, argv) < 0)
	{
		cout << "Get Option failed" << endl;
		exit(1);
	}

	if (pConfig->mDaemon) 
	{
		//初始化守护进程
		if (InitDaemon(LOCK_PATH) < 0)
		{
			LOG_ERROR(ELOG_KEY, "[startup] Create daemon failed!");
			exit(1);
		}
	}


	pConfig->ParseBaseConfig();
	pConfig->ParseRouterConfig();
	
	wMaster *pMaster = wMaster<wMaster>::Instance();
	pMaster.CreatePidFile();
	
	pMaster->MasterStart();


	//获取服务器实体
    RouterServer *pServer = RouterServer::Instance();
	if(pServer == NULL) 
	{
		LOG_ERROR(ELOG_KEY, "[startup] Get RouterServer instance failed");
		exit(1);
	}

	//准备工作
	pServer->PrepareStart(pConfig->mIPAddr, pConfig->mPort);

	//消息处理函数的注册
	wSignal stSigUsr1(Sigusr1Func);
	stSigUsr1.AddSignal(SIGUSR1);

	wSignal stSigUsr2(Sigusr1Handle);
	stSigUsr2.AddSignal(Sigusr2Func);
	
	//服务器开始运行
	LOG_INFO(ELOG_KEY, "[startup] RouterServer start succeed");
	pServer->Start();

	LOG_SHUTDOWN_ALL;

	return 0;
}
