
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
#include "wMisc.h"
#include "wLogger.h"
#include "wConfig.h"
#include "Svr.h"
#include "Agnt.h"
#include "SvrQos.h"

using namespace hnet;

class RouterConfig : public wConfig {
public:
	typedef std::map<std::string, std::vector<struct Rlt_t> > MapRlt_t;
    typedef std::map<std::string, std::vector<struct Rlt_t> >::iterator MapRltIt_t;
    typedef std::map<std::string, std::vector<struct Rlt_t> >::const_iterator MapRltCIt_t;
	typedef std::vector<struct Rlt_t> vRlt_t;
    typedef std::vector<struct Rlt_t>::iterator vRltIt_t;
    typedef std::vector<struct Rlt_t>::const_iterator vRltCIt_t;

public:
	RouterConfig();
	virtual ~RouterConfig();

	int ParseBaseConf();
	int ParseSvrConf();
	int ParseQosConf();
	int ParseAgntConf();
	int ParseRltConf();

	SvrQos* Qos() { return mSvrQos;}
	int WriteFileSvr(const struct SvrNet_t* svr = NULL, int32_t n = 0, const std::string& filename = "");

	const std::vector<struct Agnt_t>& Agnts() { return mAgnts;}
	void CleanAgnt();
	bool IsExistAgnt(const struct Agnt_t& agnt, struct Agnt_t* old = NULL);
	bool IsAgntChange(const struct Agnt_t& agnt);
	int SaveAgnt(const struct Agnt_t& agnt);
	int AddAgnt(const struct Agnt_t& agnt);
	int DelAgnt(const struct Agnt_t& agnt);
	int ModifyAgnt(const struct Agnt_t& agnt);
	int GetAgntAll(struct Agnt_t buf[], int32_t start, int32_t size);
	int WriteFileAgnt(const struct Agnt_t* agnt = NULL, int32_t n = 0, const std::string& filename = "");

	const MapRlt_t& GetRltsAll();
	const vRlt_t& Rlts(const std::string& host);
	void CleanRlt();
	bool IsExistRlt(const std::string& host, const struct Rlt_t& rlt = Rlt_t());
	int SaveRlt(const struct Rlt_t& rlt);
	int ModifyRlt(const struct Rlt_t& rlt);
	int AddRlt(const struct Rlt_t& rlt);
	int WriteFileRlt(const struct Rlt_t* rlt = NULL, int32_t n = 0, const std::string& filename = "");

protected:
	std::vector<struct Agnt_t> mAgnts;
	std::map<std::string, std::vector<struct Rlt_t> > mRelations;

	SvrQos   *mSvrQos;
	std::string mSvrFile;
	std::string mQosFile;
	std::string mBaseFile;
	std::string mAgntFile;
	std::string mRltFile;
};

#endif
