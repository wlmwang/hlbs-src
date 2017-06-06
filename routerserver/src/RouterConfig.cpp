
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "RouterConfig.h"

const char kConfXml[]	= "../config/conf.xml";
const char kSvrXml[]	= "../config/svr.xml";
const char kAgntXml[]	= "../config/agent.xml";
const char kQosXml[]	= "../config/qos.xml";

RouterConfig::RouterConfig(): mSvrMtime(0), mAgntMtime(0), mSvrFile(kSvrXml), mQosFile(kQosXml), mBaseFile(kConfXml), mAgntFile(kAgntXml) {
	SAFE_NEW(SvrQos, mSvrQos);
}

RouterConfig::~RouterConfig() {
	SAFE_DELETE(mSvrQos);
}

int RouterConfig::WriteFileAgnt(const struct Agnt_t* agnt, int32_t n, const std::string& filename) {
	TiXmlDocument document;

	// xml文件头
	TiXmlDeclaration* declaration;
	SAFE_NEW(TiXmlDeclaration("1.0", "UTF-8", ""), declaration);
	document.LinkEndChild(declaration);
	
	// 创建ROOT节点
	TiXmlElement* root;
	SAFE_NEW(TiXmlElement("ROOT"), root);
	document.LinkEndChild(root);
	
	// 创建SVRS节点
	TiXmlElement* agnts;
	SAFE_NEW(TiXmlElement("AGENTS"), agnts);
	root->LinkEndChild(agnts);

	if (agnt == NULL) {
		int32_t start = 0, num = 0;
		struct Agnt_t agnt[kMaxNum];
		do {
			num = GetAgntAll(agnt, start, kMaxNum);
			if (num <= 0) {
				break;
			}
			for (int i = 0; i < num; i++) {
				if (agnt[i].mHost <= 0 || agnt[i].mPort <= 0) {
					continue;
				}
				TiXmlElement* node;
				SAFE_NEW(TiXmlElement("AGENT"), node);
				node->SetAttribute("HOST", agnt[i].mHost);
				node->SetAttribute("PORT", agnt[i].mPort);
				node->SetAttribute("VERSION", agnt[i].mVersion);
				node->SetAttribute("IDC", agnt[i].mIdc);
				agnts->LinkEndChild(node);
			}
			start += num;
		} while (num >= kMaxNum);
	} else if (agnt && n > 0) {
		for (int32_t i = 0; i < n; i++) {
			if (agnt[i].mHost <= 0 || agnt[i].mPort <= 0) {
				continue;
			}
			TiXmlElement* node;
			SAFE_NEW(TiXmlElement("AGENT"), node);
			node->SetAttribute("HOST", agnt[i].mHost);
			node->SetAttribute("PORT", agnt[i].mPort);
			node->SetAttribute("VERSION", agnt[i].mVersion);
			node->SetAttribute("IDC", agnt[i].mIdc);
			agnts->LinkEndChild(node);
		}
	}

	if (filename.size() == 0) {
		document.SaveFile(mAgntFile.c_str());
	} else {
		document.SaveFile(filename.c_str());
	}
	return 0;
}

