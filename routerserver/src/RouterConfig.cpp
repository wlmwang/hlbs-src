
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "RouterConfig.h"
#include "wMisc.h"
#include "Svr.h"

const char kConfXml[]	= "../config/conf.xml";
const char kSvrXml[]	= "../config/svr.xml";
const char kQosXml[]	= "../config/qos.xml";

RouterConfig::RouterConfig() : mSvrFile(kSvrXml), mQosFile(kQosXml), mBaseFile(kConfXml), mMtime(0) {
	SAFE_NEW(SvrQos, mSvrQos);
}

RouterConfig::~RouterConfig() {
	SAFE_DELETE(mSvrQos);
}

const wStatus& RouterConfig::WriteSvrConfFile() {
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

	// 创建SVR节点
	int32_t start = 0, num = 0;
	struct SvrNet_t svr[kMaxNum];
	do {
		if (mSvrQos->GetNodeAll(svr, &num, start, kMaxNum).Ok() && num > 0) {
			for (int i = 0; i < num; i++) {		// 保存SVR节点
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
		start += num;
	} while (num >= kMaxNum);
	document.SaveFile(mSvrFile.c_str());
	return mStatus;
}

const wStatus& RouterConfig::ParseBaseConf() {
	TiXmlDocument document;
	if (!document.LoadFile(mBaseFile.c_str())) {
		return mStatus = wStatus::InvalidArgument("RouterConfig::ParseBaseConf Load configure(conf.xml) file failed", "");
	}
	TiXmlElement *pRoot = document.FirstChildElement();
	if (NULL == pRoot) {
		return mStatus = wStatus::InvalidArgument("RouterConfig::ParseBaseConf Read root from configure(conf.xml) failed", "");
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
			return mStatus = wStatus::InvalidArgument("RouterConfig::ParseBaseConf Get SERVER host or port from conf.xml failed", "");
		}
	} else {
		return mStatus = wStatus::InvalidArgument("RouterConfig::ParseBaseConf Get SERVER node from conf.xml failed", "");
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
			return mStatus = wStatus::InvalidArgument("RouterConfig::ParseBaseConf Get CONTROL host or port from conf.xml failed", "");
		}
	} else {
		return mStatus = wStatus::InvalidArgument("RouterConfig::ParseBaseConf Get CONTROL node from conf.xml failed", "");
	}
	return mStatus;
}

const wStatus& RouterConfig::ParseSvrConf() {
	TiXmlDocument document;
	if (!document.LoadFile(mSvrFile.c_str())) {
		return mStatus = wStatus::InvalidArgument("RouterConfig::ParseSvrConf Load configure(svr.xml) file failed", "");
	}
	TiXmlElement *pRoot = document.FirstChildElement();
	if (NULL == pRoot) {
		return mStatus = wStatus::InvalidArgument("RouterConfig::ParseSvrConf Read root from configure(svr.xml) failed", "");
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
		return mStatus = wStatus::InvalidArgument("RouterConfig::ParseSvrConf Get SVRS node from svr.xml failed", "");
	}

	return SetMtime();	// 记录svr.xml文件更新时间
}

const wStatus& RouterConfig::ParseModifySvr(struct SvrNet_t buf[], int32_t* num, int32_t start, int32_t size) {
	TiXmlDocument document;
	if (!document.LoadFile(mSvrFile.c_str())) {
		return mStatus = wStatus::InvalidArgument("RouterConfig::ParseModifySvr Load configure(svr.xml) file failed", "");
	}
	TiXmlElement *pRoot = document.FirstChildElement();
	if (NULL == pRoot) {
		return mStatus = wStatus::InvalidArgument("RouterConfig::ParseModifySvr Read root from configure(svr.xml) failed", "");
	}

	TiXmlElement* pElement = pRoot->FirstChildElement("SVRS");
	if (pElement != NULL) {
		int i = 0;
		*num = 0;
		for (TiXmlElement* pChildElm = pElement->FirstChildElement(); pChildElm != NULL ; pChildElm = pChildElm->NextSiblingElement(), i++) {
			if (i < start) {
				continue;
			}
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

				if (svr.mWeight >= 0 && mSvrQos->IsExistNode(svr)) {
					// 旧配置检测到version变化才下发
					if (mSvrQos->IsVerChange(svr)) {
						mSvrQos->SaveNode(svr);
						buf[(*num)++] = svr;
					}
				} else if (svr.mWeight >= 0) {
					// 添加新配置
					mSvrQos->SaveNode(svr);
					buf[(*num)++] = svr;
				}

				if (*num >= size) {
					break;
				}
			} else {
				LOG_ERROR(kSvrLog, "RouterConfig::ParseModifySvr Parse configure from svr.xml occur error, line : %d", i);
			}
		}
		return SetMtime();	// 记录svr.xml文件更新时间
	} else {
		return mStatus = wStatus::InvalidArgument("RouterConfig::ParseModifySvr Get SVRS node from svr.xml failed", "");
	}
}

const wStatus& RouterConfig::ParseQosConf() {
	TiXmlDocument document;
	if (!document.LoadFile(mQosFile.c_str())) {
		return mStatus = wStatus::InvalidArgument("RouterConfig::ParseQosConf Load configure(qos.xml) file failed", "");
	}
	TiXmlElement *pRoot = document.FirstChildElement();
	if (NULL == pRoot) {
		return mStatus = wStatus::InvalidArgument("RouterConfig::ParseQosConf Read root from configure(qos.xml) failed", "");
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
		return mStatus = wStatus::InvalidArgument("RouterConfig::ParseQosConf invalid !((REQ_EXTEND_RATE[%f] > 0.001) && (REQ_EXTEND_RATE[%f] < 101))", "");
	} else if (mSvrQos->mReqCfg.mReqErrMin >= 1) {
		return mStatus = wStatus::InvalidArgument("RouterConfig::ParseQosConf invalid REQ_ERR_MIN[%f] > 1", "");
	} else if (mSvrQos->mDownCfg.mPossbileDownErrRate > 1 || mSvrQos->mDownCfg.mPossbileDownErrRate < 0.01) {
		return mStatus = wStatus::InvalidArgument("RouterConfig::ParseQosConf invalid DOWN_ERR_RATE[%f] > 1 || DOWN_ERR_RATE[%f] < 0.01", "");
	} else if (mSvrQos->mDownCfg.mProbeTimes < 3) {
		return mStatus = wStatus::InvalidArgument("RouterConfig::ParseQosConf invalid TIMES[%d] < 3", "");
	} else if (mSvrQos->mReqCfg.mRebuildTm < 3) {
		return mStatus = wStatus::InvalidArgument("RouterConfig::ParseQosConf invalid REBUILD_TM[%d] < 3", "");
	}

    if (mSvrQos->mReqCfg.mPreTime <= 0 || mSvrQos->mReqCfg.mPreTime > (mSvrQos->mRebuildTm / 2)) {
		mSvrQos->mReqCfg.mPreTime = 2;
	}
	return mStatus;
}

const wStatus& RouterConfig::SetMtime() {
	struct stat stBuf;
	if (stat(mSvrFile.c_str(), &stBuf) == 0) {
		mMtime = stBuf.st_mtime;
		return mStatus;
	}
	return mStatus = wStatus::IOError("RouterConfig::SetMtime svr.xml failed", strerror(errno));
}

bool RouterConfig::IsModifySvr() {
	struct stat stBuf;
	if (stat(mSvrFile.c_str(), &stBuf) == 0 && stBuf.st_mtime > mMtime) {
		return true;
	}
	return false;
}
