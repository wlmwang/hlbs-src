
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include <json/json.h>
#include "wLogger.h"
#include "RouterHttpTask.h"
#include "RouterConfig.h"
#include "RouterServer.h"
#include "RouterMaster.h"
#include "wWorker.h"

RouterHttpTask::RouterHttpTask(wSocket *socket, int32_t type) : wHttpTask(socket, type) {
	On(CMD_SVR_HTTP, SVR_HTTP_RELOAD, &RouterHttpTask::ReloadSvrReq, this);
	On(CMD_SVR_HTTP, SVR_HTTP_SAVE, &RouterHttpTask::SaveSvrReq, this);
	On(CMD_SVR_HTTP, SVR_HTTP_COVER, &RouterHttpTask::CoverSvrReq, this);
	On(CMD_SVR_HTTP, SVR_HTTP_LIST, &RouterHttpTask::ListSvrReq, this);

	// v3.0.8起，agent将自动注册到router上
	//On(CMD_AGNT_HTTP, AGNT_HTTP_RELOAD, &RouterHttpTask::ReloadAgntReq, this);
	//On(CMD_AGNT_HTTP, AGNT_HTTP_SAVE, &RouterHttpTask::SaveAgntReq, this);
	//On(CMD_AGNT_HTTP, AGNT_HTTP_COVER, &RouterHttpTask::CoverAgntReq, this);
	On(CMD_AGNT_HTTP, AGNT_HTTP_LIST, &RouterHttpTask::ListAgntReq, this);

	On(CMD_MASTER_HTTP, MASTER_HTTP_RESTART, &RouterHttpTask::MasterRestartReq, this);
	On(CMD_MASTER_HTTP, MASTER_HTTP_RELOAD, &RouterHttpTask::MasterReloadReq, this);
	On(CMD_MASTER_HTTP, MASTER_HTTP_STOP, &RouterHttpTask::MasterStopReq, this);
	On(CMD_MASTER_HTTP, MASTER_HTTP_INFO, &RouterHttpTask::MasterInfoReq, this);

	On(CMD_RLT_HTTP, RLT_HTTP_RELOAD, &RouterHttpTask::ReloadRltReq, this);
	On(CMD_RLT_HTTP, RLT_HTTP_SAVE, &RouterHttpTask::SaveRltReq, this);
	On(CMD_RLT_HTTP, RLT_HTTP_COVER, &RouterHttpTask::CoverRltReq, this);
	On(CMD_RLT_HTTP, RLT_HTTP_LIST, &RouterHttpTask::ListRltReq, this);
}

int RouterHttpTask::ReloadSvrReq(struct Request_t *request) {
	RouterConfig* config = Config<RouterConfig*>();
	RouterServer* server = Server<RouterServer*>();

	// 重新加载配置文件
	config->Qos()->CleanNode();
	config->ParseSvrConf();
	config->WriteFileSvr();

	int32_t start = 0, i = 0;
	struct SvrResReload_t vRRt;
	do {
		vRRt.mNum = config->Qos()->GetNodeAll(vRRt.mSvr, i, start, kMaxNum);
		if (vRRt.mNum <= 0) {
			break;
		}

		server->BroadcastSvr<struct SvrResReload_t>(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));
		AsyncWorker(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));	// 同步其他worker
		
		vRRt.mCode++;
		start += vRRt.mNum;
		i = 0;
	} while (vRRt.mNum >= kMaxNum);

	Json::Value root;
	root["status"] = Json::Value("200");
	root["msg"] = Json::Value("ok");
	ResponseSet("Content-Type", "application/json; charset=UTF-8");
	Write(root.toStyledString());
	return 0;
}

