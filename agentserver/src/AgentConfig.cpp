
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include <iostream>
#include <algorithm>

#include "wType.h"
#include "tinyxml.h"	//lib tinyxml
#include "AgentConfig.h"

/**
 * 解析配置
 */
void AgentConfig::ParseBaseConfig()
{
	bool bLoadOK = mDoc->LoadFile("../config/conf.xml");
	if (!bLoadOK)
	{
		cout << "Load config file(../config/conf.xml) failed" << endl;
		exit(1);
	}

	TiXmlElement *pRoot = mDoc->FirstChildElement();
	if (NULL == pRoot)
	{
		cout << "Read root from config file(../config/conf.xml) failed" << endl;
		exit(1);
	}

	TiXmlElement *pElement = NULL;
	TiXmlElement *pChildElm = NULL;

	//读取日志配置
	pElement = pRoot->FirstChildElement("LOG");
	if(NULL != pElement)
	{
		for(pChildElm = pElement->FirstChildElement(); pChildElm != NULL; pChildElm = pChildElm->NextSiblingElement())
		{
			const char *szKey = pChildElm->Attribute("KEY");
			const char *szFile = pChildElm->Attribute("FILE");
			const char *szLevel = pChildElm->Attribute("LEVEL");

			if (szKey != NULL && szFile != NULL && szLevel != NULL)
			{
				const char *szMaxFileSize = pChildElm->Attribute("MAX_FILE_SIZE");
				long long nMaxFileSize = szMaxFileSize == NULL ? atoi(szMaxFileSize) : 10*1024*1024;

				INIT_ROLLINGFILE_LOG(szKey, szFile, (LogLevel)atoi(szLevel), nMaxFileSize, 20);
			}
			else
			{
				cout << "Warning: error log config" << endl;
			}
		}
	}
	else
	{
		cout << "Get log configure from config file failed" << endl;
		exit(1);
	}

	pElement = pRoot->FirstChildElement("SERVER");
	if(NULL != pElement)
	{
		const char *szIPAddr = pElement->Attribute("IPADDRESS");
		const char *szPort = pElement->Attribute("PORT");
		const char *szBacklog = pElement->Attribute("BACKLOG");
		const char *szWorkers = pElement->Attribute("WORKERS");
		if (szIPAddr != NULL && szPort != NULL)
		{
			mPort = atoi(szPort);
			memcpy(mIPAddr, szIPAddr, MAX_IP_LEN);
		}
		else
		{
			LOG_ERROR("error", "error ip or port config");
		}
		mBacklog = szBacklog != NULL ? atoi(szBacklog): mBacklog;
		mWorkers = szWorkers != NULL ? atoi(szWorkers): mWorkers;
	}
	else
	{
		LOG_ERROR("error", "Get Server ip and port from config file failed");
	}
}

void AgentConfig::ParseRouterConfig()
{
	bool bLoadOK = mDoc->LoadFile("../config/router.xml");
	if (!bLoadOK)
	{
		LOG_ERROR("error", "Load config file(../config/router.xml) failed");
		exit(1);
	}

	TiXmlElement *pElement = NULL;
	TiXmlElement *pChildElm = NULL;
	TiXmlElement *pRoot = mDoc->FirstChildElement();
	
	pElement = pRoot->FirstChildElement("ROUTERS");
	if(pElement != NULL)
	{
		int i = 0;
		for(pChildElm = pElement->FirstChildElement(); pChildElm != NULL ; pChildElm = pChildElm->NextSiblingElement())
		{
			const char *szIPAddr = pChildElm->Attribute("IPADDRESS");
			const char *szPort = pChildElm->Attribute("PORT");
			const char *szDisabled = pChildElm->Attribute("DISABLED");
			if (szIPAddr != NULL && szPort != NULL && i < 32)
			{
				mRouterConf[i].mPort = atoi(szPort);
				memcpy(mRouterConf[i].mIPAddr, szIPAddr, MAX_IP_LEN);
				mRouterConf[i].mDisabled = szDisabled != NULL ? atoi(szDisabled): mRouterConf[i].mDisabled;
			}
			else
			{
				LOG_ERROR("default", "error server config: line(%d)!", i+1);
			}
			i++;
		}
	}
	else
	{
		LOG_ERROR("error", "Get server from config file failed");
		exit(1);
	}
	
	FixSvr();
}

