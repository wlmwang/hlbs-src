
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include <iostream>
#include "wType.h"
#include "wMisc.h"
#include "tinyxml.h"	//lib tinyxml
#include "RouterConfig.h"

/**
 * 解析配置
 */
void RouterConfig::ParseBaseConfig()
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

void RouterConfig::ParseSvrConfig()
{
	bool bLoadOK = mDoc->LoadFile("../config/svr.xml");
	if (!bLoadOK)
	{
		LOG_ERROR("error", "Load config file(../config/svr.xml) failed");
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
			const char *szName = pChildElm->Attribute("NAME");
			const char *szIPAddr = pChildElm->Attribute("IPADDRESS");
			const char *szPort = pChildElm->Attribute("PORT");
			const char *szDisabled = pChildElm->Attribute("DISABLED");
			const char *szWeight = pChildElm->Attribute("WEIGHT");
			if (szId != NULL && szGid != NULL && szXid != NULL && (szName != NULL && (strlen(szName) < MAX_SVR_NAME_LEN || strlen(szName) >= MIN_SVR_NAME_LEN)) && szIPAddr != NULL && szPort != NULL)
			{
				Svr_t *pSvr = new Svr_t();
				pSvr->mId = atoi(szId);
				pSvr->mGid = atoi(szGid);
				pSvr->mXid = atoi(szXid);
				pSvr->mPort = atoi(szPort);
				pSvr->mWeight = szWeight != NULL ? atoi(szWeight): 0;
				pSvr->mDisabled = szDisabled != NULL ? atoi(szDisabled) : 0;
				memcpy(pSvr->mName, szName, MAX_SVR_NAME_LEN);
				memcpy(pSvr->mIp, szIPAddr, MAX_SVR_IP_LEN);
				mSvr.push_back(pSvr);
			}
			else
			{
				LOG_ERROR("svr", "error svr config: line(%d)!", i);
			}
		}
	}
	else
	{
		LOG_ERROR("error", "Get svr from config file failed");
		exit(1);
	}
	
	FixSvr();
}

int RouterConfig::ReloadSvr(Svr_t* pBuffer, int iNum)
{
	CleanSvr();
	ParseSvrConfig();
	iNum = GetSvrAll(pBuffer, 0);
	return iNum;
}

//TODO.
int RouterConfig::SyncSvr(Svr_t* pBuffer, int iNum)
{
	CleanSvr();
	ParseSvrConfig();
	iNum = GetSvrAll(pBuffer, 0);
	return iNum;
}

int RouterConfig::GetSvrAll(Svr_t* pBuffer, int iNum)
{
	vector<Svr_t*>::iterator it = mSvr.begin();
	if(iNum == 0) iNum = mSvr.size();
	for(int i = 0; i < iNum && it != mSvr.end(); i++, it++)
	{
		*(pBuffer+i) = **it;
	}
	return iNum;
}

int RouterConfig::GetSvrById(Svr_t* pBuffer, int iId)
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

int RouterConfig::GetSvrByGid(Svr_t* pBuffer, int iGid, int iNum)
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

int RouterConfig::GetSvrByName(Svr_t* pBuffer, string sName, int iNum)
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

int RouterConfig::GetSvrByGXid(Svr_t* pBuffer, int iGid, int iXid, int iNum)
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

void RouterConfig::FixSvr()
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

void RouterConfig::CleanSvr()
{
	mSvrById.clear();
	mSvrByGid.clear();
	mSvrByName.clear();
	mSvrByGXid.clear();
	for(vector<Svr_t*>::iterator it = mSvr.begin(); it != mSvr.end(); it++)
	{
		SAFE_DELETE(*it);
	}
	mSvr.clear();
}

void RouterConfig::Final()
{
	CleanSvr();
	SAFE_DELETE(mDoc);
}