int RouterHttpTask::SaveSvrReq(struct Request_t *request) {
	if (Method() != "POST") {
		Error("", "405");
		return -1;
	}
	RouterConfig* config = Config<RouterConfig*>();
	RouterServer* server = Server<RouterServer*>();

	std::string svrs = http::UrlDecode(FormGet("svrs"));
	if (!svrs.empty()) {
		Json::Value root;
		struct SvrNet_t* svr = NULL;
		int32_t num = ParseJsonSvr(svrs, &svr);
		if (num > 0) {
			struct SvrResSync_t vRRt;
			for (int32_t i = 0; i < num; i++) {
				if (svr[i].mGid > 0 && svr[i].mWeight >= 0 && config->Qos()->IsExistNode(svr[i])) {	// 旧配置检测到weight、name、idc变化才更新
					if (config->Qos()->IsWNIChange(svr[i])) {
						config->Qos()->SaveNode(svr[i]);
						vRRt.mSvr[vRRt.mNum] = svr[i];
						vRRt.mNum++;
					}
				} else if (svr[i].mGid > 0 && svr[i].mWeight > 0) {	// 添加新配置 weight>0 添加配置
					config->Qos()->SaveNode(svr[i]);
					vRRt.mSvr[vRRt.mNum] = svr[i];
					vRRt.mNum++;
				}

				// 广播agentsvrd
				if (vRRt.mNum >= kMaxNum) {
					server->BroadcastSvr<struct SvrResSync_t>(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));
					AsyncWorker(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));	// 同步其他worker
					vRRt.mCode++;
					vRRt.mNum = 0;
				}
			}
			if (vRRt.mNum > 0) {
				server->BroadcastSvr<struct SvrResSync_t>(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));
				AsyncWorker(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));	// 同步其他worker
				vRRt.mCode++;
				vRRt.mNum = 0;
			}

			config->WriteFileSvr();

			root["status"] = Json::Value("200");
			root["msg"] = Json::Value("ok");
			ResponseSet("Content-Type", "application/json; charset=UTF-8");
			Write(root.toStyledString());
		} else {
			Error("", "400");
		}

		HNET_DELETE_VEC(svr);
	} else {
		Error("", "400");
	}
	return 0;
}

int RouterHttpTask::CoverSvrReq(struct Request_t *request) {
	if (Method() != "POST") {
		Error("", "405");
		return -1;
	}
	RouterConfig* config = Config<RouterConfig*>();
	RouterServer* server = Server<RouterServer*>();

	std::string svrs = http::UrlDecode(FormGet("svrs"));
	if (!svrs.empty()) {	// json格式svr
		Json::Value root;
		struct SvrNet_t* svr = NULL;
		int32_t num = ParseJsonSvr(svrs, &svr);
		if (num > 0) {
			// 清除原有SVR记录
			config->Qos()->CleanNode();
			struct SvrResReload_t vRRt;
			for (int32_t i = 0; i < num; i++) {
				if (svr[i].mGid > 0 && svr[i].mWeight >= 0 && config->Qos()->IsExistNode(svr[i])) {	// 旧配置检测到weight、name、idc变化才更新
					if (config->Qos()->IsWNIChange(svr[i])) {
						config->Qos()->SaveNode(svr[i]);
						vRRt.mSvr[vRRt.mNum] = svr[i];
						vRRt.mNum++;
					}
				} else if (svr[i].mGid > 0 && svr[i].mWeight > 0) {	// 添加新配置 weight>0 添加配置
					config->Qos()->SaveNode(svr[i]);
					vRRt.mSvr[vRRt.mNum] = svr[i];
					vRRt.mNum++;
				}

				// 广播agentsvrd
				if (vRRt.mNum >= kMaxNum) {
					server->BroadcastSvr<struct SvrResReload_t>(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));
					AsyncWorker(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));	// 同步其他worker
					vRRt.mCode++;
					vRRt.mNum = 0;
				}
			}
			if (vRRt.mNum > 0) {
				server->BroadcastSvr<struct SvrResReload_t>(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));
				AsyncWorker(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));	// 同步其他worker
				vRRt.mCode++;
				vRRt.mNum = 0;
			}
			
			config->WriteFileSvr();

			root["status"] = Json::Value("200");
			root["msg"] = Json::Value("ok");
			ResponseSet("Content-Type", "application/json; charset=UTF-8");
			Write(root.toStyledString());
		} else {
			Error("", "400");
		}

		HNET_DELETE_VEC(svr);
	} else {
		Error("", "400");
	}
	return 0;
}