int AgentConfig::GetSvrAll(Svr_t* pBuffer, int iNum)
{
	vector<Svr_t*>::iterator it = mSvr.begin();
	if(iNum == 0) iNum = mSvr.size();
	for(int i = 0; i < iNum && it != mSvr.end(); i++, it++)
	{
		*(pBuffer+i) = **it;
	}
	return iNum;
}

int AgentConfig::GetSvrById(Svr_t* pBuffer, int iId)
{
	int iNum = 0;
	map<int, Svr_t*>::iterator it = mSvrById.find(iId);
	if(it != mSvrById.end())
	{
		iNum = 1;
		*pBuffer = *(it->second);
	}
	return iNum;
}

int AgentConfig::GetSvrByGid(Svr_t* pBuffer, int iGid, int iNum)
{
	vector<Svr_t*> vSvr;
	map<int, vector<Svr_t*> >::iterator mn = mSvrByGid.find(iGid);
	if(mn != mSvrByGid.end())
	{
		vSvr = mn->second;
		
		if(iNum == 0) iNum = vSvr.size();
		vector<Svr_t*>::iterator it = vSvr.begin();
		for(int i = 0; i < iNum && it != vSvr.end(); i++, it++)
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

int AgentConfig::GetSvrByName(Svr_t* pBuffer, string sName, int iNum)
{
	vector<Svr_t*> vSvr;
	map<string, vector<Svr_t*> >::iterator mn = mSvrByName.find(sName);
	if(mn != mSvrByName.end())
	{
		vSvr = mn->second;
		if(iNum == 0) iNum = vSvr.size();
		
		vector<Svr_t*>::iterator it = vSvr.begin();
		for(int i = 0; i < iNum && it != vSvr.end(); i++, it++)
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

int AgentConfig::GetSvrByGXid(Svr_t* pBuffer, int iGid, int iXid, int iNum)
{
	string sGid = Itos(iGid);
	string sXid = Itos(iXid);
	string sGXid = sGid + "-" + sXid;
	
	vector<Svr_t*> vSvr;
	map<string, vector<Svr_t*> >::iterator mn = mSvrByGXid.find(sGXid);
	if(mn != mSvrByGXid.end())
	{
		vSvr = mn->second;
		if(iNum == 0) iNum = vSvr.size();

		vector<Svr_t*>::iterator it = vSvr.begin();
		for(int i = 0; i < iNum && it != vSvr.end(); i++, it++)
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

int AgentConfig::InitSvr(Svr_t* pBuffer, int iNum)
{
	if (iNum <= 0)
	{
		return -1;
	}

	CleanSvr();
	for(int i = 0; i < iNum ; i++)
	{
		mSvr.push_back(&pBuffer[i]);
	}
	FixSvr();
	return 0;
}

int AgentConfig::ReloadSvr(Svr_t *pBuffer, int iNum)
{
	return InitSvr(pBuffer, iNum);
}

BYTE AgentConfig::SetSvrAttr(WORD iId, BYTE iDisabled, WORD iWeight, WORD iTimeline, WORD iConnTime, WORD iTasks, WORD iSuggest)
{
	const int SuggestRate = 45;
	const int TimelineRate = 25;
	const int ConnTimeRate = 15;
	const int TasksRate = 5;
	
	map<int, Svr_t*>::iterator it = mSvrById.find(iId);
	if(it == mSvrById.end())
	{
		return -1;
	}
	
	if(iDisabled == 1)
	{
		return DisabledSvr(iId);
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
	return SetSvrWeight(iId, iWt);
}

BYTE AgentConfig::DisabledSvr(WORD iId)
{
	vector<Svr_t*>::iterator it = mSvr.begin();
	for(it; it != mSvr.end(); it++)
	{
		if(iId == (*it)->mId)
		{
			(*it)->mDisabled = 1;
			FixSvr();
			break;
		}
	}
	return 0;
}

BYTE AgentConfig::SetSvrWeight(WORD iId, WORD iWeight)
{
	vector<Svr_t*>::iterator it = mSvr.begin();
	for(it; it != mSvr.end(); it++)
	{
		if(iId == (*it)->mId)
		{
			(*it)->mWeight = iWeight;
			FixSvr();
			break;
		}
	}
	return 0;
}

//整理容器
void AgentConfig::FixSvr()
{
	if(mSvr.size() <= 0) return;
	sort(mSvr.begin(), mSvr.end(), GreaterSvr);//降序排序

	string sId, sName, sGid, sXid, sGXid;
	vector<Svr_t*> vSvr;
	for(vector<Svr_t*>::iterator it = mSvr.begin(); it != mSvr.end(); it++)
	{
		sId = Itos((*it)->mId); sName = (*it)->mName; sGid = Itos((*it)->mGid); sXid = Itos((*it)->mXid);
		sGXid = sGid + "-" + sXid;
		
		//mSvrById
		mSvrById.insert(pair<int, Svr_t*>((*it)->mId ,*it));
		
		//mSvrByGid
		map<int, vector<Svr_t*> >::iterator mg = mSvrByGid.find((*it)->mGid);
		vSvr.clear();
		if(mg != mSvrByGid.end())
		{
			vSvr = mg->second;
			mSvrByGid.erase(mg);
		}
		vSvr.push_back(*it);
		mSvrByGid.insert(pair<int, vector<Svr_t*> >((*it)->mGid, vSvr));
		
		//mSvrByName
		map<string, vector<Svr_t*> >::iterator mn = mSvrByName.find(sName);
		vSvr.clear();
		if(mn != mSvrByName.end())
		{
			vSvr = mn->second;
			mSvrByName.erase(mn);
		}
		vSvr.push_back(*it);
		mSvrByName.insert(pair<string, vector<Svr_t*> >(sName, vSvr));
		
		//mSvrByGXid
		map<string, vector<Svr_t*> >::iterator mgx = mSvrByGXid.find(sGXid);
		vSvr.clear();
		if(mgx != mSvrByGXid.end())
		{
			vSvr = mgx->second;
			mSvrByGXid.erase(mgx);
		}
		vSvr.push_back(*it);
		mSvrByGXid.insert(pair<string, vector<Svr_t*> >(sGXid, vSvr));		
	}
}

void AgentConfig::CleanSvr()
{
	mSvrById.clear();
	mSvrByGid.clear();
	mSvrByName.clear();
	mSvrByGXid.clear();
	mSvr.clear();
}

void AgentConfig::Final()
{
	CleanSvr();
	SAFE_DELETE(mDoc);
	SAFE_DELETE(mInShareMem);
	SAFE_DELETE(mOutShareMem);
	SAFE_DELETE(mInMsgQueue);
	SAFE_DELETE(mOutMsgQueue);
}

void AgentConfig::Initialize()
{
	mPort = 0;
	mBacklog = 1024;
	mWorkers = 1;
	memset(mIPAddr, 0, sizeof(mIPAddr));
	memset(mRouterConf, 0, sizeof(mRouterConf));
	mDoc = new TiXmlDocument();
}

void AgentConfig::InitShareMemory()
{
	mInShareMem = new wShareMemory(SVR_SHARE_MEM_PIPE, 'i', MSG_QUEUE_LEN);
	mOutShareMem = new wShareMemory(SVR_SHARE_MEM_PIPE, 'o', MSG_QUEUE_LEN);
	char * pBuff = NULL;
	if((pBuff = mInShareMem->CreateShareMemory()) != NULL)
	{
		mInMsgQueue = new wMsgQueue();
		mInMsgQueue->SetBuffer(pBuff, MSG_QUEUE_LEN);
	}
	else
	{
		LOG_ERROR("error","[runtime] Create (In) Share Memory failed");
	}
	if((pBuff = mOutShareMem->CreateShareMemory()) != NULL)
	{
		mOutMsgQueue = new wMsgQueue();
		mOutMsgQueue->SetBuffer(pBuff, MSG_QUEUE_LEN);
	}
	else
	{
		LOG_ERROR("error","[runtime] Create (Out) Share Memory failed");
	}
}