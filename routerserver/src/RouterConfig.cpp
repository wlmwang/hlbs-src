
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
	const char* filename = "../config/conf.xml";
	bool bLoadOK = mDoc->LoadFile(filename);
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
	const char* filename = "../config/svr.xml";
	bool bLoadOK = mDoc->LoadFile(filename);
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
				pSvr->mDisabled = szDisabled != NULL ? atoi(szDisabled) : pSvr->mDisabled;
				pSvr->mWeight = szWeight != NULL ? atof(szWeight): pSvr->mWeight;
				memcpy(pSvr->mName, szName, MAX_SVR_NAME_LEN);
				memcpy(pSvr->mIp, szIPAddr, MAX_SVR_IP_LEN);
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

int RouterConfig::ReloadSvr(Svr_t* pBuffer)
{
	Final();
	ParseSvrConfig();
	return GetSvrAll(pBuffer);
}

//TODO.
int RouterConfig::SyncSvr(Svr_t* pBuffer)
{
	Final();
	ParseSvrConfig();
	return GetSvrAll(pBuffer);
}

int RouterConfig::GetSvrAll(Svr_t* pBuffer)
{
	vector<Svr_t*>::iterator it = mSvr.begin();
	for(int i = 0; it != mSvr.end(); i++, it++)
	{
		*(pBuffer+i) = **it;
	}
	return mSvr.size();
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

void RouterConfig::FixContainer()
{
	if(mSvr.size() <= 0) return;
	sort(mSvr.begin(), mSvr.end(), GreaterSvr);//降序排序
	
	DelContainer();
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

void RouterConfig::DelContainer()
{
	mSvrById.clear();
	mSvrByGid.clear();
	mSvrByName.clear();
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
	const char* filename = "../config/svr.xml";
	struct stat stBuf;
	int iRet = stat(filename, &stBuf);
	if (iRet == 0)
	{
		mMtime = stBuf.st_mtime;
		return mMtime;
	}
	return iRet; //-1
}

bool RouterConfig::IsModTime()
{
	const char* filename = "../config/svr.xml";
	struct stat stBuf;
	int iRet = stat(filename, &stBuf);
	if (iRet == 0 && stBuf.st_mtime > mMtime)
	{
		return true;
	}
	return false;
}

//获取修改的svr
//不能删除节点（可修改Disabled=1属性，达到删除节点效果）
int RouterConfig::GetModSvr(Svr_t* pBuffer)
{
	const char* filename = "../config/svr.xml";
	bool bLoadOK = mDoc->LoadFile(filename);
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
				pSvr->mDisabled = szDisabled != NULL ? atoi(szDisabled) : pSvr->mDisabled;
				pSvr->mWeight = szWeight != NULL ? atof(szWeight): pSvr->mWeight;
				memcpy(pSvr->mName, szName, MAX_SVR_NAME_LEN);
				memcpy(pSvr->mIp, szIPAddr, MAX_SVR_IP_LEN);

				vector<Svr_t*>::iterator it = GetItById(pSvr->mId);
				if (it != mSvr.end())
				{
					if (pSvr->mGid != (*it)->mGid || pSvr->mXid != (*it)->mXid || pSvr->mDisabled != (*it)->mDisabled || pSvr->mWeight != (*it)->mWeight || pSvr->mPort != (*it)->mPort || strcpy(pSvr->mIp,(*it)->mIp) != 0 || strcpy(pSvr->mName,(*it)->mName) != 0)
					{
						//更新配置
						mSvr.erase(it);
						mSvr.push_back(pSvr);
						*(pBuffer + j) = **it;
						j++;
					}
				}
				else
				{
					//添加新配置
					mSvr.push_back(pSvr);
					*(pBuffer + j) = **it;
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

vector<Svr_t*>::iterator RouterConfig::GetItById(int iId)
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
