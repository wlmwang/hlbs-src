
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "AgentConfig.h"
#include "wMisc.h"
#include "Svr.h"

const char kConfXml[]	= "../config/conf.xml";
const char kRouterXml[]	= "../config/router.xml";
const char kQosXml[]	= "../config/qos.xml";

AgentConfig::AgentConfig() : mRouterFile(kRouterXml), mQosFile(kQosXml), mBaseFile(kConfXml) {
	SAFE_NEW(SvrQos, mSvrQos);
}

AgentConfig::~AgentConfig() {
	SAFE_DELETE(mSvrQos);
}

const wStatus& AgentConfig::ParseBaseConf() {
	if (!mDoc.LoadFile(mBaseFile.c_str())) {
		return mStatus = wStatus::InvalidArgument("AgentConfig::ParseBaseConf Load configure(conf.xml) file failed", "");
	}
	TiXmlElement *pRoot = mDoc.FirstChildElement();
	if (NULL == pRoot) {
		return mStatus = wStatus::InvalidArgument("AgentConfig::ParseBaseConf Read root from configure(conf.xml) failed", "");
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
			return mStatus = wStatus::InvalidArgument("AgentConfig::ParseBaseConf Get SERVER host or port from conf.xml failed", "");
		}
	} else {
		return mStatus = wStatus::InvalidArgument("AgentConfig::ParseBaseConf Get SERVER node from conf.xml failed", "");
	}

	return mStatus.Clear();
}

const wStatus& AgentConfig::ParseRouterConf() {
	if (!mDoc.LoadFile(mRouterFile.c_str())) {
		return mStatus = wStatus::InvalidArgument("AgentConfig::ParseRouterConf Load configure(router.xml) file failed", "");
	}
	TiXmlElement *pRoot = mDoc.FirstChildElement();
	if (NULL == pRoot) {
		return mStatus = wStatus::InvalidArgument("AgentConfig::ParseRouterConf Read root from configure(router.xml) failed", "");
	}

	// ROUTERS配置
	TiXmlElement* pElement = pRoot->FirstChildElement("ROUTERS");
	if (pElement != NULL) {
		int i = 0;
		for (TiXmlElement* pChildElm = pElement->FirstChildElement(); pChildElm != NULL ; pChildElm = pChildElm->NextSiblingElement(), i++) {
			const char* host = pChildElm->Attribute("HOST");
			const char* port = pChildElm->Attribute("PORT");
			if (host != NULL && port != NULL) {
				SetStrConf("router_host", host);
				SetIntConf("router_port", atoi(port));
			} else {
				wStatus::InvalidArgument("AgentConfig::ParseRouterConf Parse configure from router.xml occur error", logging::NumberToString(i));
			}
		}
	} else {
		return mStatus = wStatus::InvalidArgument("AgentConfig::ParseRouterConf Get ROUTERS node from router.xml failed", "");
	}
	return mStatus.Clear();
}

const wStatus& AgentConfig::ParseQosConf() {
	if (!mDoc.LoadFile(mQosFile.c_str())) {
		return mStatus = wStatus::InvalidArgument("AgentConfig::ParseQosConf Load configure(qos.xml) file failed", "");
	}
	TiXmlElement *pRoot = mDoc.FirstChildElement();
	if (NULL == pRoot) {
		return mStatus = wStatus::InvalidArgument("AgentConfig::ParseQosConf Read root from configure(qos.xml) failed", "");
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
		return mStatus = wStatus::InvalidArgument("AgentConfig::ParseQosConf invalid !((REQ_EXTEND_RATE[%f] > 0.001) && (REQ_EXTEND_RATE[%f] < 101))", "");
	} else if (mSvrQos->mReqCfg.mReqErrMin >= 1) {
		return mStatus = wStatus::InvalidArgument("AgentConfig::ParseQosConf invalid REQ_ERR_MIN[%f] > 1", "");
	} else if (mSvrQos->mDownCfg.mPossbileDownErrRate > 1 || mSvrQos->mDownCfg.mPossbileDownErrRate < 0.01) {
		return mStatus = wStatus::InvalidArgument("AgentConfig::ParseQosConf invalid DOWN_ERR_RATE[%f] > 1 || DOWN_ERR_RATE[%f] < 0.01", "");
	} else if (mSvrQos->mDownCfg.mProbeTimes < 3) {
		return mStatus = wStatus::InvalidArgument("AgentConfig::ParseQosConf invalid TIMES[%d] < 3", "");
	} else if (mSvrQos->mReqCfg.mRebuildTm < 3) {
		return mStatus = wStatus::InvalidArgument("AgentConfig::ParseQosConf invalid REBUILD_TM[%d] < 3", "");
	}

    if (mSvrQos->mReqCfg.mPreTime <= 0 || mSvrQos->mReqCfg.mPreTime > (mSvrQos->mRebuildTm / 2)) {
		mSvrQos->mReqCfg.mPreTime = 2;
	}
	return mStatus;
}
