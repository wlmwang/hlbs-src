
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
	FixContainer();
}

int AgentConfig::GetSvrAll(Svr_t* pBuffer)
{
	vector<Svr_t*>::iterator it = mSvr.begin();
	for(int i = 0; it != mSvr.end(); i++, it++)
	{
		*(pBuffer+i) = **it;
	}
	return mSvr.size();
}

int AgentConfig::InitSvr(Svr_t* pBuffer, int iNum)
{
	if (iNum <= 0)
	{
		return -1;
	}

	Final();
	for(int i = 0; i < iNum ; i++)
	{
		mSvr.push_back(&pBuffer[i]);
	}
	FixContainer();
	return 0;
}

int AgentConfig::ReloadSvr(Svr_t *pBuffer, int iNum)
{
	return InitSvr(pBuffer, iNum);
}

int AgentConfig::SyncSvr(Svr_t *pBuffer, int iNum)
{
	if (iNum <= 0)
	{
		return -1;
	}
	int j = 0;
	for(int i = 0; i < iNum ; i++)
	{
		vector<Svr_t*>::iterator it = GetItById((pBuffer+i)->mId);
		if (it != mSvr.end())
		{
			if (pBuffer[i].mGid != (*it)->mGid || pBuffer[i].mXid != (*it)->mXid || pBuffer[i].mDisabled != (*it)->mDisabled || pBuffer[i].mWeight != (*it)->mWeight || pBuffer[i].mPort != (*it)->mPort || strcpy(pBuffer[i].mIp,(*it)->mIp) != 0 || strcpy(pBuffer[i].mName,(*it)->mName) != 0)
			{
				//更新配置
				mSvr.erase(it);
				mSvr.push_back(pBuffer+i);
				j++;
			}
		}
		else
		{
			//添加新配置
			mSvr.push_back(pBuffer+i);
			j++;
		}
	}
	FixContainer();	
	return j;
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
		vSvr = mn->second;	//已排序
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
			FixContainer();
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
			FixContainer();
			break;
		}
	}
	return 0;
}

//整理容器 & 同步其他容器
void AgentConfig::FixContainer()
{
	if(mSvr.size() <= 0) return;
	sort(mSvr.begin(), mSvr.end(), GreaterSvr);//降序排序

	/*+*/DelContainer();
	string sId, sName, sGid, sXid, sGXid;
	vector<Svr_t*> vSvr;
	for(vector<Svr_t*>::iterator it = mSvr.begin(); it != mSvr.end(); it++)
	{
		sId = Itos((*it)->mId); sName = (*it)->mName; sGid = Itos((*it)->mGid); sXid = Itos((*it)->mXid);
		sGXid = sGid + "-" + sXid;
	
		//mSvrById
		mSvrById.insert(pair<int, Svr_t*>((*it)->mId ,*it));
		
		//mSvrByGid
		vSvr.clear();
		vSvr.push_back(*it);
		mSvrByGid.insert(pair<int, vector<Svr_t*> >((*it)->mGid, vSvr));
		
		//mSvrByName
		vSvr.clear();
		vSvr.push_back(*it);
		mSvrByName.insert(pair<string, vector<Svr_t*> >(sName, vSvr));
		
		//mSvrByGXid
		vSvr.clear();
		vSvr.push_back(*it);
		mSvrByGXid.insert(pair<string, vector<Svr_t*> >(sGXid, vSvr));
	}
}

void AgentConfig::DelContainer()
{
	mSvrById.clear();
	mSvrByGid.clear();
	mSvrByName.clear();
	mSvrByGXid.clear();
}

void AgentConfig::Final()
{
	DelContainer();
	mSvr.clear();
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

vector<Svr_t*>::iterator AgentConfig::GetItById(int iId)
{
	vector<Svr_t*>::iterator it;
	for (it = mSvr.begin(); it != mSvr.end(); it++)
	{
		if ((*it)->mId == iId)
		{
			break;
		}
	}
	return it;
}

void AgentConfig::ReportSvr(SvrReportReqId_t *pReportSvr)
{
	vector<Svr_t*>::iterator it = GetItById(pReportSvr->mId);
	if (it != mSvr.end() && pReportSvr->mDelay > 0 && pReportSvr->mOkRate > 0)
	{
		(*it)->mDirty = 1;
		(*it)->mDelay = pReportSvr->mDelay;
		(*it)->mOkRate = pReportSvr->mOkRate;
		LOG_DEBUG("default","[runtime] recvive a report message(shm)");
	}
	FixContainer();
}

void AgentConfig::Statistics()
{
	if(mSvr.size() <= 0) return;

	vector<Svr_t*>::iterator it;
	for (it = mSvr.begin(); it != mSvr.end(); it++)
	{
		if ((*it)->mDirty == 1)	//TODO.要能影响其他svr
		{
			(*it)->mWeight = Calculate(*it);
			(*it)->mDirty = 0;
		}
	}
	FixContainer();

	LOG_DEBUG("default","[runtime] statistics svr success");
}

float AgentConfig::Calculate(Svr_t* pBuffer)
{
	Svr_t pSvr[255];
	int iNum = GetSvrByGXid(pSvr, pBuffer->mGid, pBuffer->mXid, 0);
	if (iNum <= 0)
	{
		return 1.0;	//讲道理的话，这里不可能执行到（至少有其自身）
	}

	int iDelay = 0;
	float fOkRate = 0.0;
	for (int i = 0; i < iNum; ++i)
	{
		if (iDelay > pSvr[i].mDelay && pSvr[i].mDelay != 0)
		{
			iDelay = pSvr[i].mDelay;	//最小延时
		}
		if (fOkRate < pSvr[i].mOkRate && pSvr[i].mOkRate != 0)
		{
			fOkRate = pSvr[i].mOkRate;	//最大成功率
		}
	}
	return iDelay!=0 && fOkRate!=0 && pBuffer->mDelay!=0 && pBuffer->mOkRate!=0? ((float)pBuffer->mDelay / iDelay)*(fOkRate / pBuffer->mOkRate) : 1.0;
}
