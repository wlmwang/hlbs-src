
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _AGENT_CONFIG_H_
#define _AGENT_CONFIG_H_

#include "wCore.h"
#include "wStatus.h"
#include "wMisc.h"
#include "wConfig.h"
#include "tinyxml.h"
#include "SvrQos.h"
#include "DetectThread.h"

using namespace hnet;

class AgentConfig : public wConfig {
public:
	AgentConfig();
	virtual ~AgentConfig();

	const wStatus& ParseBaseConf();
	const wStatus& ParseRouterConf();
	const wStatus& ParseQosConf();

	const wStatus& StartDetectThread();

	SvrQos* Qos() { return mSvrQos;}
	mDetectThread* Detect() { return mDetectThread;}

protected:
	SvrQos* mSvrQos;
	DetectThread* mDetectThread;
	TiXmlDocument mDoc;
	std::string mRouterFile;
	std::string mQosFile;
	std::string mBaseFile;
};

#endif
