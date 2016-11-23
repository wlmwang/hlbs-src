
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "RouterConfig.h"
#include "tinyxml.h"
#include "wMisc.h"
#include "Svr.h"
#include "SvrQos.h"

const kConfXml[]	= "../config/conf.xml";
const kSvrXml[]		= "../config/svr.xml";
const kQosXml[]		= "../config/qos.xml";

RouterConfig::RouterConfig() : mSvrFile(kSvrXml), mQosFile(kQosXml), mBaseFile(kConfXml), mMtime(0) {
	SAFE_NEW(SvrQos, mSvrQos);
	SAFE_NEW(TiXmlDocument, mDoc);
}

RouterConfig::~RouterConfig() {
	SAFE_DELETE(mSvrQos);
	SAFE_DELETE(mDoc);
}

const wStatus& RouterConfig::ParseBaseConf() {
	if (!mDoc->LoadFile(mBaseFile.c_str())) {
		return mStatus = wStatus::InvalidArgument("RouterConfig::GetBaseConf Load configure(config.xml) file failed", "");
	}
	TiXmlElement *pRoot = mDoc->FirstChildElement();
	if (NULL == pRoot) {
		return mStatus = wStatus::InvalidArgument("RouterConfig::GetBaseConf Read root from configure file(conf.xml) failed", "");
	}
	
	// 服务器侦听地址
	const TiXmlElement* pElement = pRoot->FirstChildElement("SERVER");
	if (NULL != pElement) {
		const char* host = pElement->Attribute("HOST");
		const char* port = pElement->Attribute("PORT");
		const char* worker = pElement->Attribute("WORKER");
		if (host != NULL && port != NULL) {
			SetStrConf("host", host);
			SetIntConf("port", atoi(port));
			SetIntConf("worker", atoi(worker));
		} else {
			return mStatus = wStatus::InvalidArgument("RouterConfig::GetBaseConf Get SERVER host or port from conf.xml failed", "");
		}
	} else {
		return mStatus = wStatus::InvalidArgument("RouterConfig::GetBaseConf Get SERVER node from conf.xml failed", "");
	}

	return mStatus.Clear();
}

const wStatus& RouterConfig::ParseSvrConf() {
	if (!mDoc->LoadFile(mSvrFile.c_str())) {
		return mStatus = wStatus::InvalidArgument("RouterConfig::GetSvrConf Load configure(svr.xml) file failed", "");
	}
	TiXmlElement *pRoot = mDoc->FirstChildElement();
	if (NULL == pRoot) {
		return mStatus = wStatus::InvalidArgument("RouterConfig::GetSvrConf Read root from configure file(svr.xml) failed", "");
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
			if (gid != NULL && xid != NULL && host != NULL && port != NULL) {

				struct SvrNet_t stSvr;
				stSvr.mGid = atoi(gid);
				stSvr.mXid = atoi(xid);
				stSvr.mPort = atoi(port);
				memcpy(stSvr.mHost, host, kMaxHost);
				if (weight != NULL) {
					stSvr.mWeight = atoi(weight);
				}
				if (version != NULL) {
					stSvr.mVersion = atoi(version);
				}
				if (stSvr.mWeight == 0) {
					continue;
				}

				// 添加配置
				mSvrQos->SaveNode(stSvr);
			} else {
				wStatus::InvalidArgument("RouterConfig::GetSvrConf Parse configure from svr.xml occur error", logging::NumberToString(i));
			}
		}
	} else {
		return wStatus::InvalidArgument("RouterConfig::GetSvrConf Get SVRS node from svr.xml failed", "");
	}

	// 记录svr.xml文件更新时间
	SetModTime();

	return mStatus.Clear();
}

const wStatus& RouterConfig::GetModSvr(struct SvrNet_t buf[], int32_t* num) {
	if (!mDoc->LoadFile(mSvrFile.c_str())) {
		return mStatus = wStatus::InvalidArgument("RouterConfig::GetModSvr Load configure(svr.xml) file failed", "");
	}
	TiXmlElement *pRoot = mDoc->FirstChildElement();
	if (NULL == pRoot) {
		return mStatus = wStatus::InvalidArgument("RouterConfig::GetModSvr Read root from configure file(svr.xml) failed", "");
	}

	TiXmlElement* pElement = pRoot->FirstChildElement("SVRS");
	if (pElement != NULL) {
		int i = 0;
		*num = 0;
		for (TiXmlElement* pChildElm = pElement->FirstChildElement(); pChildElm != NULL ; pChildElm = pChildElm->NextSiblingElement(), i++) {
			const char* gid = pChildElm->Attribute("GID");
			const char* xid = pChildElm->Attribute("XID");
			const char* host = pChildElm->Attribute("HOST");
			const char* port = pChildElm->Attribute("PORT");
			const char* weight = pChildElm->Attribute("WEIGHT");
			const char* version = pChildElm->Attribute("VERSION");

			if (gid != NULL && xid != NULL && host != NULL && port != NULL) {

				struct SvrNet_t stSvr;
				stSvr.mGid = atoi(gid);
				stSvr.mXid = atoi(xid);
				stSvr.mPort = atoi(port);
				memcpy(stSvr.mHost, host, kMaxHost);
				if (weight != NULL) {
					stSvr.mWeight = atoi(weight);
				}
				if (version != NULL) {
					stSvr.mVersion = atoi(version);
				}
				
				// 新配置始终下发，旧配置检测到version变化才下发
				if (mSvrQos->IsExistNode(stSvr)) {
					if (mSvrQos->IsVerChange(stSvr)) {
						// 版本变化，确定下发配置
						mSvrQos->SaveNode(stSvr);
						buf[(*num)++] = stSvr;
					}
				} else {
					// 添加新配置
					mSvrQos->SaveNode(stSvr);
					buf[(*num)++] = stSvr;
				}
			} else {
				wStatus::InvalidArgument("RouterConfig::GetModSvr Parse configure from svr.xml occur error", logging::NumberToString(i));
			}
		}

		// 记录svr.xml文件更新时间
		SetModTime();
	} else {
		return wStatus::InvalidArgument("RouterConfig::GetSvrConf Get SVRS node from svr.xml failed", "");
	}
	return mStatus.Clear();
}

