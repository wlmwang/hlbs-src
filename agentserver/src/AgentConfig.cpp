
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include <iostream>
#include <algorithm>

#include "wType.h"
#include "tinyxml.h"	//lib tinyxml
#include "AgentConfig.h"

void AgentConfig::Initialize()
{
	mPort = 0;
	mBacklog = 1024;
	mWorkers = 1;
	memset(mIPAddr, 0, sizeof(mIPAddr));
	memset(mRouterConf, 0, sizeof(mRouterConf));
	mDoc = new TiXmlDocument();
}

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

int AgentConfig::InitSvr(SvrNet_t* pSvr, int iNum)
{
	if (iNum <= 0)
	{
		return -1;
	}

	Final();
	for(int i = 0; i < iNum ; i++)
	{
		Svr_t *pSvr = new Svr_t(pSvr[i]);
		mSvr.push_back(pSvr);
	}
	FixContainer();
	return 0;
}

int AgentConfig::ReloadSvr(SvrNet_t *pSvr, int iNum)
{
	return InitSvr(pSvr, iNum);
}

//更新不同配置 & 添加新配置
int AgentConfig::SyncSvr(SvrNet_t *pSvr, int iNum)
{
	if (iNum <= 0)
	{
		return -1;
	}

	int j = 0;
	for(int i = 0; i < iNum ; i++)
	{
		vector<Svr_t*>::iterator it = GetItById((pSvr+i)->mId);
		if (it != mSvr.end())
		{
			if(IsChangeSvr(*it, pSvr+i))
			{
				//更新配置
				SAFE_DELETE(*it);
				mSvr.erase(it);
				Svr_t *pSvr = new Svr_t(pSvr[i]);
				mSvr.push_back(pSvr);
				j++;
			}
		}
		else
		{
			//添加新配置
			Svr_t *pSvr = new Svr_t(pSvr[i]);
			mSvr.push_back(pSvr);
			j++;
		}
	}
	FixContainer();	
	return j;
}

int AgentConfig::GetSvrAll(SvrNet_t* pBuffer)
{
	vector<Svr_t*>::iterator it = mSvr.begin();
	for(int i = 0; it != mSvr.end(); i++, it++)
	{
		*(pBuffer+i) = **it;
	}
	return mSvr.size();
}

