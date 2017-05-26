
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include <json/json.h>
#include "wLogger.h"
#include "RouterHttpTask.h"
#include "RouterConfig.h"
#include "RouterServer.h"
#include "SvrCmd.h"

RouterHttpTask::RouterHttpTask(wSocket *socket, int32_t type) : wHttpTask(socket, type) {
	On(CMD_SVR_HTTP, SVR_HTTP_RELOAD, &RouterHttpTask::ReloadSvrReq, this);
	On(CMD_SVR_HTTP, SVR_HTTP_SAVE, &RouterHttpTask::SaveSvrReq, this);
	On(CMD_SVR_HTTP, SVR_HTTP_COVER, &RouterHttpTask::CoverSvrReq, this);
	On(CMD_SVR_HTTP, SVR_HTTP_LIST, &RouterHttpTask::ListSvrReq, this);
}

int RouterHttpTask::ReloadSvrReq(struct Request_t *request) {
	RouterConfig* config = Config<RouterConfig*>();
	RouterServer* server = Server<RouterServer*>();

	config->Qos()->CleanNode();
	config->ParseSvrConf();
	config->WriteFileSvr();

	int32_t start = 0;
	struct SvrResReload_t vRRt;
	do {
		if (!config->Qos()->GetNodeAll(vRRt.mSvr, &vRRt.mNum, start, kMaxNum).Ok() || vRRt.mNum <= 0) {
			break;
		}
		start += vRRt.mNum;
		server->Broadcast(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));
	} while (vRRt.mNum >= kMaxNum);

	Json::Value root;
	root["status"] = Json::Value("200");
	root["msg"] = Json::Value("ok");
	ResponseSet("Content-Type", "application/json; charset=UTF-8");
	Write(root.toStyledString());
	return 0;
}

int RouterHttpTask::SaveSvrReq(struct Request_t *request) {
	RouterConfig* config = Config<RouterConfig*>();
	std::string svrs = http::UrlDecode(QueryGet("svrs"));
	if (!svrs.empty()) {
		Json::Value root;
		struct SvrNet_t* svr = NULL;
		int32_t num = ParseJsonSvr(svrs, &svr);
		if (num > 0) {
			struct SvrResSync_t vRRt;
			for (int32_t i = 0; i < num; i++) {
				if (svr[i].mGid > 0 && svr[i].mWeight >= 0 && config->Qos()->IsExistNode(svr[i])) {	// 旧配置检测到weight、name、idc变化才更新
					if (config->Qos()->IsWNIChange(svr[i])) {
						vRRt.mSvr[vRRt.mNum] = svr[i];
						vRRt.mNum++;
						config->Qos()->SaveNode(svr[i]);
					}
				} else if (svr[i].mGid > 0 && svr[i].mWeight > 0) {	// 添加新配置 weight>0 添加配置
					vRRt.mSvr[vRRt.mNum] = svr[i];
					vRRt.mNum++;
					config->Qos()->SaveNode(svr[i]);
				}

				// 广播agentsvrd
				if (vRRt.mNum >= kMaxNum) {
					Server<RouterServer*>()->Broadcast(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));
					vRRt.mNum = 0;
				}
			}
			if (vRRt.mNum > 0) {
				Server<RouterServer*>()->Broadcast(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));
			}

			config->WriteFileSvr();
			SAFE_DELETE_VEC(svr);

			root["status"] = Json::Value("200");
			root["msg"] = Json::Value("ok");
			ResponseSet("Content-Type", "application/json; charset=UTF-8");
			Write(root.toStyledString());
		} else {
			Error("", "400");
		}
	} else {
		Error("", "400");
	}
	return 0;
}

