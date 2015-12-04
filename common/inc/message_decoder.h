//--------------------------------------------------
// 本文件提供了序列化反序列化各种消息的接口
//-------------------------------------------------- 

#ifndef _MESSAGE_DECODER_H_
#define _MESSAGE_DECODER_H_

#include "message.hxx.pb.h"

#define CREATE_NEW_MSG(msgid,msgname) \
	case msgid: \
	{ \
		pMessage = new(mMessageBuff) msgname; \
		break; \
	}


class CMessageDecoder
{
	public:
		typedef google::protobuf::Message Message;
		CMessageDecoder();
	protected:
		// 反序列化客户端消息
		int GetClientMsgFromBuffer(char *vBuffer, int vBufferLen, CMessage &vMessage, CCSHead &vCSHead);
		// 序列化客户端消息
		int PutClientMsgIntoBuffer(char *vBuffer, int &vBufferLen, CMessage &vMessage, CCSHead &vCSHead, short vDes = 0);
		// 反序列化服务器消息
		int GetServerMsgFromBuffer(char *vBuffer, int vBufferLen, CMessage &vMessage);
		// 序列化服务器消息
		int PutServerMsgIntoBuffer(char *vBuffer, int &vBufferLen, CMessage &vMessage);
		Message *CreateMessage(int vMessageID);
		enum { MAX_MESSAGE_LEN = 1024 * 1024 };
		char mMessageBuff[MAX_MESSAGE_LEN];
};

#endif
