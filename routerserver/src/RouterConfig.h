
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _ROUTER_CONFIG_H_
#define _ROUTER_CONFIG_H_

#include "wCore.h"
#include "wStatus.h"
#include "wLogger.h"
#include "wConfig.h"
#include "SvrQos.h"
#include "tinyxml.h"

using namespace hnet;

class RouterConfig : public wConfig {
public:
	RouterConfig();
	virtual ~RouterConfig();

	const wStatus& ParseBaseConf();
	const wStatus& ParseSvrConf();
	const wStatus& ParseQosConf();

	// 获取修改节点。不能删除节点（可修改WEIGHT=0属性，达到删除节点效果）
	const wStatus& ParseModifySvr(struct SvrNet_t buf[], int32_t* num, int32_t start, int32_t size);

	SvrQos* Qos() { return mSvrQos;}

	// svr.xml文件最后一次被修改的时间
	const wStatus& SetMtime();
	bool IsModifySvr();

protected:
	SvrQos *mSvrQos;
	TiXmlDocument mDoc;
	std::string mSvrFile;
	std::string mQosFile;
	std::string mBaseFile;

	// svr.xml文件最后一次被修改的时间
	time_t mMtime;
};

#endif