int RouterConfig::WriteFileSvr(const struct SvrNet_t* svr, int32_t n, const std::string& filename) {
	TiXmlDocument document;

	// xml文件头
	TiXmlDeclaration* declaration;
	SAFE_NEW(TiXmlDeclaration("1.0", "UTF-8", ""), declaration);
	document.LinkEndChild(declaration);
	
	// 创建ROOT节点
	TiXmlElement* root;
	SAFE_NEW(TiXmlElement("ROOT"), root);
	document.LinkEndChild(root);
	
	// 创建SVRS节点
	TiXmlElement* svrs;
	SAFE_NEW(TiXmlElement("SVRS"), svrs);
	root->LinkEndChild(svrs);

	if (svr == NULL) {
		int32_t start = 0, num = 0;
		struct SvrNet_t svr[kMaxNum];
		do {
			if (!mSvrQos->GetNodeAll(svr, &num, start, kMaxNum).Ok() || num <= 0) {
				break;
			}
			for (int i = 0; i < num; i++) {
				if (svr[i].mGid <= 0 || svr[i].mXid <= 0 || svr[i].mPort <= 0) {
					continue;
				}
				TiXmlElement* node;
				SAFE_NEW(TiXmlElement("SVR"), node);
				node->SetAttribute("GID", svr[i].mGid);
				node->SetAttribute("XID", svr[i].mXid);
				node->SetAttribute("HOST", svr[i].mHost);
				node->SetAttribute("PORT", svr[i].mPort);
				node->SetAttribute("WEIGHT", svr[i].mWeight);
				node->SetAttribute("VERSION", svr[i].mVersion);
				node->SetAttribute("NAME", svr[i].mName);
				node->SetAttribute("IDC", svr[i].mIdc);
				svrs->LinkEndChild(node);
			}
			start += num;
		} while (num >= kMaxNum);
	} else if (svr && n > 0) {
		for (int32_t i = 0; i < n; i++) {
			if (svr[i].mGid <= 0 || svr[i].mXid <= 0 || svr[i].mPort <= 0) {
				continue;
			}
			TiXmlElement* node;
			SAFE_NEW(TiXmlElement("SVR"), node);
			node->SetAttribute("GID", svr[i].mGid);
			node->SetAttribute("XID", svr[i].mXid);
			node->SetAttribute("HOST", svr[i].mHost);
			node->SetAttribute("PORT", svr[i].mPort);
			node->SetAttribute("WEIGHT", svr[i].mWeight);
			node->SetAttribute("VERSION", svr[i].mVersion);
			node->SetAttribute("NAME", svr[i].mName);
			node->SetAttribute("IDC", svr[i].mIdc);
			svrs->LinkEndChild(node);
		}
	}

	if (filename.size() == 0) {
		document.SaveFile(mSvrFile.c_str());
	} else {
		document.SaveFile(filename.c_str());
	}
	return 0;
}

int RouterConfig::ParseBaseConf() {
	TiXmlDocument document;
	if (!document.LoadFile(mBaseFile.c_str())) {
		LOG_ERROR(soft::GetLogPath(), "%s : %s", "RouterConfig::ParseBaseConf Load configure(conf.xml) file failed", "");
		return -1;
	}
	TiXmlElement *pRoot = document.FirstChildElement();
	if (NULL == pRoot) {
		LOG_ERROR(soft::GetLogPath(), "%s : %s", "RouterConfig::ParseBaseConf Read root from configure(conf.xml) failed", "");
		return -1;
	}
	
	// 服务器侦听地址
	const TiXmlElement* pElement = pRoot->FirstChildElement("SERVER");
	if (NULL != pElement) {
		const char* host = pElement->Attribute("HOST");
		const char* port = pElement->Attribute("PORT");
		const char* worker = pElement->Attribute("WORKER");
		const char* protocol = pElement->Attribute("PROTOCOL");
		if (host != NULL && port != NULL) {
			SetStrConf("host", host);
			SetIntConf("port", atoi(port));
			SetIntConf("worker", atoi(worker));
			SetStrConf("protocol", protocol);
		} else {
			LOG_ERROR(soft::GetLogPath(), "%s : %s", "RouterConfig::ParseBaseConf Get SERVER host or port from conf.xml failed", "");
			return -1;
		}
	} else {
		LOG_ERROR(soft::GetLogPath(), "%s : %s", "RouterConfig::ParseBaseConf Get SERVER node from conf.xml failed", "");
		return -1;
	}

	// RestFul侦听地址
	pElement = pRoot->FirstChildElement("CONTROL");
	if (NULL != pElement) {
		const char* host = pElement->Attribute("HOST");
		const char* port = pElement->Attribute("PORT");
		const char* protocol = pElement->Attribute("PROTOCOL");
		if (host != NULL && port != NULL) {
			SetStrConf("control_host", host);
			SetIntConf("control_port", atoi(port));
			SetStrConf("control_protocol", protocol);
		} else {
			LOG_ERROR(soft::GetLogPath(), "%s : %s", "RouterConfig::ParseBaseConf Get CONTROL host or port from conf.xml failed", "");
			return -1;
		}
	} else {
		LOG_ERROR(soft::GetLogPath(), "%s : %s", "RouterConfig::ParseBaseConf Get CONTROL node from conf.xml failed", "");
		return -1;
	}
	return 0;
}

