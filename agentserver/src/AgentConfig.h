
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _AGENT_CONFIG_H_
#define _AGENT_CONFIG_H_

#include <tinyxml.h>
#include "wCore.h"
#include "wMisc.h"
#include "wConfig.h"
#include "SvrQos.h"

using namespace hnet;

class AgentConfig : public wConfig {
public:
	AgentConfig();
	virtual ~AgentConfig();

	int ParseBaseConf();
	int ParseRouterConf();
	int ParseQosConf();

	SvrQos* Qos() { return mSvrQos;}
protected:
	SvrQos* mSvrQos;
	std::string mRouterFile;
	std::string mQosFile;
	std::string mBaseFile;
};

#endif
