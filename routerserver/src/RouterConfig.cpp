
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include <iostream>
#include <algorithm>

#include "wType.h"
#include "wMisc.h"
#include "tinyxml.h"	//lib tinyxml
#include "RouterConfig.h"

/**
 * 解析配置
 */
void RouterConfig::ParseBaseConfig()
{
	bool bLoadOK = mDoc->LoadFile(mBaseConfFile);
	if (!bLoadOK)
	{
		cout << "[startup] Load config file(conf.xml) failed" << endl;
		exit(1);
	}

	TiXmlElement *pRoot = mDoc->FirstChildElement();
	if (NULL == pRoot)
	{
		cout << "[startup] Read root from config file(conf.xml) failed" << endl;
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
				cout << "[startup] Get log config from conf.xml error" << endl;
			}
		}
	}
	else
	{
		cout << "[startup] Get log config from conf.xml failed" << endl;
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
			LOG_ERROR("error", "[startup] Get SERVER ip or port from conf.xml failed");
		}
		mBacklog = szBacklog != NULL ? atoi(szBacklog): mBacklog;
		mWorkers = szWorkers != NULL ? atoi(szWorkers): mWorkers;
	}
	else
	{
		LOG_ERROR("error", "[startup] Get SERVER node from conf.xml failed");
		exit(1);
	}
}

void RouterConfig::ParseSvrConfig()
{
	bool bLoadOK = mDoc->LoadFile(mSvrConfFile);
	if (!bLoadOK)
	{
		LOG_ERROR("error", "[startup] Load config file(svr.xml) failed");
		exit(1);
	}

	TiXmlElement *pElement = NULL;
	TiXmlElement *pChildElm = NULL;
	TiXmlElement *pRoot = mDoc->FirstChildElement();
	
	pElement = pRoot->FirstChildElement("SVRS");
	if(pElement != NULL)
	{
		int i = 0;
		for(pChildElm = pElement->FirstChildElement(); pChildElm != NULL ; pChildElm = pChildElm->NextSiblingElement())
		{
			i++;
			const char *szId = pChildElm->Attribute("ID");
			const char *szGid = pChildElm->Attribute("GID");
			const char *szXid = pChildElm->Attribute("XID");
			const char *szIPAddr = pChildElm->Attribute("IPADDRESS");
			const char *szPort = pChildElm->Attribute("PORT");
			const char *szWeight = pChildElm->Attribute("WEIGHT");
			if (szId != NULL && szGid != NULL && szXid != NULL && szIPAddr != NULL && szPort != NULL)
			{
				Svr_t *pSvr = new Svr_t();
				pSvr->mId = atoi(szId);
				pSvr->mGid = atoi(szGid);
				pSvr->mXid = atoi(szXid);
				pSvr->mPort = atoi(szPort);
				memcpy(pSvr->mIp, szIPAddr, MAX_SVR_IP_LEN);
				pSvr->mSWeight = szWeight != NULL ? atoi(szWeight): pSvr->mSWeight;
				mSvr.push_back(pSvr);
			}
			else
			{
				LOG_ERROR("svr", "[startup] Parse svr config from svr.xml occur error: line(%d)!", i);
			}
		}
	}
	else
	{
		LOG_ERROR("error", "[startup] Get SVRS node from svr.xml failed");
		exit(1);
	}

	FixContainer();
	SetModTime();
}

int RouterConfig::ReloadSvr(SvrNet_t* pBuffer)
{
	Final();
	ParseSvrConfig();
	return GetSvrAll(pBuffer);
}

int RouterConfig::GetSvrAll(SvrNet_t* pBuffer)
{
	vector<Svr_t*>::iterator it = mSvr.begin();
	for(int i = 0; it != mSvr.end(); i++, it++)
	{
		*(pBuffer+i) = **it;	//Svr_t => SvrNet_t
	}
	return mSvr.size();
}