int RouterConfig::ParseSvrConf() {
	TiXmlDocument document;
	if (!document.LoadFile(mSvrFile.c_str())) {
		LOG_ERROR(soft::GetLogPath(), "%s : %s", "RouterConfig::ParseSvrConf Load configure(svr.xml) file failed", "");
		return -1;
	}
	TiXmlElement *pRoot = document.FirstChildElement();
	if (NULL == pRoot) {
		LOG_ERROR(soft::GetLogPath(), "%s : %s", "RouterConfig::ParseSvrConf Read root from configure(svr.xml) failed", "");
		return -1;
	}
	
	// SVR配置
	TiXmlElement* pElement = pRoot->FirstChildElement("SVRS");
	if (pElement != NULL) {
		int i = 0;
		for (TiXmlElement* pChildElm = pElement->FirstChildElement(); pChildElm != NULL ; pChildElm = pChildElm->NextSiblingElement(), i++) {
			const char* gid = pChildElm->Attribute("GID");
			const char* xid = pChildElm->Attribute("XID");
			const char* host = pChildElm->Attribute("HOST");
			const char* port = pChildElm->Attribute("PORT");
			const char* weight = pChildElm->Attribute("WEIGHT");
			const char* version = pChildElm->Attribute("VERSION");
			const char* name = pChildElm->Attribute("NAME");
			const char* idc = pChildElm->Attribute("IDC");
			if (gid != NULL && xid != NULL && host != NULL && port != NULL) {
				struct SvrNet_t svr;
				svr.mGid = atoi(gid);
				svr.mXid = atoi(xid);
				svr.mPort = atoi(port);
				memcpy(svr.mHost, host, kMaxHost);
				if (idc != NULL) {
					svr.mIdc = atoi(idc);
				}
				if (version != NULL) {
					svr.mVersion = atoi(version);
				}
				if (weight != NULL) {
					svr.mWeight = atoi(weight);
				}
				if (name != NULL) {
					memcpy(svr.mName, name, kMaxName);
				}

				// 添加新配置
				if (svr.mWeight > 0 && !mSvrQos->IsExistNode(svr)) {
					mSvrQos->SaveNode(svr);
				} else {
					LOG_ERROR(kSvrLog, "RouterConfig::ParseSvrConf Parse configure from svr.xml occur error(weight<=0 or exists this SvrNet_t), line : %d", i);
				}
			} else {
				LOG_ERROR(kSvrLog, "RouterConfig::ParseSvrConf Parse configure from svr.xml occur error, line : %d", i);
			}
		}
	} else {
		LOG_ERROR(soft::GetLogPath(), "%s : %s", "RouterConfig::ParseSvrConf Get SVRS node from svr.xml failed", "");
		return -1;
	}
	return SetSvrMtime();
}

