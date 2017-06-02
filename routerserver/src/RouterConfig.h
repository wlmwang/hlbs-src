
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _ROUTER_CONFIG_H_
#define _ROUTER_CONFIG_H_

#include <tinyxml.h>
#include "wCore.h"
#include "wStatus.h"
#include "wLogger.h"
#include "wConfig.h"
#include "SvrQos.h"

using namespace hnet;

class RouterConfig : public wConfig {
public:
	RouterConfig();
	virtual ~RouterConfig();

	const wStatus& ParseBaseConf();
	const wStatus& ParseSvrConf();
	const wStatus& ParseQosConf();

	// 更新变化节点
	// 不能删除节点，可修改WEIGHT=0属性，达到删除节点效果
	const wStatus& ParseModifySvr();
	const wStatus& WriteFileSvr(const struct SvrNet_t* svr = NULL, int32_t n = 0, std::string filename = "");

	// svr.xml文件最后一次被修改的时间
	const wStatus& SetMtime();
	bool IsModifySvr();

	SvrQos* Qos() { return mSvrQos;}
	
protected:
	time_t mMtime;	// svr.xml文件最后一次被修改的时间
	SvrQos *mSvrQos;
	std::string mSvrFile;
	std::string mQosFile;
	std::string mBaseFile;
};

#endif
