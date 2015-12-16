
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include <iostream>
#include <algorithm>
//#include <functional> 

#include "wType.h"
#include "tinyxml.h"	//lib tinyxml
#include "AgentConfig.h"

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

int AgentConfig::GetAllRtblReq()
{
	AgentServer *pServer = AgentServer::Instance();
	wMTcpClient<AgentServerTask>* pRouterConn = pServer->RouterConn();
	if(pRouterConn == NULL)
	{
		return -1;
	}
	wTcpClient<AgentServerTask>* pClient = pRouterConn->OneTcpClient(ROUTER_SERVER_TYPE);
	if(pClient != NULL && pClient->TcpTask())
	{
		RtblReqAll_t vRtl;
		return pClient->TcpTask()->SyncSend((char *)&vRtl, sizeof(vRtl));
	}
	return -1;
}

//整理容器
void AgentConfig::FixRtbl(Rtbl_t pRtbl[] , int iLen)
{
	if (iLen < 0)
	{
		return;
	}
	for(int i = 0; i < iLen ; i++)
	{
		mRtbl.push_back(&pRtbl[i]);
	}
	
	sort(mRtbl.begin(), mRtbl.end(), GreaterRtbl);//降序排序
	
	string sId, sName, sGid, sXid, sGXid;
	vector<Rtbl_t*> vRtbl;
	for(vector<Rtbl_t*>::iterator it = mRtbl.begin(); it != mRtbl.end(); it++)
	{
		sId = Itos((*it)->mId); sName = (*it)->mName; sGid = Itos((*it)->mGid); sXid = Itos((*it)->mXid);
		sGXid = sGid + "-" + sXid;
		
		//mRtblById
		mRtblById.insert(pair<int, Rtbl_t*>((*it)->mId ,*it));
		
		//mRtblByGid
		map<int, vector<Rtbl_t*> >::iterator mg = mRtblByGid.find((*it)->mGid);
		if(mg != mRtblByGid.end())
		{
			vRtbl = mg->second;
			mRtblByGid.erase(mg);
		}
		vRtbl.push_back(*it);
		mRtblByGid.insert(pair<int, vector<Rtbl_t*> >((*it)->mGid, vRtbl));
		
		//mRtblByName
		map<string, vector<Rtbl_t*> >::iterator mn = mRtblByName.find(sName);
		if(mn != mRtblByName.end())
		{
			vRtbl = mn->second;
			mRtblByName.erase(mn);
		}
		vRtbl.push_back(*it);
		mRtblByName.insert(pair<string, vector<Rtbl_t*> >(sName, vRtbl));
		
		//mRtblByGXid
		map<string, vector<Rtbl_t*> >::iterator mgx = mRtblByGXid.find(sGXid);
		if(mgx != mRtblByGXid.end())
		{
			vRtbl = mgx->second;
			mRtblByGXid.erase(mgx);
		}
		vRtbl.push_back(*it);
		mRtblByGXid.insert(pair<string, vector<Rtbl_t*> >(sGXid, vRtbl));		
	}
}

int AgentConfig::GetRtblById(Rtbl_t* pBuffer, int iId)
{
	int iNum = 0;
	map<int, Rtbl_t*>::iterator it = mRtblById.find(iId);
	if(it != mRtblById.end() && (it->second)->mDisabled == 0)
	{
		iNum = 1;
		*pBuffer = *(it->second);
	}
	return iNum;
}

int AgentConfig::GetRtblAll(Rtbl_t* pBuffer, int iNum)
{
	vector<Rtbl_t*>::iterator it = mRtbl.begin();
	if(iNum == 0) iNum = mRtbl.size();
	for(int i = 0; i < iNum && it != mRtbl.end() && (*it)->mDisabled == 0; i++, it++)
	{
		*(pBuffer+i) = **it;
	}
	return iNum;
}

