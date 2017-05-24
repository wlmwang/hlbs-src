
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include <json/json.h>
#include "wLogger.h"
#include "RouterHttpTask.h"
#include "RouterConfig.h"
#include "SvrCmd.h"

RouterHttpTask::RouterHttpTask(wSocket *socket, int32_t type) : wHttpTask(socket, type) {
	On(CMD_SVR_HTTP, SVR_HTTP_INIT, &RouterHttpTask::InitSvrReq, this);
	On(CMD_SVR_HTTP, SVR_HTTP_SAVE, &RouterHttpTask::SaveSvrReq, this);
	On(CMD_SVR_HTTP, SVR_HTTP_FSET, &RouterHttpTask::FsetSvrReq, this);
	On(CMD_SVR_HTTP, SVR_HTTP_GALL, &RouterHttpTask::GallSvrReq, this);
	//On(CMD_SVR_HTTP, SVR_HTTP_INFO, &RouterHttpTask::InfoReq, this);
}

int RouterHttpTask::InitSvrReq(struct Request_t *request) {
	RouterConfig* config = Config<RouterConfig*>();
	// 重新加载配置
	config->Qos()->CleanNode();
	config->ParseSvrConf();
	config->WriteSvrConfFile();

	Json::Value root;
	root["status"] = Json::Value("200");
	root["msg"] = Json::Value("ok");
	ResponseSet("Content-Type", "application/json; charset=UTF-8");
	Write(root.toStyledString());
	return 0;
}

int RouterHttpTask::InfoReq(struct Request_t *request) {
	return 0;
}

int RouterHttpTask::GallSvrReq(struct Request_t *request) {
	RouterConfig* config = Config<RouterConfig*>();
	Json::Value root;
	Json::Value svrs;
	int32_t start = 0, num = 0;
	struct SvrNet_t svr[kMaxNum];
	do {
		if (config->Qos()->GetNodeAll(svr, &num, start, kMaxNum).Ok() && num > 0) {
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

int RouterHttpTask::SaveSvrReq(struct Request_t *request) {
	RouterConfig* config = Config<RouterConfig*>();
	std::string svrs = http::UrlDecode(QueryGet("svrs"));
	if (!svrs.empty()) {
		Json::Value root;
		if (SaveJsonSvr(svrs) >= 0) {	// 更新json格式svr记录
			config->WriteSvrConfFile();
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

int RouterHttpTask::FsetSvrReq(struct Request_t *request) {
	RouterConfig* config = Config<RouterConfig*>();
	std::string svrs = http::UrlDecode(QueryGet("svrs"));
	if (!svrs.empty()) {	// json格式svr
		Json::Value root;
		// 清除原有SVR记录
		config->Qos()->CleanNode();
		if (SaveJsonSvr(svrs) >= 0) {	// 更新json格式svr记录
			config->WriteSvrConfFile();
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

// [{"gid":1,"xid":1,"host":"127.0.0.1","port":5555,"weight":1000,"name":"test"}]
int RouterHttpTask::SaveJsonSvr(const std::string& svrs) {
	RouterConfig* config = Config<RouterConfig*>();
	Json::Reader reader;
	Json::Value value;
	if (reader.parse(svrs, value)) {
		for (unsigned int i = 0; i < value.size(); i++) {
			if (value[i].isMember("gid") && value[i].isMember("xid") && value[i].isMember("host") && value[i].isMember("port")) {
				struct SvrNet_t svr;
				svr.mGid = value[i]["gid"].asInt();
				svr.mXid = value[i]["xid"].asInt();
				svr.mPort = value[i]["port"].asInt();
				memcpy(svr.mHost, value[i]["host"].asCString(), kMaxHost);
				if (value[i].isMember("name")) {
					memcpy(svr.mName, value[i]["name"].asCString(), kMaxName);
				}
				if (value[i].isMember("weight")) {
					svr.mWeight = value[i]["weight"].asInt();
				}
				if (value[i].isMember("version")) {
					svr.mVersion = value[i]["version"].asInt();
				}
				if (value[i].isMember("idc")) {
					svr.mIdc = value[i]["idc"].asInt();
				}
				config->Qos()->SaveNode(svr);
			}
		}
		return 0;
	}
	return -1;
}
