
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include <iostream>
#include "wType.h"
#include "tinyxml.h"	//lib tinyxml
#include "RouterConfig.h"

/**
 * 解析配置
 */
void RouterConfig::ParseBaseConfig()
{
	TiXmlDocument stDoc;
	bool bLoadOK = stDoc.LoadFile("../config/conf.xml");
	if (!bLoadOK)
	{
		printf("Load config file(../config/conf.xml) failed\n");
		exit(1);
	}

	/*
		--debug start

		TiXmlPrinter stPrinter;//提供的工具类,目的是将xml的数据按格式输出
		stDoc.Accept(&stPrinter);
		cout<<stPrinter.CStr()<<endl;//输出

		--debug end
	*/

	TiXmlElement *pRoot = stDoc.FirstChildElement();
	if (NULL == pRoot)
	{
		printf("Read root from config file(../config/conf.xml) failed\n");
		exit(1);
	}
	
	TiXmlElement *pElement = NULL;
	TiXmlElement *pChildElm = NULL;
	pElement = pRoot->FirstChildElement("ROUTER_SERVER");
	if(NULL != pElement)
	{
		for(pChildElm = pElement->FirstChildElement(); pChildElm != NULL ; pChildElm = pChildElm->NextSiblingElement())
		{
			const char *szIPAddr = pChildElm->Attribute("IPADDRESS");
			const char *szPort = pChildElm->Attribute("PORT");
			const char *szBacklog = pChildElm->Attribute("BACKLOG");

			mBacklog = atoi(szBacklog);
			mExPort = atoi(szPort);
			memcpy(mExIPAddr, szIPAddr, MAX_IP_LEN);
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

void RouterConfig::ParseRtblConfig()
{
	TiXmlDocument stDoc;
	stDoc.LoadFile("../config/rtbl.xml");
	TiXmlElement *pElement = NULL;
	TiXmlElement *pChildElm = NULL;

	TiXmlElement *pRoot = stDoc.FirstChildElement();
	
	pElement = pRoot->FirstChildElement("SERVER");
	if(pElement != NULL)
	{
		for(pChildElm = pElement->FirstChildElement(); pChildElm != NULL ; pChildElm = pChildElm->NextSiblingElement())
		{
			const char *szId = pChildElm->Attribute("ID");
			const char *szGid = pChildElm->Attribute("GID");
			const char *szXid = pChildElm->Attribute("XID");
			const char *szName = pChildElm->Attribute("NAME");
			const char *szIPAddr = pChildElm->Attribute("IPADDRESS");
			const char *szPort = pChildElm->Attribute("PORT");
			const char *szDisabled = pChildElm->Attribute("DISABLED");
			const char *szWeight = pChildElm->Attribute("WEIGHT");
			
			Rtbl_t vRtbl;
			vRtbl.mId = atoi(szId);
			vRtbl.mGid = atoi(szGid);
			vRtbl.mXid = atoi(szXid);
			vRtbl.mPort = atoi(szPort);
			vRtbl.mWeight = atoi(szWeight);
			vRtbl.mDisabled = atoi(szDisabled);
			memcpy(vRtbl.mName, szName, MAX_NAME_LEN);
			memcpy(vRtbl.mIp, szIPAddr, MAX_IP_LEN);
			mRtbl.push_back(vRtbl);
		}
	}
	else
	{
		printf("Get extranet connect ip and port from config file failed\n");
		exit(1);
	}
}
