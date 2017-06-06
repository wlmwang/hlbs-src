
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _ROUTER_CONFIG_H_
#define _ROUTER_CONFIG_H_

#include <vector>
#include <algorithm>
#include <tinyxml.h>
#include "wCore.h"
#include "wStatus.h"
#include "wMisc.h"
#include "wLogger.h"
#include "wConfig.h"
#include "Svr.h"
#include "Agnt.h"
#include "SvrQos.h"

using namespace hnet;

class RouterConfig : public wConfig {
public:
	RouterConfig();
	virtual ~RouterConfig();

	int ParseBaseConf();
	int ParseSvrConf();
	int ParseQosConf();
	int ParseAgntConf();

	// 更新变化节点
	// 不能删除节点，可修改WEIGHT=0属性，达到删除节点效果
	int ParseModifySvr();
	int WriteFileSvr(const struct SvrNet_t* svr = NULL, int32_t n = 0, const std::string& filename = "");

	// svr.xml文件最后一次被修改的时间
	int SetSvrMtime();
	bool IsModifySvr();
	SvrQos* Qos() { return mSvrQos;}

	int SetAgntMtime();
	bool IsModifyAgnt();
	bool CleanAgnt();
	bool IsExistAgnt(const struct Agnt_t& agnt);
	bool IsAgntChange(const struct Agnt_t& agnt);
	bool SaveAgnt(const struct Agnt_t& agnt);
	bool AddAgnt(const struct Agnt_t& agnt);
	bool DelAgnt(const struct Agnt_t& agnt);
	bool ModifyAgnt(const struct Agnt_t& agnt);
	int GetAgntAll(struct Agnt_t buf[], int32_t start, int32_t size);
	int WriteFileAgnt(const struct Agnt_t* agnt = NULL, int32_t n = 0, const std::string& filename = "");

protected:
	time_t mSvrMtime;	// svr.xml文件最后一次被修改的时间
	time_t mAgntMtime;	// agent.xml文件最后一次被修改的时间

	std::vector<struct Agnt_t> mAgnts;
	SvrQos   *mSvrQos;
	std::string mSvrFile;
	std::string mQosFile;
	std::string mBaseFile;
	std::string mAgntFile;
};

#endif