void RouterConfig::FixContainer()
{
	if(mSvr.size() <= 0) return;
	sort(mSvr.begin(), mSvr.end(), GreaterSvr);//降序排序
	
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

void RouterConfig::DelContainer()
{
	mSvrByGid.clear();
	mSvrByGXid.clear();
}

void RouterConfig::Final()
{
	DelContainer();
	for(vector<Svr_t*>::iterator it = mSvr.begin(); it != mSvr.end(); it++)
	{
		SAFE_DELETE(*it);
	}
	mSvr.clear();
}

int RouterConfig::SetModTime()
{
	struct stat stBuf;
	int iRet = stat(mSvrConfFile, &stBuf);
	if (iRet == 0)
	{
		mMtime = stBuf.st_mtime;
		return mMtime;
	}
	return iRet; //-1
}

bool RouterConfig::IsModTime()
{
	struct stat stBuf;
	int iRet = stat(mSvrConfFile, &stBuf);
	if (iRet == 0 && stBuf.st_mtime > mMtime)
	{
		return true;
	}
	return false;
}

//获取修改的svr
//不能删除节点（可修改Disabled=1属性，达到删除节点效果）
int RouterConfig::GetModSvr(SvrNet_t* pBuffer)
{
	bool bLoadOK = mDoc->LoadFile(mSvrConfFile);
	if (!bLoadOK)
	{
		LOG_ERROR("error", "[modify svr] Load config file(svr.xml) failed");
		return -1;
	}

	TiXmlElement *pElement = NULL;
	TiXmlElement *pChildElm = NULL;
	TiXmlElement *pRoot = mDoc->FirstChildElement();
	
	pElement = pRoot->FirstChildElement("SVRS");

	int i = 0 , j = 0;
	if(pElement != NULL)
	{
		for(pChildElm = pElement->FirstChildElement(); pChildElm != NULL ; pChildElm = pChildElm->NextSiblingElement())
		{
			i++;
			const char *szId = pChildElm->Attribute("ID");
			const char *szGid = pChildElm->Attribute("GID");
			const char *szXid = pChildElm->Attribute("XID");
			const char *szIPAddr = pChildElm->Attribute("IPADDRESS");
			const char *szPort = pChildElm->Attribute("PORT");
			const char *szWeight = pChildElm->Attribute("WEIGHT");
			if (szId != NULL && szGid != NULL && szXid != NULL && szIPAddr != NULL && szPort != NULL)
			{
				Svr_t *pSvr = new Svr_t();
				pSvr->mId = atoi(szId);
				pSvr->mGid = atoi(szGid);
				pSvr->mXid = atoi(szXid);
				pSvr->mPort = atoi(szPort);
				memcpy(pSvr->mIp, szIPAddr, MAX_SVR_IP_LEN);
				pSvr->mSWeight = szWeight != NULL ? atof(szWeight): pSvr->mSWeight;
				
				vector<Svr_t*>::iterator it = GetItFromV(pSvr);
				if (it != mSvr.end())
				{
					if (IsChangeSvr(*it, pSvr))
					{
						//更新配置
						mSvr.erase(it);
						mSvr.push_back(pSvr);
						*(pBuffer + j) = **it;	//Svr_t => SvrNet_t
						j++;
					}
				}
				else
				{
					//添加新配置
					mSvr.push_back(pSvr);
					*(pBuffer + j) = **it;	//Svr_t => SvrNet_t
					j++;
				}
			}
			else
			{
				LOG_ERROR("svr", "[modify svr] Parse svr config from svr.xml occur error: line(%d)!", i);
			}
		}
		//重新整理容器
		FixContainer();
		SetModTime();
	}
	else
	{
		LOG_ERROR("error", "[modify svr] Get SVRS node from svr.xml failed");
		return -1;
	}
	return j;
}

vector<Svr_t*>::iterator RouterConfig::GetItFromV(Svr_t* pSvr)
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

bool RouterConfig::IsChangeSvr(const Svr_t* pR1, const Svr_t* pR2)
{
	if (pR1->mGid!=pR2->mGid || pR1->mXid!=pR2->mXid || pR1->mSWeight!=pR2->mSWeight || pR1->mPort!=pR2->mPort || strncmp(pR1->mIp,pR2->mIp,MAX_SVR_IP_LEN)!=0)
	{
		return true;
	}
	return false;
}