int RouterHttpTask::CoverSvrReq(struct Request_t *request) {
	RouterConfig* config = Config<RouterConfig*>();
	std::string svrs = http::UrlDecode(QueryGet("svrs"));
	if (!svrs.empty()) {	// json格式svr
		Json::Value root;
		struct SvrNet_t* svr = NULL;
		int32_t num = ParseJsonSvr(svrs, &svr);
		if (num > 0) {
			config->Qos()->CleanNode();	// 清除原有SVR记录

			struct SvrResReload_t vRRt;
			for (int32_t i = 0; i < num; i++) {
				if (svr[i].mGid > 0 && svr[i].mWeight >= 0 && config->Qos()->IsExistNode(svr[i])) {	// 旧配置检测到weight、name、idc变化才更新
					if (config->Qos()->IsWNIChange(svr[i])) {
						vRRt.mSvr[vRRt.mNum] = svr[i];
						vRRt.mNum++;
						config->Qos()->SaveNode(svr[i]);
					}
				} else if (svr[i].mGid > 0 && svr[i].mWeight > 0) {	// 添加新配置 weight>0 添加配置
					vRRt.mSvr[vRRt.mNum] = svr[i];
					vRRt.mNum++;
					config->Qos()->SaveNode(svr[i]);
				}

				// 广播agentsvrd
				if (vRRt.mNum >= kMaxNum) {
					Server<RouterServer*>()->Broadcast(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));
					vRRt.mNum = 0;
				}
			}
			if (vRRt.mNum > 0) {
				Server<RouterServer*>()->Broadcast(reinterpret_cast<char*>(&vRRt), sizeof(vRRt));
			}
			
			config->WriteFileSvr();
			SAFE_DELETE_VEC(svr);

			root["status"] = Json::Value("200");
			root["msg"] = Json::Value("ok");
			ResponseSet("Content-Type", "application/json; charset=UTF-8");
			Write(root.toStyledString());
		} else {
			Error("", "400");
		}
	} else {
		Error("", "400");
	}
	return 0;
}

int RouterHttpTask::ListSvrReq(struct Request_t *request) {
	RouterConfig* config = Config<RouterConfig*>();
	Json::Value root;
	Json::Value svrs;
	int32_t start = 0, num = 0;
	struct SvrNet_t svr[kMaxNum];
	do {
		if (!config->Qos()->GetNodeAll(svr, &num, start, kMaxNum).Ok() || num <= 0) {
			break;
		}
		for (int i = 0; i < num; i++) {
			Json::Value item;
			item["GID"] = Json::Value(svr[i].mGid);
			item["XID"] = Json::Value(svr[i].mXid);
			item["HOST"] = Json::Value(svr[i].mHost);
			item["PORT"] = Json::Value(svr[i].mPort);
			item["WEIGHT"] = Json::Value(svr[i].mWeight);
			item["VERSION"] = Json::Value(svr[i].mVersion);
			item["NAME"] = Json::Value(svr[i].mName);
			item["IDC"] = Json::Value(svr[i].mIdc);
			svrs.append(Json::Value(item));
		}
		start += num;
	} while (num >= kMaxNum);

	root["svrs"] = Json::Value(svrs);
	root["status"] = Json::Value("200");
	root["msg"] = Json::Value("ok");
	ResponseSet("Content-Type", "application/json; charset=UTF-8");
	Write(root.toStyledString());
	return 0;
}

int RouterHttpTask::HardInfoReq(struct Request_t *request) {
	return 0;
}

// [{"gid":1,"xid":1,"host":"127.0.0.1","port":5555,"weight":1000,"name":"test"}]
int32_t RouterHttpTask::ParseJsonSvr(const std::string& svrs, struct SvrNet_t** svr) {
	Json::Reader reader;
	Json::Value value;
	if (reader.parse(svrs, value) && value.size() > 0) {
		SAFE_NEW_VEC(value.size(), struct SvrNet_t, *svr);
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
					(*svr)[i].mWeight = value[i]["weight"].asInt();
				}
				if (value[i].isMember("version")) {
					(*svr)[i].mVersion = value[i]["version"].asInt();
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
