#include "message_decoder.h"
#include "zlib.h"
#include "my_assert.h"
#include "servermessage.hxx.pb.h"
#include "clientmessage.hxx.pb.h"

#define UNCOMPRESSIONSIZE 1024 * 1024

CMessageDecoder::CMessageDecoder()
{
	memset(&mMessageBuff, 0, sizeof(mMessageBuff));
}

// 此vBufferLen包括消息总长度
int CMessageDecoder::GetClientMsgFromBuffer(char *vBuffer, int vBufferLen, CMessage &vMessage, CCSHead &vCSHead)
{
	// 一个完整的客户端消息一个总长度，一个转发位置长度，一个CCSHead的长度，一个msghead的长度，一个消息体的长度
	MY_ASSERT(vBuffer != NULL && vBufferLen > sizeof(int) + sizeof(short) + sizeof(short) + sizeof(short) + sizeof(int), return 1);

	char *pBuffer = vBuffer;
	int iLeftLen = vBufferLen;
	// 总长度判断
	MY_ASSERT(*(int *)pBuffer + sizeof(int) == vBufferLen, return 2);
	pBuffer += sizeof(int);
	iLeftLen -= sizeof(int);

	// 跳过需要转发的位置
	pBuffer += sizeof(short);
	iLeftLen -= sizeof(short);

	// CCSHead
	int iLen = (int)(*(unsigned short *)pBuffer);
	pBuffer += sizeof(short);
	iLeftLen -= sizeof(short);

	bool bRet = vCSHead.ParseFromArray(pBuffer, iLen);
	if( !bRet )
	{
		//LOG_ERROR("default", "client message: ParseFromArray CCSHead failed");
		return 3;
	}

	// message head
	pBuffer += iLen;
	iLeftLen -= iLen;

	iLen = (int)(*(unsigned short *)pBuffer);
	pBuffer += sizeof(short);
	iLeftLen -= sizeof(short);

	bRet = vMessage.mutable_msghead()->ParseFromArray(pBuffer, iLen);
	if( !bRet )
	{
		//LOG_ERROR("default", "client message: ParseFromArray message head failed");
		return 4;
	}

	// 创建message body
	Message *pMessage = CreateMessage(vMessage.msghead().messageid());
	if( pMessage == NULL )
	{
		//LOG_ERROR("default", "client message: new message by id(%d) failed", vMessage.msghead().messageid());
		return 5;
	}

	// message body
	pBuffer += iLen;
	iLeftLen -= iLen;

	iLen = *(int *)pBuffer;
	pBuffer += sizeof(int);
	iLeftLen -= sizeof(int);

	char szBuffer[UNCOMPRESSIONSIZE];
	unsigned long iUncompressionSize = UNCOMPRESSIONSIZE;
	if( vMessage.msghead().messageid() == ID_C2S_REQUEST_STOREINFO )
	{
		int iRet = uncompress((Bytef *)szBuffer, &iUncompressionSize, (const Bytef *)pBuffer, iLen);
		if( iRet != Z_OK ) 
		{
			pMessage->~Message();
			return 8;
		}

		bRet = pMessage->ParseFromArray(szBuffer, iUncompressionSize);
		if( !bRet ) 
		{
			pMessage->~Message();
			return 9;
		}
	}
	else 
	{
		bRet = pMessage->ParseFromArray(pBuffer, iLen);
		if( !bRet )
		{
			//LOG_ERROR("default", "client message: ParseFromArray message body failed");
			pMessage->~Message();
			return 6;
		}
	}

	// 判断总长度
	iLeftLen -= iLen;
	if( iLeftLen != 0 )
	{
		//LOG_ERROR("default", "client message: message len error");
		pMessage->~Message();
		return 7;
	}

	// 设置成相应的消息体
	vMessage.set_msgbody((unsigned long long)pMessage);

	return 0;
}