int RouterHttpTask::ListSvrReq(struct Request_t *request) {
	RouterConfig* config = Config<RouterConfig*>();

	Json::Value root;
	Json::Value svrs;
	int32_t start = 0, num = 0, j = 0;
	struct SvrNet_t svr[kMaxNum];
	do {
		num = config->Qos()->GetNodeAll(svr, j, start, kMaxNum);
		if (num <= 0) {
			break;
		}

		for (int i = 0; i < num; i++) {
			Json::Value item;
			item["gid"] = Json::Value(svr[i].mGid);
			item["xid"] = Json::Value(svr[i].mXid);
			item["host"] = Json::Value(svr[i].mHost);
			item["port"] = Json::Value(svr[i].mPort);
			item["weight"] = Json::Value(svr[i].mWeight);
			item["name"] = Json::Value(svr[i].mName);
			item["idc"] = Json::Value(svr[i].mIdc);
			item["version"] = Json::Value(svr[i].mVersion);
			svrs.append(Json::Value(item));
		}
		start += num;
		j = 0;
	} while (num >= kMaxNum);

	root["svrs"] = Json::Value(svrs);
	root["status"] = Json::Value("200");
	root["msg"] = Json::Value("ok");
	ResponseSet("Content-Type", "application/json; charset=UTF-8");
	Write(root.toStyledString());
	return 0;
}

// @TODO
// 单进程（没有同步worker逻辑）
int RouterHttpTask::ReloadRltReq(struct Request_t *request) {
	RouterConfig* config = Config<RouterConfig*>();
	RouterServer* server = Server<RouterServer*>();

	// 重新加载配置文件
	config->CleanRlt();
	config->ParseRltConf();
	config->WriteFileRlt();

	struct SvrReqInit_t vRRt;
	server->Broadcast(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));

	Json::Value root;
	root["status"] = Json::Value("200");
	root["msg"] = Json::Value("ok");
	ResponseSet("Content-Type", "application/json; charset=UTF-8");
	Write(root.toStyledString());
	return 0;
}

// @TODO
// 单进程（没有同步worker逻辑）
int RouterHttpTask::SaveRltReq(struct Request_t *request) {
	if (Method() != "POST") {
		Error("", "405");
		return -1;
	}
	RouterConfig* config = Config<RouterConfig*>();
	RouterServer* server = Server<RouterServer*>();

	std::string rlts = http::UrlDecode(FormGet("rlts"));
	if (!rlts.empty()) {
		Json::Value root;
		struct Rlt_t* rlt = NULL;
		int32_t num = ParseJsonRlt(rlts, &rlt);
		if (num > 0) {
			for (int32_t i = 0; i < num; i++) {
				if (rlt[i].mGid > 0 && rlt[i].mXid && rlt[i].mHost[0] > 0 && rlt[i].mWeight >= 0) {
					config->SaveRlt(rlt[i]);
				}
			}

			config->WriteFileRlt();

			struct SvrReqInit_t vRRt;
			server->Broadcast(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));

			root["status"] = Json::Value("200");
			root["msg"] = Json::Value("ok");
			ResponseSet("Content-Type", "application/json; charset=UTF-8");
			Write(root.toStyledString());
		} else {
			Error("", "400");
		}

		HNET_DELETE_VEC(rlt);
	} else {
		Error("", "400");
	}
	return 0;
}

// @TODO
// 单进程（没有同步worker逻辑）
int RouterHttpTask::CoverRltReq(struct Request_t *request) {
	if (Method() != "POST") {
		Error("", "405");
		return -1;
	}
	RouterConfig* config = Config<RouterConfig*>();
	RouterServer* server = Server<RouterServer*>();
	
	std::string rlts = http::UrlDecode(FormGet("rlts"));
	if (!rlts.empty()) {	// json格式svr
		Json::Value root;
		struct Rlt_t* rlt = NULL;
		int32_t num = ParseJsonRlt(rlts, &rlt);

		if (num > 0) {
			// 清除原有Rlt记录
			config->CleanRlt();
			for (int32_t i = 0; i < num; i++) {
				if (rlt[i].mGid > 0 && rlt[i].mXid && rlt[i].mHost[0] > 0 && rlt[i].mWeight >= 0) {
					config->SaveRlt(rlt[i]);
				}
			}

			config->WriteFileRlt();

			struct SvrReqInit_t vRRt;
			server->Broadcast(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));

			root["status"] = Json::Value("200");
			root["msg"] = Json::Value("ok");
			ResponseSet("Content-Type", "application/json; charset=UTF-8");
			Write(root.toStyledString());
		} else {
			Error("", "400");
		}

		HNET_DELETE_VEC(rlt);
	} else {
		Error("", "400");
	}
	return 0;
}