int AgentConfig::GetRtblByGid(Rtbl_t* pBuffer, int iGid, int iNum)
{
	vector<Rtbl_t*> vRtbl;
	map<int, vector<Rtbl_t*> >::iterator mn = mRtblByGid.find(iGid);
	if(mn != mRtblByGid.end())
	{
		vRtbl = mn->second;
		
		if(iNum == 0) iNum = vRtbl.size();
		vector<Rtbl_t*>::iterator it = vRtbl.begin();
		for(int i = 0; i < iNum && it != vRtbl.end() && (*it)->mDisabled == 0; i++, it++)
		{
			*(pBuffer+i) = **it;
		}
	}
	else
	{
		iNum = 0;
	}
	return iNum;
}

int AgentConfig::GetRtblByName(Rtbl_t* pBuffer, string sName, int iNum)
{
	vector<Rtbl_t*> vRtbl;
	map<string, vector<Rtbl_t*> >::iterator mn = mRtblByName.find(sName);
	if(mn != mRtblByName.end())
	{
		vRtbl = mn->second;
		if(iNum == 0) iNum = vRtbl.size();
		
		vector<Rtbl_t*>::iterator it = vRtbl.begin();
		for(int i = 0; i < iNum && it != vRtbl.end() && (*it)->mDisabled == 0; i++, it++)
		{
			*(pBuffer+i) = **it;
		}
	}
	else
	{
		iNum = 0;
	}
	return iNum;
}

int AgentConfig::GetRtblByGXid(Rtbl_t* pBuffer, int iGid, int iXid, int iNum)
{
	string sGid = Itos(iGid);
	string sXid = Itos(iXid);
	string sGXid = sGid + "-" + sXid;
	
	vector<Rtbl_t*> vRtbl;
	map<string, vector<Rtbl_t*> >::iterator mn = mRtblByGXid.find(sGXid);
	if(mn != mRtblByGXid.end())
	{
		vRtbl = mn->second;
		if(iNum == 0) iNum = vRtbl.size();

		vector<Rtbl_t*>::iterator it = vRtbl.begin();
		for(int i = 0; i < iNum && it != vRtbl.end() && (*it)->mDisabled == 0; i++, it++)
		{
			*(pBuffer+i) = **it;
		}
	}
	else
	{
		iNum = 0;
	}
	return iNum;
}

BYTE AgentConfig::SetRtblAttr(WORD iId, BYTE iDisabled, WORD iWeight, WORD iTimeline, WORD iConnTime, WORD iTasks, WORD iSuggest)
{
	const int SuggestRate = 45;
	const int TimelineRate = 25;
	const int ConnTimeRate = 15;
	const int TasksRate = 5;
	
	map<int, Rtbl_t*>::iterator it = mRtblById.find(iId);
	if(it == mRtblById.end())
	{
		return -1;
	}
	
	if(iDisabled == 1)
	{
		return DisabledRtbl(iId);
	}
	
	int iWt = 0;
	if(iWeight > 0)
	{
		iWt = iWeight;
	}
	else
	{
		iWt = (SuggestRate*iSuggest*0.01)+ (TimelineRate*iTimeline*0.01)+ (ConnTimeRate*iConnTime*0.01)+ (TasksRate*iTasks*0.01);
		iWt = iWt >= 100 ? 100 : iWt;
	}
	return SetRtblWeight(iId, iWt);
}

BYTE AgentConfig::DisabledRtbl(WORD iId)
{
	vector<Rtbl_t*>::iterator it = mRtbl.begin();
	for(it; it != mRtbl.end(); it++)
	{
		if(iId == (*it)->mId)
		{
			(*it)->mDisabled = 1;
			FixRtbl();
			break;
		}
	}
	return 0;
}

BYTE AgentConfig::SetRtblWeight(WORD iId, WORD iWeight)
{
	vector<Rtbl_t*>::iterator it = mRtbl.begin();
	for(it; it != mRtbl.end(); it++)
	{
		if(iId == (*it)->mId)
		{
			(*it)->mWeight = iWeight;
			FixRtbl();
			break;
		}
	}
	return 0;
}
