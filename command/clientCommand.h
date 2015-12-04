
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include <string.h>
#include "wType.h"

#pragma pack(1)

enum CLIENT_MSG_ID
{
	C2S_REQUEST_LOGINLOGIC		= 0x0001;	//客户端登录登录服务器并准备跳转
	S2C_RESPONSE_LOGINLOGIC		= 0x0002;	//服务器下发需要跳转的服务器
	C2S_REQUEST_STOREINFO		= 0x0003;	//客户端请求存储手机信息
	C2S_REQUEST_PUSH			= 0x0004;	//客户端push的请求
	S2C_RESPONSE_PUSH			= 0x0005;	//服务器的push回复
	C2S_REQUEST_TEST			= 0x0006;	//测试的请求
	S2C_RESPONSE_TEST			= 0x0007;	//测试的回复
	C2S_REQUEST_PUSHRESULT		= 0x0008;	//客户端上传push的结果
	S2C_RESPONSE_PUSHRESULT		= 0x0009;	//服务器发送push结果的回复
	C2S_REQUEST_UPGRADEDERROR	= 0x000A;	//客户端上传upgraded错误信息
};

/*
//C2S_REQUEST_LOGINLOGIC
message CMessageLoginLogicRequest
{
	optional string Platform	= 1;		// 平台
	optional uint32 Type		= 2;		// 类型
};

//S2C_RESPONSE_LOGINLOGIC
message CMessageLoginLogicResponse
{
	optional string IPAddress	= 1;		// 地址
	optional uint32 Port		= 2;		// 端口
};
*/

#pragma pack()