int RouterHttpTask::ListRltReq(struct Request_t *request) {
	RouterConfig* config = Config<RouterConfig*>();
	Json::Value root;
	Json::Value rlts;
	const RouterConfig::MapRlt_t& allrlts = config->GetRltsAll();
	for (RouterConfig::MapRltCIt_t it = allrlts.begin(); it != allrlts.end(); it++) {
		if (it->second.empty()) {
			continue;
		}
		for (RouterConfig::vRltCIt_t rlt = it->second.begin(); rlt != it->second.end(); rlt++) {
			Json::Value item;
			item["gid"] = Json::Value(rlt->mGid);
			item["xid"] = Json::Value(rlt->mXid);
			item["host"] = Json::Value(rlt->mHost);
			item["weight"] = Json::Value(rlt->mWeight);
			rlts.append(Json::Value(item));
		}
	}

	root["rlts"] = Json::Value(rlts);
	root["status"] = Json::Value("200");
	root["msg"] = Json::Value("ok");
	ResponseSet("Content-Type", "application/json; charset=UTF-8");
	Write(root.toStyledString());
	return 0;
}

// ignore
int RouterHttpTask::ReloadAgntReq(struct Request_t *request) {
	RouterConfig* config = Config<RouterConfig*>();

	// 重新加载配置文件
	config->CleanAgnt();
	config->ParseAgntConf();
	config->WriteFileAgnt();

	int32_t start = 0;
	struct AgntResReload_t vRRt;
	do {
		vRRt.mNum = config->GetAgntAll(vRRt.mAgnt, start, kMaxNum);
		if (vRRt.mNum <= 0) {
			break;
		}
		Server<RouterServer*>()->Broadcast(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));
		AsyncWorker(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));	// 同步其他worker
		vRRt.mCode++;
		start += vRRt.mNum;
	} while (vRRt.mNum >= kMaxNum);

	Json::Value root;
	root["status"] = Json::Value("200");
	root["msg"] = Json::Value("ok");
	ResponseSet("Content-Type", "application/json; charset=UTF-8");
	Write(root.toStyledString());
	return 0;
}

// ignore
int RouterHttpTask::SaveAgntReq(struct Request_t *request) {
	if (Method() != "POST") {
		Error("", "405");
		return -1;
	}
	RouterConfig* config = Config<RouterConfig*>();

	std::string agnts = http::UrlDecode(FormGet("agnts"));
	if (!agnts.empty()) {
		Json::Value root;
		struct Agnt_t* agnt = NULL;
		int32_t num = ParseJsonAgnt(agnts, &agnt);
		if (num > 0) {
			struct AgntResSync_t vRRt;
			for (int32_t i = 0; i < num; i++) {
				agnt[i].mConfig = 0;
				struct Agnt_t old;
				if (agnt[i].mHost[0] > 0 && agnt[i].mWeight >= 0 && config->IsExistAgnt(agnt[i], &old)) {	// 旧配置检测到status、idc变化才更新
					if (agnt[i].mIdc != old.mIdc || agnt[i].mWeight != old.mWeight || 
						(agnt[i].mIdc == old.mIdc && agnt[i].mWeight == old.mWeight && old.mStatus == kAgntUreg)) {
						if (agnt[i].mWeight == 0) {
							agnt[i].mConfig = -1;
							if (old.mStatus == kAgntOk) {
								agnt[i].mStatus = kAgntUreg;
							} else {
								agnt[i].mStatus = old.mStatus;
							}
						} else if (old.mStatus == kAgntUreg || old.mStatus == kAgntOk) {		// 状态转换
							agnt[i].mStatus = kAgntOk;
						} else {
							agnt[i].mStatus = old.mStatus;
						}
						vRRt.mAgnt[vRRt.mNum] = agnt[i];
						vRRt.mNum++;
						config->SaveAgnt(agnt[i]);
					}
				} else if (agnt[i].mHost[0] > 0 && agnt[i].mWeight > 0) {	// 添加新配置 host>0 添加配置
					vRRt.mAgnt[vRRt.mNum] = agnt[i];
					vRRt.mNum++;
					config->SaveAgnt(agnt[i]);
				}

				// 广播agentsvrd
				if (vRRt.mNum >= kMaxNum) {
					Server<RouterServer*>()->Broadcast(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));
					AsyncWorker(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));	// 同步其他worker
					vRRt.mCode++;
					vRRt.mNum = 0;
				}
			}
			if (vRRt.mNum > 0) {
				Server<RouterServer*>()->Broadcast(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));
				AsyncWorker(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));	// 同步其他worker
				vRRt.mCode++;
				vRRt.mNum = 0;
			}

			config->WriteFileAgnt();

			root["status"] = Json::Value("200");
			root["msg"] = Json::Value("ok");
			ResponseSet("Content-Type", "application/json; charset=UTF-8");
			Write(root.toStyledString());
		} else {
			Error("", "400");
		}

		HNET_DELETE_VEC(agnt);
	} else {
		Error("", "400");
	}
	return 0;
}