int RouterConfig::ParseAgntConf() {
	TiXmlDocument document;
	if (!document.LoadFile(mAgntFile.c_str())) {
		LOG_ERROR(soft::GetLogPath(), "%s : %s", "RouterConfig::ParseAgntConf Load configure(agent.xml) file failed", "");
		return -1;
	}
	TiXmlElement *pRoot = document.FirstChildElement();
	if (NULL == pRoot) {
		LOG_ERROR(soft::GetLogPath(), "%s : %s", "RouterConfig::ParseAgntConf Read root from configure(agent.xml) failed", "");
		return -1;
	}
	
	// AGENT配置
	TiXmlElement* pElement = pRoot->FirstChildElement("AGENTS");
	if (pElement != NULL) {
		int i = 0;
		for (TiXmlElement* pChildElm = pElement->FirstChildElement(); pChildElm != NULL ; pChildElm = pChildElm->NextSiblingElement(), i++) {
			const char* host = pChildElm->Attribute("HOST");
			const char* port = pChildElm->Attribute("PORT");
			const char* version = pChildElm->Attribute("VERSION");
			const char* idc = pChildElm->Attribute("IDC");
			if (host != NULL && port != NULL) {
				struct Agnt_t agnt;
				agnt.mPort = atoi(port);
				memcpy(agnt.mHost, host, kMaxHost);
				if (idc != NULL) {
					agnt.mIdc = atoi(idc);
				}
				if (version != NULL) {
					agnt.mVersion = atoi(version);
				}

				// 添加新配置
				if (!IsExistAgnt(agnt)) {
					SaveAgnt(agnt);
				} else {
					LOG_ERROR(kSvrLog, "RouterConfig::ParseAgntConf Parse configure from agent.xml occur error(exists this Agnt_t), line : %d", i);
				}
			} else {
				LOG_ERROR(kSvrLog, "RouterConfig::ParseAgntConf Parse configure from agent.xml occur error, line : %d", i);
			}
		}
	} else {
		LOG_ERROR(soft::GetLogPath(), "%s : %s", "RouterConfig::ParseAgntConf Get AGENTS node from agent.xml failed", "");
		return -1;
	}
	return SetAgntMtime();
}

int RouterConfig::ParseModifySvr() {
	TiXmlDocument document;
	if (!document.LoadFile(mSvrFile.c_str())) {
		LOG_ERROR(soft::GetLogPath(), "%s : %s", "RouterConfig::ParseModifySvr Load configure(svr.xml) file failed", "");
		return -1;
	}
	TiXmlElement *pRoot = document.FirstChildElement();
	if (NULL == pRoot) {
		LOG_ERROR(soft::GetLogPath(), "%s : %s", "RouterConfig::ParseModifySvr Read root from configure(svr.xml) failed", "");
		return -1;
	}

	TiXmlElement* pElement = pRoot->FirstChildElement("SVRS");
	if (pElement != NULL) {
		int i = 0;
		for (TiXmlElement* pChildElm = pElement->FirstChildElement(); pChildElm != NULL ; pChildElm = pChildElm->NextSiblingElement(), i++) {
			const char* gid = pChildElm->Attribute("GID");
			const char* xid = pChildElm->Attribute("XID");
			const char* host = pChildElm->Attribute("HOST");
			const char* port = pChildElm->Attribute("PORT");
			const char* weight = pChildElm->Attribute("WEIGHT");
			const char* version = pChildElm->Attribute("VERSION");
			const char* name = pChildElm->Attribute("NAME");
			const char* idc = pChildElm->Attribute("IDC");
			if (gid != NULL && xid != NULL && host != NULL && port != NULL) {
				struct SvrNet_t svr;
				svr.mGid = atoi(gid);
				svr.mXid = atoi(xid);
				svr.mPort = atoi(port);
				memcpy(svr.mHost, host, kMaxHost);
				if (idc != NULL) {
					svr.mIdc = atoi(idc);
				}
				if (version != NULL) {
					svr.mVersion = atoi(version);
				}
				if (weight != NULL) {
					svr.mWeight = atoi(weight);
				}
				if (name != NULL) {
					memcpy(svr.mName, name, kMaxName);
				}

				if (svr.mWeight >= 0 && mSvrQos->IsExistNode(svr)) {	// 旧配置检测到weight、name、idc变化才更新
					if (mSvrQos->IsWNIChange(svr)) {
						mSvrQos->SaveNode(svr);
					}
				} else if (svr.mWeight > 0) {	// 添加新配置 weight>0 添加配置
					mSvrQos->SaveNode(svr);
				}
			} else {
				LOG_ERROR(kSvrLog, "RouterConfig::ParseModifySvr Parse configure from svr.xml occur error, line : %d", i);
			}
		}
		return SetSvrMtime();
	}
	return -1;
}

