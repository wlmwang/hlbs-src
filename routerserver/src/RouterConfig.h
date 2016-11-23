
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _ROUTER_CONFIG_H_
#define _ROUTER_CONFIG_H_

#include "wCore.h"
#include "wStatus.h"
#include "wMisc.h"
#include "wConfig.h"

using namespace hnet;

class SvrQos;
class TiXmlDocument;

class RouterConfig : public wConfig {
public:
	RouterConfig();
	virtual ~RouterConfig();

	const wStatus& ParseBaseConf();
	const wStatus& ParseSvrConf();
	const wStatus& ParseQosConf();
	const wStatus& ParseModSvr(struct SvrNet_t buf[], int32_t* num);	// 获取修改节点。不能删除节点（可修改WEIGHT=0属性，达到删除节点效果）

	bool IsModTime();
	int SetModTime();
	SvrQos* Qos() { return mSvrQos;}

protected:
	SvrQos *mSvrQos;
	TiXmlDocument *mDoc;
	std::string mSvrFile;
	std::string mQosFile;
	std::string mBaseFile;

	// svr.xml修改时间
	time_t mMtime;
};

#endif