int AgentConfig::GetAllSvrByGid(SvrNet_t* pBuffer, int iGid)
{
	vector<Svr_t*> vSvr;
	int iNum = 0;
	map<int, vector<Svr_t*> >::iterator mn = mSvrByGid.find(iGid);
	if(mn != mSvrByGid.end())
	{
		vSvr = mn->second;
		
		iNum = vSvr.size();
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

int AgentConfig::GetAllSvrByGXid(SvrNet_t* pBuffer, int iGid, int iXid)
{
	string sGid = Itos(iGid);
	string sXid = Itos(iXid);
	string sGXid = sGid + "-" + sXid;
	
	vector<Svr_t*> vSvr;
	int iNum = 0;
	map<string, vector<Svr_t*> >::iterator mn = mSvrByGXid.find(sGXid);
	if(mn != mSvrByGXid.end())
	{
		vSvr = mn->second;	//已排序
		iNum = vSvr.size();

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

int AgentConfig::GetSvrByGXid(SvrNet_t* pBuffer, int iGid, int iXid)
{
	string sGid = Itos(iGid);
	string sXid = Itos(iXid);
	string sGXid = sGid + "-" + sXid;
	
	vector<Svr_t*> vSvr;
	int iNum = 0;
	map<string, vector<Svr_t*> >::iterator mn = mSvrByGXid.find(sGXid);
	if(mn != mSvrByGXid.end())
	{
		vSvr = mn->second;	//已排序

		iNum = ChooseOneSvr(vSvr, pBuffer);
	}
	else
	{
		iNum = 0;
	}
	return iNum;
}

//加权轮询
int ChooseOneSvr(vector<Svr_t*> vSvr, SvrNet_t* pBuffer)
{
	//static long iIndex = 0;
	
	vector<Svr_t*>::iterator it = vSvr.begin();
	for(; it != vSvr.end(); it++)
	{
		if ((*it)->mDisabled == 1)
		{
			continue;
		}
		if (CalcOverLoad(*it) == 1)	//过载
		{
			(*it)->mRfuNum++;
			continue;
		}

/*
		i = -1;
		cw = 0;
		while (true) {
		  i = (i + 1) % 3;
		  if (i == 0) {
		     cw = cw - gcd(S); 
		     if (cw <= 0) {
		       cw = max(S);
		       if (cw == 0)
		         return NULL;
		     }
		  } 
		  if (W(Si) >= cw) 
		    return Si;
		}
*/

		//WRR Start
		*pBuffer = **it;
		
		return 1;
	}
	return 0;
}

//使用私有结构
void AgentConfig::ReportSvr(SvrReportReqId_t *pReportSvr)
{
	vector<Svr_t*>::iterator it = GetItById(pReportSvr->mId);
	if (it != mSvr.end() && pReportSvr->mDelay != 0 && pReportSvr->mStu != 0)
	{
		SetGXDirty((*it), 1);	//设置同组所有svr更新位
		(*it)->mDelay = pReportSvr->mDelay;
		if (pReportSvr->mStu == 1)	//成功
		{
			(*it)->mOkNum++;
		}
		else if(pReportSvr->mStu == -1)	//失败
		{
			(*it)->mErrNum++;
		}
		LOG_DEBUG("default","[runtime] recvive a report message(shm), Ok(%d),Err(%d)", (*it)->mOkNum, (*it)->mErrNum);
	}
}

void AgentConfig::Statistics()
{
	if(mSvr.size() <= 0) return;

	vector<Svr_t*>::iterator it;
	for (it = mSvr.begin(); it != mSvr.end(); it++)
	{
		if ((*it)->mDirty == 1)
		{
			(*it)->mOkRate = (float)(*it)->mOkNum / (*it)->mTotalNum;
			(*it)->mErrRate = (float)(*it)->mErrNum / (*it)->mTotalNum;
			(*it)->mRfuRate = (float)(*it)->mRfuNum / (*it)->mTotalNum;

			(*it)->mPreNum = CalcPre(*it);
			(*it)->mWeight = CalcWeight(*it);
			
			(*it)->ClearStatistics();	//清除统计数据
		}
	}
	FixContainer();
	LOG_DEBUG("default","[runtime] statistics svr success");
}

short AgentConfig::CalcPre(Svr_t* stSvr)
{
	return 1;
}

//计算动态权重
int AgentConfig::CalcWeight(Svr_t* stSvr)
{
	int fDefault = 1;
	if (stSvr->mDelay == 0 && stSvr->mOkRate == 0)
	{
		return fDefault;
	}

	Svr_t pSvr[MAX_SVR_NUM];
	int iNum = GetAllSvrByGXid(pSvr, stSvr->mGid, stSvr->mXid);
	if (iNum <= 0)
	{
		return fDefault;	//讲道理的话，这里不可能执行到（至少有其自身）
	}

	int iDelay = stSvr->mDelay;
	float fOkRate = stSvr->mOkRate;
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
	float fLoadW = ((float)stSvr->mDelay/iDelay) * (fOkRate/stSvr->mOkRate);
	do {
		fLoadW *= 10;
	} while (RATE_PRECISION--);
	return (int)fLoadW;
}

//检测是否过载
short AgentConfig::CalcOverLoad(Svr_t* stSvr)
{
	float fDefault = 0.5; //错误率
	if (stSvr->mErrNum > 0 && stSvr->mPreNum <= 0)	//本周期
	{
		return 1; //过载
	}
	else if(stSvr->mErrRate > fDefault)	//上一周期错误率
	{
		return 1; //过载(存在故障可能性)
	}
	return 0;
}

//设置同组所有svr更新位
void AgentConfig::SetGXDirty(Svr_t* stSvr, int iDirty)
{
	Svr_t pSvr[MAX_SVR_NUM];
	int iNum = GetAllSvrByGXid(pSvr, stSvr->mGid, stSvr->mXid);
	if (iNum <= 0)
	{
		return;	//讲道理的话，这里不可能执行到（至少有其自身）
	}
	for (int i = 0; i < iNum; ++i)
	{
		 pSvr[i].mDirty = iDirty;
	}
}

vector<Svr_t*>::iterator AgentConfig::GetItFromV(Svr_t* pSvr)
{
	vector<Svr_t*>::iterator it;
	for (it = mSvr.begin(); it != mSvr.end(); it++)
	{
		if (**it == *pSvr)
		{
			break;
		}
	}
	return it;
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

bool AgentConfig::IsChangeSvr(const Svr_t* pR1, const Svr_t* pR2)
{
	if (pR1->mGid!=pR2->mGid || pR1->mXid!=pR2->mXid || pR1->mSWeight!=pR2->mSWeight || pR1->mPort!=pR2->mPort || strncmp(pR1->mIp,pR2->mIp,MAX_SVR_IP_LEN)!=0)
	{
		return true;
	}
	return false;
}

void AgentConfig::DelContainer()
{
	mSvrByGid.clear();
	mSvrByGXid.clear();
}

void AgentConfig::Final()
{
	DelContainer();
	for(vector<Svr_t*>::iterator it = mSvr.begin(); it != mSvr.end(); it++)
	{
		SAFE_DELETE(*it);
	}
	mSvr.clear();
}

//整理容器 & 同步其他容器
void AgentConfig::FixContainer()
{
	if(mSvr.size() <= 0) return;
	//sort(mSvr.begin(), mSvr.end(), GreaterSvr);	//降序排序

	DelContainer();
	string sGid, sXid, sGXid;
	vector<Svr_t*> vSvr;
	for(vector<Svr_t*>::iterator it = mSvr.begin(); it != mSvr.end(); it++)
	{
		sGid = Itos((*it)->mGid); 
		sXid = Itos((*it)->mXid);
		sGXid = sGid + "-" + sXid;

		//mSvrByGid
		vSvr.clear();
		vSvr.push_back(*it);
		mSvrByGid.insert(pair<int, vector<Svr_t*> >((*it)->mGid, vSvr));
		
		//mSvrByGXid
		vSvr.clear();
		vSvr.push_back(*it);
		mSvrByGXid.insert(pair<string, vector<Svr_t*> >(sGXid, vSvr));
	}
}