int RouterConfig::ParseQosConf() {
	TiXmlDocument document;
	if (!document.LoadFile(mQosFile.c_str())) {
		LOG_ERROR(soft::GetLogPath(), "%s : %s", "RouterConfig::ParseQosConf Load configure(qos.xml) file failed", "");
		return -1;
	}
	TiXmlElement *pRoot = document.FirstChildElement();
	if (NULL == pRoot) {
		LOG_ERROR(soft::GetLogPath(), "%s : %s", "RouterConfig::ParseQosConf Read root from configure(qos.xml) failed", "");
		return -1;
	}
	
	// 成功率、时延比例配置
	TiXmlElement* pElement = pRoot->FirstChildElement("FACTOR");
	if (NULL != pElement) {
		if (pElement->Attribute("RATE_WEIGHT") != NULL) {
			mSvrQos->mRateWeight = atoi(pElement->Attribute("RATE_WEIGHT"));
		} else {
			mSvrQos->mRateWeight = 7;
		}
		if (pElement->Attribute("DELAY_WEIGHT") != NULL) {
			mSvrQos->mDelayWeight = atoi(pElement->Attribute("DELAY_WEIGHT"));
		} else {
			mSvrQos->mDelayWeight = 1;
		}
	}
	
	//  路由重建时间
	pElement = pRoot->FirstChildElement("CFG");
	if (NULL != pElement) {
		if (pElement->Attribute("REBUILD") != NULL) {
			mSvrQos->mRebuildTm = atoi(pElement->Attribute("REBUILD"));
		} else {
			mSvrQos->mRebuildTm = 60;
		}
	}

	// 访问量配置
	pElement = pRoot->FirstChildElement("REQ");
	if (NULL != pElement) {
		if (pElement->Attribute("REQ_MAX") != NULL) {
			mSvrQos->mReqCfg.mReqMax = atoi(pElement->Attribute("REQ_MAX"));
		} else {
			mSvrQos->mReqCfg.mReqMax = 100000;
		}

		if (pElement->Attribute("REQ_MIN") != NULL) {
			mSvrQos->mReqCfg.mReqMin = atoi(pElement->Attribute("REQ_MIN"));
		} else {
			mSvrQos->mReqCfg.mReqMin = 10;
		}

		if (pElement->Attribute("REQ_ERR_MIN") != NULL) {
			mSvrQos->mReqCfg.mReqErrMin = atof(pElement->Attribute("REQ_ERR_MIN"));
		} else {
			mSvrQos->mReqCfg.mReqErrMin = 0.5;
		}

		if (pElement->Attribute("REQ_EXTEND_RATE") != NULL) {
			mSvrQos->mReqCfg.mReqExtendRate = atof(pElement->Attribute("REQ_EXTEND_RATE"));
		} else {
			mSvrQos->mReqCfg.mReqExtendRate = 0.2;
		}

		if (pElement->Attribute("PRE_TIME") != NULL) {
			mSvrQos->mReqCfg.mPreTime = atoi(pElement->Attribute("PRE_TIME"));
		} else {
			mSvrQos->mReqCfg.mPreTime = 4;
		}
	}
	mSvrQos->mReqCfg.mRebuildTm = mSvrQos->mRebuildTm;

	// 宕机配置
	pElement = pRoot->FirstChildElement("DOWN");
	if (NULL != pElement) {
		if (pElement->Attribute("PROBE_BEGIN") != NULL) {
			mSvrQos->mDownCfg.mProbeBegin = atoi(pElement->Attribute("PROBE_BEGIN"));
		} else {
			mSvrQos->mDownCfg.mProbeBegin = 1;
		}

		if (pElement->Attribute("PROBE_TRIGER_REQ") != NULL) {
			mSvrQos->mDownCfg.mReqCountTrigerProbe = atoi(pElement->Attribute("PROBE_TRIGER_REQ"));
		} else {
			mSvrQos->mDownCfg.mReqCountTrigerProbe = 10000;
		}

		if (pElement->Attribute("PROBE_TIMES") != NULL) {
			mSvrQos->mDownCfg.mProbeTimes = atoi(pElement->Attribute("PROBE_TIMES"));
		} else {
			mSvrQos->mDownCfg.mProbeTimes = 3;
		}

		if (pElement->Attribute("PROBE_INTERVAL") != NULL) {
			mSvrQos->mDownCfg.mProbeInterval = atoi(pElement->Attribute("PROBE_INTERVAL"));
		} else {
			mSvrQos->mDownCfg.mProbeInterval = 10;
		}

		if (pElement->Attribute("PROBE_EXPIRED") != NULL) {
			mSvrQos->mDownCfg.mProbeNodeExpireTime = atoi(pElement->Attribute("PROBE_EXPIRED"));
		} else {
			mSvrQos->mDownCfg.mProbeNodeExpireTime = 600;
		}

		if (pElement->Attribute("DOWN_TIME") != NULL) {
			mSvrQos->mDownCfg.mDownTimeTrigerProbe = atoi(pElement->Attribute("DOWN_TIME"));
		} else {
			mSvrQos->mDownCfg.mDownTimeTrigerProbe = 600;
		}

		if (pElement->Attribute("DOWN_ERR_REQ") != NULL) {
			mSvrQos->mDownCfg.mPossibleDownErrReq = atoi(pElement->Attribute("DOWN_ERR_REQ"));
		} else {
			mSvrQos->mDownCfg.mPossibleDownErrReq = 10;
		}

		if (pElement->Attribute("DOWN_ERR_RATE") != NULL) {
			mSvrQos->mDownCfg.mPossbileDownErrRate = atof(pElement->Attribute("DOWN_ERR_RATE"));
		} else {
			mSvrQos->mDownCfg.mPossbileDownErrRate = 0.5;
		}
	}

	// 校验配置
	// rate 需在 0.01-100 之间
    float rate = static_cast<float>(mSvrQos->mRateWeight) / mSvrQos->mDelayWeight;
    if (rate > 100000) {
        mSvrQos->mRateWeight = 100000;
        mSvrQos->mDelayWeight = 1;
    } else if (rate < 0.00001) {
        mSvrQos->mDelayWeight = 100000;
        mSvrQos->mRateWeight = 1;
    }

	if (!(mSvrQos->mReqCfg.mReqExtendRate > 0.001 && mSvrQos->mReqCfg.mReqExtendRate < 101)) {
		LOG_ERROR(soft::GetLogPath(), "RouterConfig::ParseQosConf invalid !((REQ_EXTEND_RATE[%f] > 0.001) && (REQ_EXTEND_RATE[%f] < 101))", mSvrQos->mReqCfg.mReqExtendRate, mSvrQos->mReqCfg.mReqExtendRate);
		return -1;
	} else if (mSvrQos->mReqCfg.mReqErrMin >= 1) {
		LOG_ERROR(soft::GetLogPath(), "RouterConfig::ParseQosConf invalid REQ_ERR_MIN[%f] > 1", mSvrQos->mReqCfg.mReqErrMin);
		return -1;
	} else if (mSvrQos->mDownCfg.mPossbileDownErrRate > 1 || mSvrQos->mDownCfg.mPossbileDownErrRate < 0.01) {
		LOG_ERROR(soft::GetLogPath(), "RouterConfig::ParseQosConf invalid DOWN_ERR_RATE[%f] > 1 || DOWN_ERR_RATE[%f] < 0.01", mSvrQos->mDownCfg.mPossbileDownErrRate, mSvrQos->mDownCfg.mPossbileDownErrRate);
		return -1;
	} else if (mSvrQos->mDownCfg.mProbeTimes < 3) {
		LOG_ERROR(soft::GetLogPath(), "RouterConfig::ParseQosConf invalid TIMES[%d] < 3", mSvrQos->mDownCfg.mProbeTimes);
		return -1;
	} else if (mSvrQos->mReqCfg.mRebuildTm < 3) {
		LOG_ERROR(soft::GetLogPath(), "RouterConfig::ParseQosConf invalid REBUILD_TM[%d] < 3", mSvrQos->mReqCfg.mRebuildTm);
		return -1;
	}

    if (mSvrQos->mReqCfg.mPreTime <= 0 || mSvrQos->mReqCfg.mPreTime > (mSvrQos->mRebuildTm / 2)) {
		mSvrQos->mReqCfg.mPreTime = 2;
	}
	return 0;
}