// ignore
int RouterHttpTask::CoverAgntReq(struct Request_t *request) {
	if (Method() != "POST") {
		Error("", "405");
		return -1;
	}
	RouterConfig* config = Config<RouterConfig*>();
	std::string agnts = http::UrlDecode(FormGet("agnts"));
	if (!agnts.empty()) {	// json格式svr
		Json::Value root;
		struct Agnt_t* agnt = NULL;
		int32_t num = ParseJsonAgnt(agnts, &agnt);
		if (num > 0) {
			std::vector<struct Agnt_t> oldagnts(config->Agnts());	// 备份原始AGENT记录
			config->CleanAgnt();	// 清除原始AGENT记录

			struct AgntResReload_t vRRt;
			for (int32_t i = 0; i < num; i++) {
				agnt[i].mConfig = 0;
				std::vector<struct Agnt_t>::iterator it = std::find(oldagnts.begin(), oldagnts.end(), agnt[i]);
				if (agnt[i].mHost[0] > 0 && agnt[i].mWeight >= 0 && it != oldagnts.end()) {
					if (agnt[i].mIdc != it->mIdc || agnt[i].mWeight != it->mWeight || 
						(agnt[i].mIdc == it->mIdc && agnt[i].mWeight == it->mWeight && it->mStatus != kAgntOk)) {
						if (agnt[i].mWeight == 0) {
							agnt[i].mConfig = -1;
							if (it->mStatus == kAgntOk) {
								agnt[i].mStatus = kAgntUreg;
							} else {
								agnt[i].mStatus = it->mStatus;
							}
						} else if (it->mStatus == kAgntUreg || it->mStatus == kAgntOk) {
							agnt[i].mStatus = kAgntOk;
						} else {
							agnt[i].mStatus = it->mStatus;
						}
					} else if (it->mStatus == kAgntUreg || it->mStatus == kAgntOk) {
						agnt[i].mStatus = kAgntOk;
					} else {
						agnt[i].mStatus = it->mStatus;
					}
					vRRt.mAgnt[vRRt.mNum] = agnt[i];
					vRRt.mNum++;
					config->SaveAgnt(agnt[i]);
					oldagnts.erase(it);
				} else if (agnt[i].mHost[0] > 0 && agnt[i].mWeight > 0) {
					config->SaveAgnt(agnt[i]);
					vRRt.mAgnt[vRRt.mNum] = agnt[i];
					vRRt.mNum++;
				}

				// 广播agentsvrd
				if (vRRt.mNum >= kMaxNum) {
					Server<RouterServer*>()->Broadcast(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));
					AsyncWorker(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));	// 同步其他worker
					vRRt.mCode++;
					vRRt.mNum = 0;
				}
			}
			if (vRRt.mNum > 0) {
				Server<RouterServer*>()->Broadcast(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));
				AsyncWorker(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));	// 同步其他worker
				vRRt.mCode++;
				vRRt.mNum = 0;
			}
			
			// 已连接agent列表信息，原样加入列表
			if (!oldagnts.empty()) {
				for (std::vector<struct Agnt_t>::iterator it = oldagnts.begin(); it != oldagnts.end(); it++) {
					if (it->mConfig == 0) {
						if (it->mStatus == kAgntOk) {
							it->mConfig = -1;
							it->mStatus = kAgntUreg;
							config->SaveAgnt(*it);
							vRRt.mAgnt[vRRt.mNum] = *it;
							vRRt.mNum++;
						}
					} else if (it->mConfig == -1) {
						if (it->mStatus == kAgntUreg) {
							config->SaveAgnt(*it);
							vRRt.mAgnt[vRRt.mNum] = *it;
							vRRt.mNum++;
						}
					}

					// 广播agentsvrd
					if (vRRt.mNum >= kMaxNum) {
						Server<RouterServer*>()->Broadcast(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));
						AsyncWorker(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));	// 同步其他worker
						vRRt.mCode++;
						vRRt.mNum = 0;
					}
				}
				if (vRRt.mNum > 0) {
					Server<RouterServer*>()->Broadcast(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));
					AsyncWorker(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));	// 同步其他worker
					vRRt.mCode++;
					vRRt.mNum = 0;
				}
			}

			config->WriteFileAgnt();

			root["status"] = Json::Value("200");
			root["msg"] = Json::Value("ok");
			ResponseSet("Content-Type", "application/json; charset=UTF-8");
			Write(root.toStyledString());
		} else {
			Error("", "400");
		}

		HNET_DELETE_VEC(agnt);
	} else {
		Error("", "400");
	}
	return 0;
}

