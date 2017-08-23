
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "Define.h"
#include "RouterConfig.h"

const char kConfXml[]	= "../config/conf.xml";
const char kSvrXml[]	= "../config/svr.xml";
const char kAgntXml[]	= "../config/agent.xml";
const char kRltXml[]	= "../config/relation.xml";
const char kQosXml[]	= "../config/qos.xml";

RouterConfig::RouterConfig(): mSvrFile(kSvrXml),mQosFile(kQosXml),mBaseFile(kConfXml),mAgntFile(kAgntXml),mRltFile(kRltXml) {
	HNET_NEW(SvrQos, mSvrQos);
}

RouterConfig::~RouterConfig() {
	HNET_DELETE(mSvrQos);
}

int RouterConfig::WriteFileSvr(const struct SvrNet_t* svr, int32_t n, const std::string& filename) {
	TiXmlDocument document;

	// xml文件头
	TiXmlDeclaration* declaration;
	HNET_NEW(TiXmlDeclaration("1.0", "UTF-8", ""), declaration);
	document.LinkEndChild(declaration);
	
	// 创建ROOT节点
	TiXmlElement* root;
	HNET_NEW(TiXmlElement("ROOT"), root);
	document.LinkEndChild(root);
	
	// 创建SVRS节点
	TiXmlElement* svrs;
	HNET_NEW(TiXmlElement("SVRS"), svrs);
	root->LinkEndChild(svrs);

	if (!svr) {
		int32_t start = 0, num = 0, j = 0;
		struct SvrNet_t svr[kMaxNum];
		do {
			num = mSvrQos->GetNodeAll(svr, j, start, kMaxNum);
			if (num <= 0) {
				break;
			}

			for (int i = 0; i < num; i++) {
				if (svr[i].mGid <= 0 || svr[i].mXid <= 0 || svr[i].mPort <= 0 || svr[i].mWeight <= 0) {
					continue;
				}
				TiXmlElement* node;
				HNET_NEW(TiXmlElement("SVR"), node);
				node->SetAttribute("GID", svr[i].mGid);
				node->SetAttribute("XID", svr[i].mXid);
				node->SetAttribute("HOST", svr[i].mHost);
				node->SetAttribute("PORT", svr[i].mPort);
				node->SetAttribute("WEIGHT", svr[i].mWeight);
				node->SetAttribute("NAME", svr[i].mName);
				node->SetAttribute("IDC", svr[i].mIdc);
				svrs->LinkEndChild(node);
			}
			start += num;
			j = 0;
		} while (num >= kMaxNum);

	} else if (svr && n > 0) {
		for (int32_t i = 0; i < n; i++) {
			if (svr[i].mGid <= 0 || svr[i].mXid <= 0 || svr[i].mPort <= 0 || svr[i].mWeight <= 0) {
				continue;
			}
			TiXmlElement* node;
			HNET_NEW(TiXmlElement("SVR"), node);
			node->SetAttribute("GID", svr[i].mGid);
			node->SetAttribute("XID", svr[i].mXid);
			node->SetAttribute("HOST", svr[i].mHost);
			node->SetAttribute("PORT", svr[i].mPort);
			node->SetAttribute("WEIGHT", svr[i].mWeight);
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

int RouterConfig::WriteFileAgnt(const struct Agnt_t* agnt, int32_t n, const std::string& filename) {
	TiXmlDocument document;

	// xml文件头
	TiXmlDeclaration* declaration;
	HNET_NEW(TiXmlDeclaration("1.0", "UTF-8", ""), declaration);
	document.LinkEndChild(declaration);
	
	// 创建ROOT节点
	TiXmlElement* root;
	HNET_NEW(TiXmlElement("ROOT"), root);
	document.LinkEndChild(root);
	
	// 创建SVRS节点
	TiXmlElement* agnts;
	HNET_NEW(TiXmlElement("AGENTS"), agnts);
	root->LinkEndChild(agnts);

	if (!agnt) {
		for (std::vector<struct Agnt_t>::iterator it = mAgnts.begin(); it != mAgnts.end(); it++) {
			if (it->mHost[0] <= 0 || it->mConfig == -1 || it->mWeight <= 0) {
				continue;
			}
			TiXmlElement* node;
			HNET_NEW(TiXmlElement("AGENT"), node);
			node->SetAttribute("HOST", it->mHost);
			node->SetAttribute("PORT", it->mPort);
			node->SetAttribute("WEIGHT", it->mWeight);
			node->SetAttribute("IDC", it->mIdc);
			node->SetAttribute("NAME", it->mName);
			agnts->LinkEndChild(node);
		}
	} else if (agnt && n > 0) {
		for (int32_t i = 0; i < n; i++) {
			if (agnt[i].mHost[0] <= 0 || agnt[i].mConfig == -1 || agnt[i].mWeight <= 0) {
				continue;
			}
			TiXmlElement* node;
			HNET_NEW(TiXmlElement("AGENT"), node);
			node->SetAttribute("HOST", agnt[i].mHost);
			node->SetAttribute("PORT", agnt[i].mPort);
			node->SetAttribute("WEIGHT", agnt[i].mWeight);
			node->SetAttribute("IDC", agnt[i].mIdc);
			node->SetAttribute("NAME", agnt[i].mName);
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

int RouterConfig::WriteFileRlt(const struct Rlt_t* rlt, int32_t n, const std::string& filename) {
	TiXmlDocument document;

	// xml文件头
	TiXmlDeclaration* declaration;
	HNET_NEW(TiXmlDeclaration("1.0", "UTF-8", ""), declaration);
	document.LinkEndChild(declaration);
	
	// 创建ROOT节点
	TiXmlElement* root;
	HNET_NEW(TiXmlElement("ROOT"), root);
	document.LinkEndChild(root);
	
	// 创建SVRS节点
	TiXmlElement* rlts;
	HNET_NEW(TiXmlElement("RELATIONS"), rlts);
	root->LinkEndChild(rlts);

	if (!rlt) {
		for (RouterConfig::MapRltIt_t it = mRelations.begin(); it != mRelations.end(); it++) {
			if (it->second.empty()) {
				continue;
			}
			for (RouterConfig::vRltIt_t r = it->second.begin(); r != it->second.end(); r++) {
				if (r->mGid <= 0 || r->mXid <= 0 || r->mHost[0] <= 0 || r->mWeight <= 0) {
					continue;
				}
				TiXmlElement* node;
				HNET_NEW(TiXmlElement("RELATION"), node);
				node->SetAttribute("GID", r->mGid);
				node->SetAttribute("XID", r->mXid);
				node->SetAttribute("HOST", r->mHost);
				node->SetAttribute("WEIGHT", r->mWeight);
				rlts->LinkEndChild(node);
			}
		}
	} else if (rlt && n > 0) {
		for (int32_t i = 0; i < n; i++) {
			if (rlt[i].mGid <= 0 || rlt[i].mXid <= 0 || rlt->mHost[0] <= 0 || rlt[i].mWeight <= 0) {
				continue;
			}
			TiXmlElement* node;
			HNET_NEW(TiXmlElement("RELATION"), node);
			node->SetAttribute("GID", rlt[i].mGid);
			node->SetAttribute("XID", rlt[i].mXid);
			node->SetAttribute("HOST", rlt[i].mHost);
			node->SetAttribute("WEIGHT", rlt[i].mWeight);
			rlts->LinkEndChild(node);
		}
	}

	if (filename.size() == 0) {
		document.SaveFile(mRltFile.c_str());
	} else {
		document.SaveFile(filename.c_str());
	}
	return 0;
}

int RouterConfig::ParseBaseConf() {
	TiXmlDocument document;
	if (!document.LoadFile(mBaseFile.c_str())) {
		HNET_ERROR(soft::GetLogPath(), "%s : %s", "RouterConfig::ParseBaseConf Load configure(conf.xml) file failed", "");
		return -1;
	}
	TiXmlElement *pRoot = document.FirstChildElement();
	if (NULL == pRoot) {
		HNET_ERROR(soft::GetLogPath(), "%s : %s", "RouterConfig::ParseBaseConf Read root from configure(conf.xml) failed", "");
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
			HNET_ERROR(soft::GetLogPath(), "%s : %s", "RouterConfig::ParseBaseConf Get SERVER host or port from conf.xml failed", "");
			return -1;
		}
	} else {
		HNET_ERROR(soft::GetLogPath(), "%s : %s", "RouterConfig::ParseBaseConf Get SERVER node from conf.xml failed", "");
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
			HNET_ERROR(soft::GetLogPath(), "%s : %s", "RouterConfig::ParseBaseConf Get CONTROL host or port from conf.xml failed", "");
			return -1;
		}
	} else {
		HNET_ERROR(soft::GetLogPath(), "%s : %s", "RouterConfig::ParseBaseConf Get CONTROL node from conf.xml failed", "");
		return -1;
	}
	return 0;
}

int RouterConfig::ParseSvrConf() {
	TiXmlDocument document;
	if (!document.LoadFile(mSvrFile.c_str())) {
		HNET_ERROR(soft::GetLogPath(), "%s : %s", "RouterConfig::ParseSvrConf Load configure(svr.xml) file failed", "");
		return -1;
	}
	TiXmlElement *pRoot = document.FirstChildElement();
	if (NULL == pRoot) {
		HNET_ERROR(soft::GetLogPath(), "%s : %s", "RouterConfig::ParseSvrConf Read root from configure(svr.xml) failed", "");
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
				if (weight != NULL) {
					int wt = atoi(weight);
					if (wt >= 0 && wt <= kMaxWeight) {
						svr.mWeight = wt;
					}
				}
				if (name != NULL) {
					memcpy(svr.mName, name, kMaxName);
				}

				// 添加新配置
				if (svr.mWeight > 0 && !mSvrQos->IsExistNode(svr)) {
					mSvrQos->SaveNode(svr);
				} else {
					HNET_ERROR(soft::GetLogdirPath() + kSvrLogFilename, "RouterConfig::ParseSvrConf Parse configure from svr.xml occur error(weight<=0 or exists this SvrNet_t), line : %d", i);
				}
			} else {
				HNET_ERROR(soft::GetLogdirPath() + kSvrLogFilename, "RouterConfig::ParseSvrConf Parse configure from svr.xml occur error, line : %d", i);
			}
		}
	} else {
		HNET_ERROR(soft::GetLogPath(), "%s : %s", "RouterConfig::ParseSvrConf Get SVRS node from svr.xml failed", "");
		return -1;
	}
	return 0;
}

int RouterConfig::ParseAgntConf() {
	TiXmlDocument document;
	if (!document.LoadFile(mAgntFile.c_str())) {
		HNET_ERROR(soft::GetLogPath(), "%s : %s", "RouterConfig::ParseAgntConf Load configure(agent.xml) file failed", "");
		return -1;
	}
	TiXmlElement *pRoot = document.FirstChildElement();
	if (NULL == pRoot) {
		HNET_ERROR(soft::GetLogPath(), "%s : %s", "RouterConfig::ParseAgntConf Read root from configure(agent.xml) failed", "");
		return -1;
	}
	
	// AGENT配置
	TiXmlElement* pElement = pRoot->FirstChildElement("AGENTS");
	if (pElement != NULL) {
		int i = 0;
		for (TiXmlElement* pChildElm = pElement->FirstChildElement(); pChildElm != NULL ; pChildElm = pChildElm->NextSiblingElement(), i++) {
			const char* host = pChildElm->Attribute("HOST");
			const char* port = pChildElm->Attribute("PORT");
			const char* weight = pChildElm->Attribute("WEIGHT");
			const char* idc = pChildElm->Attribute("IDC");
			const char* name = pChildElm->Attribute("NAME");
			if (host != NULL) {
				struct Agnt_t agnt;
				memcpy(agnt.mHost, host, kMaxHost);
				if (port != NULL) {
					agnt.mPort = atoi(port);
				}
				if (weight != NULL) {
					int wt = atoi(weight);
					if (wt >= 0) {
						agnt.mWeight = wt;
					}
				}
				if (idc != NULL) {
					agnt.mIdc = atoi(idc);
				}
				if (name != NULL) {
					memcpy(agnt.mName, name, kMaxName);
				}
				agnt.mConfig = 0;

				// 添加新配置
				if (agnt.mWeight >= 0 && !IsExistAgnt(agnt)) {
					SaveAgnt(agnt);
				} else {
					HNET_ERROR(soft::GetLogdirPath() + kSvrLogFilename, "RouterConfig::ParseAgntConf Parse configure from agent.xml occur error(exists this Agnt_t), line : %d", i);
				}
			} else {
				HNET_ERROR(soft::GetLogdirPath() + kSvrLogFilename, "RouterConfig::ParseAgntConf Parse configure from agent.xml occur error, line : %d", i);
			}
		}
	} else {
		HNET_ERROR(soft::GetLogPath(), "%s : %s", "RouterConfig::ParseAgntConf Get AGENTS node from agent.xml failed", "");
		return -1;
	}
	return 0;
}

int RouterConfig::ParseRltConf() {
	TiXmlDocument document;
	if (!document.LoadFile(mRltFile.c_str())) {
		HNET_ERROR(soft::GetLogPath(), "%s : %s", "RouterConfig::ParseRltConf Load configure(relation.xml) file failed", "");
		return -1;
	}
	TiXmlElement *pRoot = document.FirstChildElement();
	if (NULL == pRoot) {
		HNET_ERROR(soft::GetLogPath(), "%s : %s", "RouterConfig::ParseRltConf Read root from configure(relation.xml) failed", "");
		return -1;
	}
	
	// RELATION配置
	TiXmlElement* pElement = pRoot->FirstChildElement("RELATIONS");
	if (pElement != NULL) {
		int i = 0;
		for (TiXmlElement* pChildElm = pElement->FirstChildElement(); pChildElm != NULL ; pChildElm = pChildElm->NextSiblingElement(), i++) {
			const char* gid = pChildElm->Attribute("GID");
			const char* xid = pChildElm->Attribute("XID");
			const char* host = pChildElm->Attribute("HOST");
			const char* weight = pChildElm->Attribute("WEIGHT");
			if (gid != NULL && xid != NULL && host != NULL) {
				struct Rlt_t rlt(atoi(gid), atoi(xid));
				memcpy(rlt.mHost, host, kMaxHost);
				if (weight != NULL) {
					int wt = atoi(weight);
					if (wt >= 0) {
						rlt.mWeight = wt;
					}
				}

				// 添加新配置
				SaveRlt(rlt);
			} else {
				HNET_ERROR(soft::GetLogdirPath() + kSvrLogFilename, "RouterConfig::ParseRltConf Parse configure from relation.xml occur error, line : %d", i);
			}
		}
	} else {
		HNET_ERROR(soft::GetLogPath(), "%s : %s", "RouterConfig::ParseRltConf Get RELATIONS node from relation.xml failed", "");
		return -1;
	}
	return 0;
}

int RouterConfig::ParseQosConf() {
	TiXmlDocument document;
	if (!document.LoadFile(mQosFile.c_str())) {
		HNET_ERROR(soft::GetLogPath(), "%s : %s", "RouterConfig::ParseQosConf Load configure(qos.xml) file failed", "");
		return -1;
	}
	TiXmlElement *pRoot = document.FirstChildElement();
	if (NULL == pRoot) {
		HNET_ERROR(soft::GetLogPath(), "%s : %s", "RouterConfig::ParseQosConf Read root from configure(qos.xml) failed", "");
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
		HNET_ERROR(soft::GetLogPath(), "RouterConfig::ParseQosConf invalid !((REQ_EXTEND_RATE[%f] > 0.001) && (REQ_EXTEND_RATE[%f] < 101))", mSvrQos->mReqCfg.mReqExtendRate, mSvrQos->mReqCfg.mReqExtendRate);
		return -1;
	} else if (mSvrQos->mReqCfg.mReqErrMin >= 1) {
		HNET_ERROR(soft::GetLogPath(), "RouterConfig::ParseQosConf invalid REQ_ERR_MIN[%f] > 1", mSvrQos->mReqCfg.mReqErrMin);
		return -1;
	} else if (mSvrQos->mDownCfg.mPossbileDownErrRate > 1 || mSvrQos->mDownCfg.mPossbileDownErrRate < 0.01) {
		HNET_ERROR(soft::GetLogPath(), "RouterConfig::ParseQosConf invalid DOWN_ERR_RATE[%f] > 1 || DOWN_ERR_RATE[%f] < 0.01", mSvrQos->mDownCfg.mPossbileDownErrRate, mSvrQos->mDownCfg.mPossbileDownErrRate);
		return -1;
	} else if (mSvrQos->mDownCfg.mProbeTimes < 3) {
		HNET_ERROR(soft::GetLogPath(), "RouterConfig::ParseQosConf invalid TIMES[%d] < 3", mSvrQos->mDownCfg.mProbeTimes);
		return -1;
	} else if (mSvrQos->mReqCfg.mRebuildTm < 3) {
		HNET_ERROR(soft::GetLogPath(), "RouterConfig::ParseQosConf invalid REBUILD_TM[%d] < 3", mSvrQos->mReqCfg.mRebuildTm);
		return -1;
	}

    if (mSvrQos->mReqCfg.mPreTime <= 0 || mSvrQos->mReqCfg.mPreTime > (mSvrQos->mRebuildTm / 2)) {
		mSvrQos->mReqCfg.mPreTime = 2;
	}
	return 0;
}

bool RouterConfig::IsExistAgnt(const struct Agnt_t& agnt, struct Agnt_t* old) {
	std::vector<struct Agnt_t>::iterator it = std::find(mAgnts.begin(), mAgnts.end(), agnt);
	if (it == mAgnts.end()) {
		return false;
	}
	if (old) {
		*old = *it;
	}
	return true;
}

bool RouterConfig::IsAgntChange(const struct Agnt_t& agnt) {
	std::vector<struct Agnt_t>::iterator it = std::find(mAgnts.begin(), mAgnts.end(), agnt);
	if (it != mAgnts.end()) {
		const struct Agnt_t& old = *it;
		if (old.mStatus == agnt.mStatus && old.mIdc == agnt.mIdc && old.mWeight == agnt.mWeight) {
			return false;
		}
	}
	return true;
}

int RouterConfig::SaveAgnt(const struct Agnt_t& agnt) {
	if (IsExistAgnt(agnt)) {
		return ModifyAgnt(agnt);
	}
	return AddAgnt(agnt);
}

int RouterConfig::ModifyAgnt(const struct Agnt_t& agnt) {
	HNET_DEBUG(soft::GetLogdirPath() + kAgntLogFilename, "RouterConfig::ModifyAgnt modify Agnt_t, HOST(%s),PORT(%d), WEIGHT(%d)", agnt.mHost, agnt.mPort, agnt.mWeight);

	if (agnt.mWeight < 0) {
		HNET_ERROR(soft::GetLogdirPath() + kAgntLogFilename, "RouterConfig::ModifyAgnt modify Agnt_t failed, HOST(%s),PORT(%d), WEIGHT(%d)", agnt.mHost, agnt.mPort, agnt.mWeight);
		return -1;
	} else if (agnt.mWeight == 0 && agnt.mStatus == kAgntInit) {
		return DelAgnt(agnt);
	} else {
		std::vector<struct Agnt_t>::iterator it = std::find(mAgnts.begin(), mAgnts.end(), agnt);
		struct Agnt_t& oldagnt = const_cast<struct Agnt_t&>(*it);
		oldagnt = agnt;
	}
	return 0;
}

int RouterConfig::DelAgnt(const struct Agnt_t& agnt) {
	std::vector<struct Agnt_t>::iterator it = std::find(mAgnts.begin(), mAgnts.end(), agnt);
	if (it == mAgnts.end()) {
		HNET_ERROR(soft::GetLogdirPath() + kAgntLogFilename, "RouterConfig::DelAgnt delete Agnt_t failed(cannot find the Agnt_t), HOST(%s),PORT(%d)", agnt.mHost, agnt.mPort);
		return -1;
	}
	mAgnts.erase(it);
	return 0;
}

int RouterConfig::AddAgnt(const struct Agnt_t& agnt) {
	HNET_DEBUG(soft::GetLogdirPath() + kAgntLogFilename, "RouterConfig::AddAgnt add Agnt_t success, HOST(%s),PORT(%d)", agnt.mHost, agnt.mPort);
	
	if (agnt.mWeight > 0) {
		mAgnts.push_back(agnt);
	}
	return 0;
}

void RouterConfig::CleanAgnt() {
	HNET_DEBUG(soft::GetLogdirPath() + kAgntLogFilename, "RouterConfig::CleanAgnt clean Agnt_t success");
	mAgnts.clear();
}

int RouterConfig::GetAgntAll(struct Agnt_t buf[], int32_t start, int32_t size) {
	HNET_DEBUG(soft::GetLogdirPath() + kAgntLogFilename, "RouterConfig::GetAgntAll get all Agnt_t start, [%d, %d]", start, size);

	int num = 0;
	if (static_cast<size_t>(start) >= mAgnts.size()) {
		return num;
	}
	std::vector<struct Agnt_t>::iterator it = mAgnts.begin();
	for (std::advance(it, start); it != mAgnts.end() && num < size; it++) {
		buf[num++] = *it;
	}
	return num;
}

bool RouterConfig::IsExistRlt(const std::string& host, const struct Rlt_t& rlt) {
	MapRltIt_t mapIt = mRelations.find(host);
	if (mapIt != mRelations.end()) {
		if (rlt.mGid > 0 && rlt.mXid > 0) {
			if (std::find(mapIt->second.begin(), mapIt->second.end(), rlt) != mapIt->second.end()) {
				return true;
			} else {
				return false;
			}
		}
		return true;
	}
	return false;
}

const RouterConfig::MapRlt_t& RouterConfig::GetRltsAll() {
	return mRelations;
}

const RouterConfig::vRlt_t& RouterConfig::Rlts(const std::string& host) {
	return mRelations[host];
}

int RouterConfig::SaveRlt(const struct Rlt_t& rlt) {
	if (IsExistRlt(rlt.mHost)) {
		return ModifyRlt(rlt);
	}
	return AddRlt(rlt);
}

int RouterConfig::ModifyRlt(const struct Rlt_t& rlt) {
	MapRltIt_t mapIt = mRelations.find(rlt.mHost);
	if (mapIt == mRelations.end()) {
		return -1;
	}

	vRlt_t rlts = mapIt->second;
	vRltIt_t vit = std::find(rlts.begin(), rlts.end(), rlt);
	if (vit != rlts.end()) {
		rlts.erase(vit);

		if (rlt.mWeight == 0) {	// 删除节点
			if (rlts.empty()) {
				mRelations.erase(mapIt);
			}

			HNET_DEBUG(soft::GetLogdirPath() + kRltLogFilename, "RouterConfig::ModifyRlt delete Rlt_t success HOST(%s),GID(%d),XID(%d)", rlt.mHost,rlt.mGid, rlt.mXid);
			return 0;
		}
	}

	rlts.push_back(rlt);
	mRelations.erase(mapIt);
	mRelations.insert(std::make_pair(rlt.mHost, rlts));

	HNET_DEBUG(soft::GetLogdirPath() + kRltLogFilename, "RouterConfig::ModifyRlt modify Rlt_t success HOST(%s),GID(%d),XID(%d)", rlt.mHost,rlt.mGid, rlt.mXid);
	return 0;
}

int RouterConfig::AddRlt(const struct Rlt_t& rlt) {
	if (rlt.mWeight > 0) {
		vRlt_t rlts;
		rlts.push_back(rlt);
		mRelations.insert(std::make_pair(rlt.mHost, rlts));

		HNET_DEBUG(soft::GetLogdirPath() + kRltLogFilename, "RouterConfig::AddRlt add Rlt_t success, HOST(%s),GID(%d),XID(%d)", rlt.mHost,rlt.mGid, rlt.mXid);
	}
	return 0;
}

void RouterConfig::CleanRlt() {
	HNET_DEBUG(soft::GetLogdirPath() + kRltLogFilename, "RouterConfig::CleanRlt clean Rlt_t success");
	mRelations.clear();
}