int RouterConfig::SetSvrMtime() {
	struct stat stBuf;
	if (stat(mSvrFile.c_str(), &stBuf) == 0) {
		mSvrMtime = stBuf.st_mtime;
		return 0;
	}
	return -1;
}

bool RouterConfig::IsModifySvr() {
	struct stat stBuf;
	if (stat(mSvrFile.c_str(), &stBuf) == 0 && stBuf.st_mtime > mSvrMtime) {
		return true;
	}
	return false;
}

int RouterConfig::SetAgntMtime() {
	struct stat stBuf;
	if (stat(mAgntFile.c_str(), &stBuf) == 0) {
		mAgntMtime = stBuf.st_mtime;
		return 0;
	}
	return -1;
}

bool RouterConfig::IsModifyAgnt() {
	struct stat stBuf;
	if (stat(mAgntFile.c_str(), &stBuf) == 0 && stBuf.st_mtime > mAgntMtime) {
		return true;
	}
	return false;
}

bool RouterConfig::IsExistAgnt(const struct Agnt_t& agnt) {
	if (std::find(mAgnts.begin(), mAgnts.end(), agnt) == mAgnts.end()) {
		return false;
	}
	return true;
}

bool RouterConfig::IsAgntChange(const struct Agnt_t& agnt) {
	std::vector<struct Agnt_t>::iterator it = std::find(mAgnts.begin(), mAgnts.end(), agnt);
	if (it != mAgnts.end()) {
		const struct Agnt_t& k = *it;
		if (k.mStatus == agnt.mStatus && k.mIdc == agnt.mIdc) {
			return false;
		}
	}
	return true;
}

