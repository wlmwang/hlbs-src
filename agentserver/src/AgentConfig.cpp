
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include <iostream>
#include "wType.h"
#include "tinyxml.h"	//lib tinyxml
#include "AgentConfig.h"

#include "RouterCommand.h"

/**
 * 解析配置
 */
void AgentConfig::ParseBaseConfig()
{
	TiXmlDocument stDoc;
	bool bLoadOK = stDoc.LoadFile("../config/conf.xml");
	if (!bLoadOK)
	{
		printf("Load config file(../config/conf.xml) failed\n");
		exit(1);
	}

	TiXmlElement *pRoot = stDoc.FirstChildElement();
	if (NULL == pRoot)
	{
		printf("Read root from config file(../config/conf.xml) failed\n");
		exit(1);
	}
	
	TiXmlElement *pElement = NULL;
	TiXmlElement *pChildElm = NULL;
	pElement = pRoot->FirstChildElement("SERVER");
	if(NULL != pElement)
	{
		for(pChildElm = pElement->FirstChildElement(); pChildElm != NULL ; pChildElm = pChildElm->NextSiblingElement())
		{
			const char *szIPAddr = pChildElm->Attribute("IPADDRESS");
			const char *szPort = pChildElm->Attribute("PORT");
			const char *szBacklog = pChildElm->Attribute("BACKLOG");

			mBacklog = atoi(szBacklog);
			mPort = atoi(szPort);
			memcpy(mIPAddr, szIPAddr, MAX_IP_LEN);
		}
	}
	else
	{
		printf("Get extranet connect ip and port from config file failed\n");
		exit(1);
	}

	pElement = pRoot->FirstChildElement("ROUTER_SERVER");
	if(NULL != pElement)
	{
		for(pChildElm = pElement->FirstChildElement(); pChildElm != NULL ; pChildElm = pChildElm->NextSiblingElement())
		{
			const char *szIPAddr = pChildElm->Attribute("IPADDRESS");
			const char *szPort = pChildElm->Attribute("PORT");
			
			mRouterPort = atoi(szPort);
			memcpy(mRouterIPAddr, szIPAddr, MAX_IP_LEN);
		}
	}
	else
	{
		printf("Get extranet connect ip and port from config file failed\n");
		exit(1);
	}
	
	//读取日志配置
	pElement = pRoot->FirstChildElement("LOG");
	if(NULL != pElement)
	{
		for(pChildElm = pElement->FirstChildElement(); pChildElm != NULL; pChildElm = pChildElm->NextSiblingElement())
		{
			const char *szKey = pChildElm->Attribute("KEY");
			const char *szFile = pChildElm->Attribute("FILE");
			const char *szLevel = pChildElm->Attribute("LEVEL");

			//const char *szMaxFileSize = pChildElm->Attribute("MAX_FILE_SIZE");
			//long long nMaxFileSize = atoi(szMaxFileSize);

			//初始化日志
			INIT_ROLLINGFILE_LOG(szKey, szFile, (LogLevel)atoi(szLevel), 10*1024*1024, 20);
		}
	}
	else
	{
		printf("Get log configure from config file failed\n");
		exit(1);
	}
}

int AgentConfig::RequestGetAllRtbl()
{
	AgentServer *pServer = AgentServer::Instance();
	wMTcpClient<AgentServerTask>* pRouterConn = pServer->RouterConn();
	if(pRouterConn == NULL)
	{
		return -1;
	}
	wTcpClient<AgentServerTask>* pTask = pRouterConn->OneTcpClient(ROUTER_SERVER_TYPE);
	if(pTask != NULL)
	{
		RtblAll_t vRlt;
		return pTask->SyncSend((char *)&vRlt, sizeof(RtblAll_t));
	}
	return -1;
}

int AgentConfig::ResponseGetAllRtbl()
{
	//
}
