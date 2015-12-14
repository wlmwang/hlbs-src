
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
		printf("Get Server ip and port from config file failed\n");
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
			
			Rtbl_t *pRtbl = new Rtbl_t();
			pRtbl->mId = atoi(szId);
			pRtbl->mGid = atoi(szGid);
			pRtbl->mXid = atoi(szXid);
			pRtbl->mPort = atoi(szPort);
			pRtbl->mWeight = atoi(szWeight);
			pRtbl->mDisabled = atoi(szDisabled);
			memcpy(pRtbl->mName, szName, MAX_NAME_LEN);
			memcpy(pRtbl->mIp, szIPAddr, MAX_IP_LEN);
			
			mRtbl.push_back(pRtbl);
		}
	}
	else
	{
		printf("Get extranet connect ip and port from config file failed\n");
		exit(1);
	}
	
	FixRtbl();
}

//整理容器
void RouterConfig::FixRtbl()
{
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

Rtbl_t RouterConfig::GetRtblById(int iId)
{
	Rtbl_t vRtbl;
	map<int, Rtbl_t*>::iterator it = mRtblById.find(iId);
	if(it != mRtblById.end())
	{
		vRtbl = *(it->second);
	}
	return vRtbl;
}

int RouterConfig::GetRtblAll(Rtbl_t* pBuffer, int iNum)
{
	vector<Rtbl_t*>::iterator it = mRtbl.begin();
	if(iNum == 0) iNum = mRtbl.size();
	for(int i = 0; i < iNum && it != mRtbl.end(); i++, it++)
	{
		*(pBuffer+i) = **it;
	}
	return iNum;
}

int RouterConfig::GetRtblByGid(Rtbl_t* pBuffer, int iGid, int iNum)
{
	vector<Rtbl_t*> vRtbl;
	map<int, vector<Rtbl_t*> >::iterator mn = mRtblByGid.find(iGid);
	if(mn != mRtblByGid.end())
	{
		vRtbl = mn->second;
		
		if(iNum == 0) iNum = vRtbl.size();
		vector<Rtbl_t*>::iterator it = vRtbl.begin();
		for(int i = 0; i < iNum && it != vRtbl.end(); i++, it++)
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

int RouterConfig::GetRtblByName(Rtbl_t* pBuffer, string sName, int iNum)
{
	vector<Rtbl_t*> vRtbl;
	map<string, vector<Rtbl_t*> >::iterator mn = mRtblByName.find(sName);
	if(mn != mRtblByName.end())
	{
		vRtbl = mn->second;
		if(iNum == 0) iNum = vRtbl.size();
		
		vector<Rtbl_t*>::iterator it = vRtbl.begin();
		for(int i = 0; i < iNum && it != vRtbl.end(); i++, it++)
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

int RouterConfig::GetRtblByGXid(Rtbl_t* pBuffer, int iGid, int iXid, int iNum)
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
		for(int i = 0; i < iNum && it != vRtbl.end(); i++, it++)
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