// ignore
int RouterHttpTask::ListAgntReq(struct Request_t *request) {
	RouterConfig* config = Config<RouterConfig*>();
	Json::Value root;
	Json::Value agnts;
	int32_t start = 0, num = 0;
	struct Agnt_t agnt[kMaxNum];
	do {
		num = config->GetAgntAll(agnt, start, kMaxNum);
		if (num <= 0) {
			break;
		}
		for (int i = 0; i < num; i++) {
			Json::Value item;
			item["config"] = Json::Value(agnt[i].mConfig);
			item["host"] = Json::Value(agnt[i].mHost);
			item["port"] = Json::Value(agnt[i].mPort);
			item["weight"] = Json::Value(agnt[i].mWeight);
			item["status"] = Json::Value(agnt[i].mStatus);
			item["idc"] = Json::Value(agnt[i].mIdc);
			item["version"] = Json::Value(agnt[i].mVersion);
			item["name"] = Json::Value(agnt[i].mName);
			agnts.append(Json::Value(item));
		}
		start += num;
	} while (num >= kMaxNum);

	root["agnts"] = Json::Value(agnts);
	root["status"] = Json::Value("200");
	root["msg"] = Json::Value("ok");
	ResponseSet("Content-Type", "application/json; charset=UTF-8");
	Write(root.toStyledString());
	return 0;
}

int RouterHttpTask::MasterRestartReq(struct Request_t *request) {
	int ret = Server()->Master()->SignalProcess("restart");
	if (ret == -1) {
		Error("", "500");
		return -1;
	}
	Error("", "200");
	return 0;
}

// 不会返回，服务器直接退出！
int RouterHttpTask::MasterStopReq(struct Request_t *request) {
	int ret = Server()->Master()->SignalProcess("stop");
	if (ret == -1) {
		Error("", "500");
		return -1;
	}
	Error("", "200");
	return 0;
}

int RouterHttpTask::MasterReloadReq(struct Request_t *request) {
	Error("", "400");
	return -1;
}

int RouterHttpTask::MasterInfoReq(struct Request_t *request) {
	if (Server()->Master()->WorkerNum() <= 0) {
		Error("", "400");
		return -1;
	}
	Json::Value root;
	Json::Value workers;
	for (uint32_t i = 0; i < kMaxProcess; i++) {
		wWorker* worker = Server()->Master()->Worker(i);
		if (worker->Pid() > 0) {
			Json::Value item;
			item["pid"] = Json::Value(worker->Pid());
			item["name"] = Json::Value(worker->Title() + " - worker");
			item["timeline"] = Json::Value(worker->Timeline());
			item["respawn"] = Json::Value(worker->Respawn());
			item["exited"] = Json::Value(worker->Exited());
			item["exiting"] = Json::Value(worker->Exiting());
			workers.append(Json::Value(item));
		}
	}
	root["version"] = Json::Value(soft::GetSoftVer());
	root["pid"] = Json::Value(Server()->Master()->Pid());
	root["name"] = Json::Value(Server()->Master()->Title() + " - master");
	root["num"] = Json::Value(Server()->Master()->WorkerNum());
	root["workers"] = Json::Value(workers);
	root["status"] = Json::Value("200");
	root["msg"] = Json::Value("ok");
	ResponseSet("Content-Type", "application/json; charset=UTF-8");
	Write(root.toStyledString());
	return 0;
}

