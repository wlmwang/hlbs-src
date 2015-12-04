
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include <string.h>
#include "wType.h"

#pragma pack(1)


enum SERVER_MSG_ID
{
	S2S_REQUEST_REGISTER_SERVER		= 0x0101;			// 向服务器注册
	S2S_RESPONSE_REGISTER_SERVER	= 0x0102;			// 向服务器注册的回复
	S2S_REQUEST_EXECUTESQL			= 0x0103;			// 服务器执行sql请求
	S2S_RESPONSE_EXECUTESQL			= 0x0104;			// 执行sql的回复
	S2S_REQUEST_STOREERROR			= 0x0105;			// db线程请求存储错误信息
	S2S_REQUEST_PUSHINFO			= 0x0106;			// 服务器请求push信息
	S2S_RESPONSE_PUSHINFO			= 0x0107;			// 对push信息请求的回复
	S2S_INFO_UPDATESETTING			= 0x0108;			// db内部的更新配置的通知
	S2S_REQUEST_CLIENTTEST			= 0x0109;			// 客户端测试消息
	S2S_RESPONSE_CLIENTTEST			= 0x010A;			// 客户端测试消息的回复
	S2S_REQUEST_CLIENTPUSHRESULT	= 0x010B;			// 客户端上传push结果
	S2S_RESPONSE_CLIENTPUSHRESULT	= 0x010C;			// 服务器返回更新push信息的结果
};

/*
//ID_S2S_REQUEST_REGISTER_SERVER
message CMessageRegisterServerRequest
{
	optional string IPAddress	= 1;	// 对外网的IP地址
	optional uint32 Port		= 2;	// 对外网的端口
};

//ID_S2S_RESPONSE_REGISTER_SERVER 
message CMessageRegisterServerResponse
{
};
*/

#pragma pack()