int CMessageDecoder::PutClientMsgIntoBuffer(char *vBuffer, int &vBufferLen, CMessage &vMessage, CCSHead &vCSHead, short vDes)
{
	MY_ASSERT(vBuffer != NULL && vBufferLen > sizeof(int) + sizeof(short) + sizeof(short) + sizeof(short) + sizeof(int), return 1);

	char *pBuffer = vBuffer;
	int iLeftLen = vBufferLen;
	int iTotalLen = 0;

	// 跳过总长度
	pBuffer += sizeof(int);
	iLeftLen -= sizeof(int);
	// iTotalLen += sizeof(int);

	// 目标服务器位置
	*(short *)pBuffer = vDes;
	pBuffer += sizeof(short);
	iLeftLen -= sizeof(short);
	iTotalLen += sizeof(short);

	// 跳过CCSHead的长度
	pBuffer += sizeof(short);
	iLeftLen -= sizeof(short);
	iTotalLen += sizeof(short);

	bool bRet = vCSHead.SerializeToArray(pBuffer, iLeftLen);
	if( !bRet )
	{
		//LOG_ERROR("default", "client message: SerializeToArray CCSHead failed");
		return 2;
	}

	// CCSHead的长度赋值
	int iTempLen = vCSHead.ByteSize();
	*(short *)(pBuffer - sizeof(short)) = (short)iTempLen;

	pBuffer += iTempLen;
	iLeftLen -= iTempLen;
	iTotalLen += iTempLen;

	// 跳过message head的长度
	pBuffer += sizeof(short);
	iLeftLen -= sizeof(short);
	iTotalLen += sizeof(short);

	bRet = vMessage.msghead().SerializeToArray(pBuffer, iLeftLen);
	if( !bRet )
	{
		//LOG_ERROR("default", "client message: ParseFromArray message head failed");
		return 3;
	}

	// message head的长度赋值
	iTempLen = vMessage.msghead().ByteSize();
	*(short *)(pBuffer - sizeof(short)) = (short)iTempLen;

	pBuffer += iTempLen;
	iLeftLen -= iTempLen;
	iTotalLen += iTempLen;

	// 跳过message body的长度
	pBuffer += sizeof(int);
	iLeftLen -= sizeof(int);
	iTotalLen += sizeof(int);

	Message* pMsgBody = (Message *)vMessage.msgbody();
	MY_ASSERT(pMsgBody != NULL, return 4);

	bRet = pMsgBody->SerializeToArray(pBuffer, iLeftLen);
	if( !bRet )
	{
		//LOG_ERROR("default", "client message: SerializeToArray message body failed");
		return 5;
	}

	// message body的长度赋值
	iTempLen = pMsgBody->ByteSize();
	*(int *)(pBuffer - sizeof(int)) = iTempLen;

	pBuffer += iTempLen;
	iLeftLen -= iTempLen;
	iTotalLen += iTempLen;

	// 消息总长度赋值
	pBuffer = vBuffer;
	*(int *)pBuffer = iTotalLen;

	// 消息长度判断
	if( iLeftLen < 0 )
	{
		//LOG_ERROR("default", "client message: buffer len is too short");
		return 6;
	}

	// 消息体的总长度赋值
	vBufferLen = iTotalLen + sizeof(int);
	return 0;
}

