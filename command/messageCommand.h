
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include <string.h>
#include "wType.h"

#pragma pack(1)

/*
//客户端与服务器之间的消息头
message CCSHead
{
    optional uint32		Uin			= 1;	// 玩家ID
    optional uint32		Token		= 2;	// 登录令牌
};

// 消息头
message CMessageHead
{
	optional uint32 MessageID	= 1;		// 消息ID
	optional uint32 SrcFE		= 2; 		// 消息源实体
	optional uint32 DstFE		= 3; 		// 消息目的实体
	optional uint64 SrcID		= 4; 		// 消息源ID
	optional uint64 DstID		= 5; 		// 消息目的ID
	optional uint32 Option		= 6;		// 保留字
};

// 消息
message CMessage
{
	required CMessageHead	MsgHead = 1;	// 消息头
	optional fixed64		MsgBody = 2;	// 消息体( 指针 8bytes )
};
*/

#pragma pack()