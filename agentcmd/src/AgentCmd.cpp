
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "wLog.h"
#include "wMisc.h"
#include "AgentCmd.h"
#include "AgentCmdConfig.h"

AgentCmd::AgentCmd() : wTcpClient<AgentCmdTask>(AGENT_SERVER_TYPE, "AgentServer", true)
{
	Initialize();
}

AgentCmd::~AgentCmd() 
{
	//
}

void AgentCmd::Initialize()
{
	AgentCmdConfig *pConfig = AgentCmdConfig::Instance();
	mAgentIp = pConfig->mIPAddr;
	mPort = pConfig->mPort;

	if (ConnectToServer(mAgentIp, mPort) < 0)
	{
		LOG_ERROR("default", "Connect to AgentServer failed");
		exit(1);
	}
	
	//初始化定时器
	mClientCheckTimer = wTimer(KEEPALIVE_TIME);
}


//准备工作
void AgentCmd::PrepareRun()
{
	//
}

void AgentCmd::Run()
{
	const int MAXLINE = 8196;

    char vBuff[MAXLINE];

    printf("%s %d>", mAgentIp.c_str(), mPort);

    while(fgets(vBuff, MAXLINE,stdin) != NULL)   //ctrl-D
    {
        vBuff[MAXLINE - 1] = 0;



        printf("%s %d>", mAgentIp.c_str(), mPort);
    }

    AgentCmd::Instance()->SetStatus();
}