// 此vBufferLen包括消息总长度
int CMessageDecoder::GetServerMsgFromBuffer(char *vBuffer, int vBufferLen, CMessage &vMessage)
{
	// 一个完整的服务器消息一个总长度，一个msghead的长度，一个消息体的长度
	MY_ASSERT(vBuffer != NULL && vBufferLen > sizeof(int) + sizeof(short), return 1);

	char *pBuffer = vBuffer;
	int iLeftLen = vBufferLen;

	//--------------------------------------------------
	// // 总长度判断
	// MY_ASSERT(*(int *)pBuffer + sizeof(int) == vBufferLen, return 2);
	// pBuffer += sizeof(int);
	// iLeftLen -= sizeof(int);
	//-------------------------------------------------- 

	// message head
	int iLen = (int)(*(unsigned short *)pBuffer);
	pBuffer += sizeof(short);
	iLeftLen -= sizeof(short);

	bool bRet = vMessage.mutable_msghead()->ParseFromArray(pBuffer, iLen);
	if( !bRet )
	{
		//LOG_ERROR("default", "server message: ParseFromArray message head failed");
		return 3;
	}

	// 创建message body
	Message *pMessage = CreateMessage(vMessage.msghead().messageid());
	if( pMessage == NULL )
	{
		//LOG_ERROR("default", "new message by id(%d) failed", vMessage.msghead().messageid());
		return 4;
	}

	// message body
	pBuffer += iLen;
	iLeftLen -= iLen;

	iLen = *(int *)pBuffer;
	pBuffer += sizeof(int);
	iLeftLen -= sizeof(int);

	char szBuffer[UNCOMPRESSIONSIZE];
	unsigned long iUncompressionSize = UNCOMPRESSIONSIZE;
	if( vMessage.msghead().messageid() == ID_C2S_REQUEST_STOREINFO ) 
	{
		int iRet = uncompress((Bytef *)szBuffer, &iUncompressionSize, (const Bytef *)pBuffer, iLen);
		if( iRet != Z_OK ) 
		{
			pMessage->~Message();
			return 7;
		}

		bRet = pMessage->ParseFromArray(szBuffer, iUncompressionSize);
		if( !bRet ) 
		{
			pMessage->~Message();
			return 8;
		}
	}
	else
	{
		bRet = pMessage->ParseFromArray(pBuffer, iLen);
		if( !bRet )
		{
			//LOG_ERROR("default", "ParseFromArray message body failed");
			pMessage->~Message();
			return 5;
		}
	}

	// 判断总长度
	iLeftLen -= iLen;
	if( iLeftLen != 0 )
	{
		//LOG_ERROR("default", "message len error");
		pMessage->~Message();
		return 6;
	}

	// 设置成相应的消息体
	vMessage.set_msgbody((unsigned long long)pMessage);

	return 0;
}

int CMessageDecoder::PutServerMsgIntoBuffer(char *vBuffer, int &vBufferLen, CMessage &vMessage)
{
	MY_ASSERT(vBuffer != NULL && vBufferLen > sizeof(int) + sizeof(short) + sizeof(int), return 1);

	char *pBuffer = vBuffer;
	int iLeftLen = vBufferLen;
	int iTotalLen = 0;

	// 跳过总长度
	pBuffer += sizeof(int);
	iLeftLen -= sizeof(int);
	//iTotalLen += sizeof(int);

	// 跳过message head的长度
	pBuffer += sizeof(short);
	iLeftLen -= sizeof(short);
	iTotalLen += sizeof(short);

	bool bRet = vMessage.msghead().SerializeToArray(pBuffer, iLeftLen);
	if( !bRet )
	{
		//LOG_ERROR("default", "server message: SerializeToArray message head failed");
		return 2;
	}

	// 对message head的长度进行赋值
	int iTempLen = vMessage.msghead().ByteSize();
	*(short *)(pBuffer - sizeof(short)) = iTempLen;

	pBuffer += iTempLen;
	iLeftLen -= iTempLen;
	iTotalLen += iTempLen;

	// 跳过message body的长度
	pBuffer += sizeof(int);
	iLeftLen -= sizeof(int);
	iTotalLen += sizeof(int);

	Message *pMsgBody = (Message *)vMessage.msgbody();
	MY_ASSERT(pMsgBody != NULL, return 3);

	if( vMessage.msghead().messageid() == ID_C2S_REQUEST_STOREINFO ) 
	{
		char szBuffer[UNCOMPRESSIONSIZE];
		unsigned long iUncompressionSize = UNCOMPRESSIONSIZE;

		bRet = pMsgBody->SerializeToArray(szBuffer, iUncompressionSize);
		if( !bRet ) 
		{
			return 6;
		}

		unsigned long uTempLen = iLeftLen;
		int iRet = compress((Bytef *)pBuffer, &uTempLen, (const Bytef *)szBuffer, pMsgBody->ByteSize());
		if( iRet != Z_OK ) 
		{
			return 7;
		}

		iTempLen = uTempLen;
	}
	else 
	{
		bRet = pMsgBody->SerializeToArray(pBuffer, iLeftLen);
		if( !bRet )
		{
			//LOG_ERROR("default", "server message: SerializeToArray message body failed");
			return 4;
		}

		// message body的长度赋值
		iTempLen = pMsgBody->ByteSize();
	}

	*(int *)(pBuffer - sizeof(int)) = iTempLen;

	pBuffer += iTempLen;
	iLeftLen -= iTempLen;
	iTotalLen += iTempLen;

	// 消息总长度赋值
	pBuffer = vBuffer;
	*(int *)pBuffer = iTotalLen;

	// 消息长度判断
	if( iLeftLen < 0 )
	{
		//LOG_ERROR("default", "server message: buffer len is too short");
		return 5;
	}

	// 消息体的总长度赋值
	vBufferLen = iTotalLen + sizeof(int);
	return 0;
}