const wStatus& RouterConfig::ParseQosConf() {
	if (!mDoc->LoadFile(mQosFile.c_str())) {
		return mStatus = wStatus::InvalidArgument("RouterConfig::GetQosConf Load configure(qos.xml) file failed", "");
	}
	TiXmlElement *pRoot = mDoc->FirstChildElement();
	if (NULL == pRoot) {
		return mStatus = wStatus::InvalidArgument("RouterConfig::GetQosConf Read root from configure file(qos.xml) failed", "");
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
			mSvrQos->mReqCfg.mReqMax = 10000;
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
		if (pElement->Attribute("BEGIN") != NULL) {
			mSvrQos->mDownCfg.mProbeBegin = atoi(pElement->Attribute("BEGIN"));
		} else {
			mSvrQos->mDownCfg.mProbeBegin = 3;
		}

		if (pElement->Attribute("INTERVAL") != NULL) {
			mSvrQos->mDownCfg.mProbeInterval = atoi(pElement->Attribute("INTERVAL"));
		} else {
			mSvrQos->mDownCfg.mProbeInterval = 10;
		}

		if (pElement->Attribute("EXPIRED") != NULL) {
			mSvrQos->mDownCfg.mProbeNodeExpireTime = atoi(pElement->Attribute("EXPIRED"));
		} else {
			mSvrQos->mDownCfg.mProbeNodeExpireTime = 600;
		}

		if (pElement->Attribute("TIMES") != NULL) {
			mSvrQos->mDownCfg.mProbeTimes = atoi(pElement->Attribute("TIMES"));
		} else {
			mSvrQos->mDownCfg.mProbeTimes = 3;
		}

		if (pElement->Attribute("REQ_COUNT") != NULL) {
			mSvrQos->mDownCfg.mReqCountTrigerProbe = atoi(pElement->Attribute("REQ_COUNT"));
		} else {
			mSvrQos->mDownCfg.mReqCountTrigerProbe = 100000;
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

    if (mSvrQos->mReqCfg.mPreTime <= 0 || mSvrQos->mReqCfg.mPreTime > (mSvrQos->mRebuildTm / 2)) {
		mSvrQos->mReqCfg.mPreTime = 2;
	}

	if (!(mSvrQos->mReqCfg.mReqExtendRate > 0.001 && mSvrQos->mReqCfg.mReqExtendRate < 101)) {
		return mStatus = wStatus::InvalidArgument("RouterConfig::GetQosConf invalid !((REQ_EXTEND_RATE[%f] > 0.001) && (REQ_EXTEND_RATE[%f] < 101))", "");
	} else if (mSvrQos->mReqCfg.mReqErrMin >= 1) {
		return mStatus = wStatus::InvalidArgument("RouterConfig::GetQosConf invalid REQ_ERR_MIN[%f] > 1", "");
	} else if (mSvrQos->mDownCfg.mPossbileDownErrRate > 1 || mSvrQos->mDownCfg.mPossbileDownErrRate < 0.01) {
		return mStatus = wStatus::InvalidArgument("RouterConfig::GetQosConf invalid DOWN_ERR_RATE[%f] > 1 || DOWN_ERR_RATE[%f] < 0.01", "");
	} else if (mSvrQos->mDownCfg.mProbeTimes < 3) {
		return mStatus = wStatus::InvalidArgument("RouterConfig::GetQosConf invalid TIMES[%d] < 3", "");
	} else if (mSvrQos->mReqCfg.mRebuildTm < 3) {
		return mStatus = wStatus::InvalidArgument("RouterConfig::GetQosConf invalid REBUILD[%d] < 3", "");
	}
	return mStatus.Clear();
}

int RouterConfig::SetModTime() {
	struct stat stBuf;
	int iRet = stat(mSvrFile.c_str(), &stBuf);
	if (iRet == 0) {
		return mMtime = stBuf.st_mtime;
	}
	return iRet; // -1
}

bool RouterConfig::IsModTime() {
	struct stat stBuf;
	int iRet = stat(mSvrFile.c_str(), &stBuf);
	if (iRet == 0 && stBuf.st_mtime > mMtime) {
		return true;
	}
	return false;
}