bool RouterConfig::DelAgnt(const struct Agnt_t& agnt) {
	std::vector<struct Agnt_t>::iterator it = std::find(mAgnts.begin(), mAgnts.end(), agnt);
	if (it == mAgnts.end()) {
		LOG_ERROR(kAgntLog, "RouterConfig::DelAgnt delete Agnt_t failed(cannot find the Agnt_t), HOST(%s),PORT(%d)", agnt.mHost, agnt.mPort);
		return false;
	}
	mAgnts.erase(it);
	return true;
}

bool RouterConfig::SaveAgnt(const struct Agnt_t& agnt) {
	if (IsExistAgnt(agnt)) {
		return ModifyAgnt(agnt);
	}
	return AddAgnt(agnt);
}

bool RouterConfig::ModifyAgnt(const struct Agnt_t& agnt) {
	std::vector<struct Agnt_t>::iterator it = std::find(mAgnts.begin(), mAgnts.end(), agnt);
	if (it == mAgnts.end()) {
		LOG_ERROR(kAgntLog, "RouterConfig::ModifyAgnt delete Agnt_t failed(cannot find the Agnt_t), HOST(%s),PORT(%d)", agnt.mHost, agnt.mPort);
		return false;
	}

	LOG_DEBUG(kAgntLog, "RouterConfig::ModifyAgnt modify Agnt_t success, HOST(%s),PORT(%d)", agnt.mHost, agnt.mPort);
	struct Agnt_t& oldagnt = const_cast<struct Agnt_t&>(*it);
	oldagnt = agnt;
	return true;
}

bool RouterConfig::AddAgnt(const struct Agnt_t& agnt) {
	LOG_DEBUG(kAgntLog, "RouterConfig::AddAgnt add Agnt_t success, HOST(%s),PORT(%d)", agnt.mHost, agnt.mPort);
	mAgnts.push_back(agnt);
	return true;
}

bool RouterConfig::CleanAgnt() {
	LOG_DEBUG(kAgntLog, "RouterConfig::CleanAgnt clean Agnt_t success");
	mAgnts.clear();
	return true;
}

int RouterConfig::GetAgntAll(struct Agnt_t buf[], int32_t start, int32_t size) {
	LOG_DEBUG(kAgntLog, "RouterConfig::GetAgntAll get all Agnt_t start, [%d, %d]", start, size);

	int num = 0;
	std::vector<struct Agnt_t>::iterator it = mAgnts.begin();
	for (std::advance(it, start); it != mAgnts.end() && num < size; it++) {
		buf[num++] = *it;
	}
	return num;
}