// [{"gid":1,"xid":1,"host":"127.0.0.1","port":5555,"weight":1000,"name":"test"}]
int32_t RouterHttpTask::ParseJsonSvr(const std::string& svrs, struct SvrNet_t** svr) {
	Json::Reader reader;
	Json::Value value;
	if (reader.parse(svrs, value) && value.size() > 0) {
		HNET_NEW_VEC(value.size(), struct SvrNet_t, *svr);
		for (unsigned int i = 0; i < value.size(); i++) {
			if (value[i].isMember("gid") && value[i].isMember("xid") && value[i].isMember("host") && value[i].isMember("port")) {
				(*svr)[i].mGid = value[i]["gid"].asInt();
				(*svr)[i].mXid = value[i]["xid"].asInt();
				(*svr)[i].mPort = value[i]["port"].asInt();
				memcpy((*svr)[i].mHost, value[i]["host"].asCString(), kMaxHost);
				if (value[i].isMember("name")) {
					memcpy((*svr)[i].mName, value[i]["name"].asCString(), kMaxName);
				}
				if (value[i].isMember("weight")) {
					int wt = value[i]["weight"].asInt();
					if (wt >= 0 && wt <= kMaxWeight) {
						(*svr)[i].mWeight = wt;
					}
				}
				if (value[i].isMember("idc")) {
					(*svr)[i].mIdc = value[i]["idc"].asInt();
				}
			}
		}
		return value.size();
	}
	return -1;
}

// [{"host":"192.168.8.13", "weight":1, "idc":0}]
int32_t RouterHttpTask::ParseJsonAgnt(const std::string& agnts, struct Agnt_t** agnt) {
	Json::Reader reader;
	Json::Value value;
	if (reader.parse(agnts, value) && value.size() > 0) {
		HNET_NEW_VEC(value.size(), struct Agnt_t, *agnt);
		for (unsigned int i = 0; i < value.size(); i++) {
			if (value[i].isMember("host")) {
				memcpy((*agnt)[i].mHost, value[i]["host"].asCString(), kMaxHost);
				if (value[i].isMember("port")) {
					(*agnt)[i].mPort = value[i]["port"].asInt();
				}
				if (value[i].isMember("weight")) {
					int wt = value[i]["weight"].asInt();
					if (wt >= 0) {
						(*agnt)[i].mWeight = wt;
					}
				}
				if (value[i].isMember("idc")) {
					(*agnt)[i].mIdc = value[i]["idc"].asInt();
				}
				if (value[i].isMember("name")) {
					memcpy((*agnt)[i].mName, value[i]["name"].asCString(), kMaxName);
				}
			}
		}
		return value.size();
	}
	return -1;
}

// [{"host":"192.168.8.13", "gid":1, "xid":1, "weight":1}]
int32_t RouterHttpTask::ParseJsonRlt(const std::string& rlts, struct Rlt_t** rlt) {
	Json::Reader reader;
	Json::Value value;
	if (reader.parse(rlts, value) && value.size() > 0) {
		HNET_NEW_VEC(value.size(), struct Rlt_t, *rlt);
		for (unsigned int i = 0; i < value.size(); i++) {
			if (value[i].isMember("gid") && value[i].isMember("xid") && value[i].isMember("host")) {
				(*rlt)[i].mGid = value[i]["gid"].asInt();
				(*rlt)[i].mXid = value[i]["xid"].asInt();
				memcpy((*rlt)[i].mHost, value[i]["host"].asCString(), kMaxHost);
				if (value[i].isMember("weight")) {
					int wt = value[i]["weight"].asInt();
					if (wt >= 0) {
						(*rlt)[i].mWeight = wt;
					}
				}
			}
		}
		return value.size();
	}
	return -1;
}