google::protobuf::Message *CMessageDecoder::CreateMessage(int vMessageID)
{
	Message *pMessage = NULL;
	switch(vMessageID)
	{
		CREATE_NEW_MSG(ID_S2S_REQUEST_REGISTER_SERVER, CMessageRegisterServerRequest)
		CREATE_NEW_MSG(ID_S2S_RESPONSE_REGISTER_SERVER, CMessageRegisterServerResponse)
		CREATE_NEW_MSG(ID_C2S_REQUEST_LOGINLOGIC, CMessageLoginLogicRequest)
		CREATE_NEW_MSG(ID_S2S_REQUEST_EXECUTESQL, CMessageExecuteSqlRequest)
		CREATE_NEW_MSG(ID_S2S_RESPONSE_EXECUTESQL, CMessageExecuteSqlResponse)
		CREATE_NEW_MSG(ID_S2S_REQUEST_STOREERROR, CMessageStoreInfoRequest)
		CREATE_NEW_MSG(ID_C2S_REQUEST_PUSH, CMessagePushRequest)
		CREATE_NEW_MSG(ID_S2S_INFO_UPDATESETTING, CMessageUpdateSettingInfo)
		CREATE_NEW_MSG(ID_S2S_REQUEST_PUSHINFO, CMessagePushInfoRequest)
		CREATE_NEW_MSG(ID_S2S_RESPONSE_PUSHINFO, CMessagePushInfoResponse)
		CREATE_NEW_MSG(ID_C2S_REQUEST_STOREINFO, CMessageStoreInfoRequest)
		CREATE_NEW_MSG(ID_C2S_REQUEST_TEST, CMessageTestRequest)
		CREATE_NEW_MSG(ID_S2S_REQUEST_CLIENTTEST, CMessageClientTestRequest)
		CREATE_NEW_MSG(ID_S2S_RESPONSE_CLIENTTEST, CMessageClientTestResponse)
		CREATE_NEW_MSG(ID_C2S_REQUEST_PUSHRESULT, CMessagePushResultRequest)
		CREATE_NEW_MSG(ID_S2S_REQUEST_CLIENTPUSHRESULT, CMessageClientPushResultRequest)
		CREATE_NEW_MSG(ID_S2S_RESPONSE_CLIENTPUSHRESULT, CMessageClientPushResultResponse)
		CREATE_NEW_MSG(ID_C2S_REQUEST_UPGRADEDERROR, CMessageUpgradedErrorRequest)

		default:
		{
			LOG_ERROR("default", "invalid msg id %d", vMessageID);
			break;
		}
	}

	return pMessage;
}
