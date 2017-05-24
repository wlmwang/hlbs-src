
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _ROUTER_HTTP_TASK_H_
#define _ROUTER_HTTP_TASK_H_

#include "wCore.h"
#include "wStatus.h"
#include "wMisc.h"
#include "wHttpTask.h"

using namespace hnet;

const uint8_t CMD_SVR_HTTP = 70;

// 重载SVR记录
const uint8_t SVR_HTTP_INIT = 0;

// 更新SVR记录
const uint8_t SVR_HTTP_SAVE = 1;

// 设置SVR记录
const uint8_t SVR_HTTP_FSET = 2;

// 查找所有SVR记录
const uint8_t SVR_HTTP_GALL = 3;

// 服务器统计信息
const uint8_t SVR_HTTP_INFO = 4;

class RouterHttpTask : public wHttpTask {
public:
	RouterHttpTask(wSocket *socket, int32_t type);

	int SaveSvrReq(struct Request_t *request);
	int InitSvrReq(struct Request_t *request);
	int FsetSvrReq(struct Request_t *request);
	int GallSvrReq(struct Request_t *request);
	int InfoReq(struct Request_t *request);
	
protected:
	int SaveJsonSvr(const std::string& svrs);
};

#endif
