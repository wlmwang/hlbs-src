#include <iostream>
#include <time.h>
#include "login_server.h"
#include "server_ctrl.h"
#include "object_pool.h"
#include "server_tool.h"
#include "core_type.h"
#include "login_config.h"
#include "google/protobuf/descriptor.h"
#include "clientmessage.hxx.pb.h"
#include "error_no.h"
#include "hash.h"
#include <string>
using namespace std;
template<> CLoginServer* CSingleton<CLoginServer>::mSingletonPtr = NULL;

void *CLoginServer::operator new(size_t vSize)
{
	return (void *)(CServerCtrl::GetSingletonPtr()->CreateSegment(vSize));
}

void CLoginServer::operator delete(void *pMem)
{

}

CLoginServer::CLoginServer()
{
	if( CServerCtrl::GetSingletonPtr()->mStatus == SERVER_STATUS_INIT ) 
	{
		Initialize();
	}

	CObjectPool *pPool = new CObjectPool;
	if( pPool == NULL ) 
	{
		LOG_ERROR("default", "new CObjectPool failed");
		exit(1);
	}
	
	mConnectCtrl.mRemoveAcceptFDProc = RemoveAcceptFD;
	mInMsgQueue.SetBuffer(mInBufferPtr, MSG_QUEUE_LEN);
	mOutMsgQueue.SetBuffer(mOutBufferPtr, MSG_QUEUE_LEN);
}

CLoginServer::~CLoginServer()
{

}

void CLoginServer::Initialize()
{
	mLastTicker = GetTickCount();
	mReconnectTimer = CTimer(RECONNECT_TIME);
	mRegisterTimer = CTimer(REGISTER_TIME);
	//--------------------------------------------------
	// mRegisterToGateFlag = false;
	// mPlayerContainer.initailize();
	// mFDContainer.initailize();	
	//-------------------------------------------------- 
}

void CLoginServer::PrepareToRun()
{
	// 监听端口
	mConnectCtrl.ListenToAddress(LISTEN_TO_ALL, 
			CLoginConfig::GetSingletonPtr()->mLoginIPAddress, 
			(short)CLoginConfig::GetSingletonPtr()->mLoginPort);

	//--------------------------------------------------
	// // 连接到gate
	// mConnectCtrl.ConnectToServer(CONNECT_TO_GATESERVER,
	// 		CLoginConfig::GetSingletonPtr()->mGateIPAddress,
	// 		CLoginConfig::GetSingletonPtr()->mGatePort, 1);
	//-------------------------------------------------- 

	//--------------------------------------------------
	// mConnectCtrl.ConnectToServer(CONNECT_TO_DBSERVER,
	// 		CLoginConfig::GetSingletonPtr()->mDBIPAddress,
	// 		CLoginConfig::GetSingletonPtr()->mDBPort, 0);
	//-------------------------------------------------- 

	//--------------------------------------------------
	// // 向gate注册
	// RegisterToGate();
	//-------------------------------------------------- 

	//--------------------------------------------------
	// // 向db注册
	// RegisterToDB();
	//-------------------------------------------------- 
}

void CLoginServer::Run()
{
	while(1)
	{
		// 检查服务器运行状态
		// 包括是否需要关闭服务器等等
		CheckServerStatus();

		// 检查服务器时间
		CheckTimer();

		// 检查客户端消息
		CheckClientMessage();
		
		// 接收并处理消息
		mConnectCtrl.ReceiveAndProcessMessage(DecodeMessage);
	}
}

size_t CLoginServer::GetMemorySize()
{
	return sizeof(CLoginServer) + CObjectPool::GetMemorySize();
}

// 检查服务器运行状态
void CLoginServer::CheckServerStatus()
{
	if( CServerCtrl::GetSingletonPtr()->mStatus == SERVER_STATUS_QUIT )
	{
		// 关机处理

		// 关闭服务器
		exit(0);
	}
}

// 检查服务器时间
void CLoginServer::CheckTimer()
{
	int iInterval = (int)(GetTickCount() - mLastTicker);

	// 传说中的服务器周期
	if( iInterval < 100 )
	{
		return;
	}

	// 加上间隔时间
	mLastTicker += iInterval;

	// 检测到gateserver的注册
	if( mRegisterTimer.CheckTimer(iInterval) )
	{
		//--------------------------------------------------
		// if( GetGateConnect() != NULL && GetGateConnect()->GetSocketFD() < 0 )
		// {
		// 	mRegisterToGateFlag = false;
		// }
		// else if( !mRegisterToGateFlag )
		// {
		// 	RegisterToGate();
		// }
		//-------------------------------------------------- 

		//--------------------------------------------------
		// if( GetDBConnect() != NULL && GetDBConnect()->GetSocketFD() < 0 )
		// {
		// 	mRegisterToDBFlag = false;
		// }
		// else if( !mRegisterToDBFlag )
		// {
		// 	RegisterToDB();
		// }
		//-------------------------------------------------- 
	}

	if( mReconnectTimer.CheckTimer(iInterval) )
	{
		// 检查连接状态，并试图重连
		mConnectCtrl.CheckConnectStatus();
	}

	//--------------------------------------------------
	// SESSION_CONTAINER::iterator it = mSessionContainer.begin();
	// std::vector<unsigned long long> stVec;
	// for(  ; it != mSessionContainer.end() ; ++it)
	// {
	// 	CSession *pSession = it->second;
	// 	if( pSession == NULL )
	// 	{
	// 		LOG_ERROR("default", "sql timeout check, session %llu not found", it->first );
	// 		stVec.push_back(it->first);
	// 		continue;
	// 	}
	//-------------------------------------------------- 

	//--------------------------------------------------
	// 	if( pSession->mTimer.CheckTimer(iInterval) )
	// 	{
	// 		LOG_ERROR("default", "session %llu time out", *it);
	// 		CObjectPool::GetSingletonPtr()->DestroyObject(it->first);
	// 		stVec.push_back(it->first);
	// 		continue;
	// 	}
	// }
	//-------------------------------------------------- 

	//--------------------------------------------------
	// for( std::vector<unsigned long long>::iterator stVecIter = stVec.begin() ; stVecIter != stVec.end() ; stVecIter++ )
	// {
	// 	mSessionContainer.erase(*stVecIter);
	// }
	//-------------------------------------------------- 
}

// 检查客户端消息
void CLoginServer::CheckClientMessage()
{
	static char szBuff[MAX_PACKAGE_LEN] = {0};

	while(1)
	{
		int iRet = mInMsgQueue.GetOneMsg(szBuff, MAX_PACKAGE_LEN);
		// 没有新消息
		if( iRet == 0 ) 
		{
			return;
		}

		// 如果出错
		if( iRet < 0 ) 
		{
			LOG_ERROR("default", "get one message from msg queue failed: %d", iRet);
			return;
		}

		// 如果长度不正常
		if( iRet < MIN_CLIENT_MSG_LEN || iRet > MAX_CLIENT_MSG_BUFFER_LEN ) 
		{
			LOG_ERROR("default", "get a msg from msg queue with invalid len: %d", iRet);
			return;
		}

		// 解析客户端消息
		DecodeClientMessage((char *)&szBuff[sizeof(int)], iRet - sizeof(int));
	}
}

// 处理消息
int CLoginServer::DecodeMessage(char *vBuffer, int vBufferLen, ServerConnect &vTCPClient, int vIsFromClient, int vAcceptPos)
{
	//--------------------------------------------------
	// if( vIsFromClient )
	// {
	// 	return CLoginServer::GetSingletonPtr()->DecodeClientMessage(vBuffer, vBufferLen, vTCPClient);
	// }
	// else
	// {
	//-------------------------------------------------- 
	return CLoginServer::GetSingletonPtr()->DecodeServerMessage(vBuffer, vBufferLen, vTCPClient, vAcceptPos);
	//--------------------------------------------------
	// }
	//-------------------------------------------------- 
}

// 处理客户端来的消息
int CLoginServer::DecodeClientMessage(char *vBuffer, int vBufferLen)
{
	MY_ASSERT(vBuffer != NULL && vBufferLen >= sizeof(short), return -1);

	char *pBuffer = vBuffer;
	//pBuffer += sizeof(int);

	int iLen = (int)*(short *)pBuffer;
	pBuffer += sizeof(short);

	PBNetHead stNetHead;
	if( stNetHead.ParseFromArray(pBuffer, iLen) == false )
	{
		LOG_ERROR("default", "PBNetHead ParseFromArray failed");
		return 0;
	}

	//--------------------------------------------------
	// // 如果是注册成功的返回消息
	// if( stNetHead.clientstate() > 0 )
	// {
	// 	LOG_INFO("default", "register to gateserver succeed");
	// 	mRegisterToGateFlag = true;
	// 	return 0;
	// }
	// else if( stNetHead.clientstate() < 0 )
	//-------------------------------------------------- 
	if( stNetHead.clientstate() < 0 )
	{
		LOG_INFO("default", "client fd(%d) close connect now", stNetHead.clientfd(0));

		// TODO: 玩家离线处理

		return 0;
	}

	pBuffer += stNetHead.ByteSize();

	CMessage stMessage;
	CCSHead stCSHead;
	int iRet = GetClientMsgFromBuffer(pBuffer, vBufferLen - sizeof(short) - stNetHead.ByteSize(), stMessage, stCSHead);
	if( iRet != 0 )
	{
		DisconnectClient(stNetHead.clientfd(0));
		return iRet;
	}

	// TODO: 令牌等其他附加信息的判断
	
#ifdef _DEBUG_
	Message* pUnknownMessageBody = (Message *)stMessage.msgbody();
	MY_ASSERT(pUnknownMessageBody != NULL, return -2);
	const ::google::protobuf::Descriptor* pDescriptor = pUnknownMessageBody->GetDescriptor();

	LOG_DEBUG("default", "receive client fd(%d) name[%s] message head[%s] message body[%s]", stNetHead.clientfd(0), pDescriptor->name().c_str(), stMessage.msghead().ShortDebugString().c_str(), pUnknownMessageBody->ShortDebugString().c_str());
#endif

	// 处理消息
	ProcessClientMessage(stMessage, stNetHead.clientfd(0));

	// 析构消息体
	((Message *)(stMessage.msgbody()))->~Message();

	return 0;
}

// 真正的客户端消息处理函数
int CLoginServer::ProcessClientMessage(CMessage &vMessage, int vClientFD)
{
	switch(vMessage.msghead().messageid())
	{
		// 客户端登录Login请求跳转
		case ID_C2S_REQUEST_LOGINLOGIC:
		{
			OnMessageLoginLogicRequest(vMessage, vClientFD);
			break;
		}
		default:
		{
			LOG_DEBUG("default", "client fd(%d) send a invalid msg id(%u), close it", vClientFD, vMessage.msghead().messageid());
			DisconnectClient(vClientFD);
			break;
		}
	}
	return 0;
}

// 真正的服务器消息处理函数
int CLoginServer::ProcessServerMessage(CMessage &vMessage, ServerConnect &vTCPClient, int vAcceptPos)
{
	switch ( vMessage.msghead().messageid() )
	{
		// 服务器注册回复
		case ID_S2S_RESPONSE_REGISTER_SERVER:
		{
			OnMessageRegisterServerResponse(vMessage, vTCPClient, vAcceptPos);
			break;
		}
		// 服务器注册消息
		case ID_S2S_REQUEST_REGISTER_SERVER:
		{
			OnMessageRegisterServerRequest(vMessage, vTCPClient, vAcceptPos);
			break;
		}
		// 数据库服务器的返回消息
		//--------------------------------------------------
		// case ID_S2S_RESPONSE_EXECUTESQL:
		// {
		// 	OnMessageExecuteSqlResponse(vMessage, vTCPClient, vAcceptPos);
		// 	break;
		// }
		//-------------------------------------------------- 
		default :
			break;
	}
	return 0;
}

// 处理服务器来的消息
int CLoginServer::DecodeServerMessage(char *vBuffer, int vBufferLen, ServerConnect &vTCPClient, int vAcceptPos)
{
	MY_ASSERT(vBuffer != NULL && vBufferLen >= sizeof(int) + sizeof(short) + sizeof(int), return -1);

	char *pBuffer = vBuffer;

	CMessage stMessage;
	int iRet = GetServerMsgFromBuffer(pBuffer, vBufferLen, stMessage);
	if( iRet != 0 )
	{
		return iRet;
	}

#ifdef _DEBUG_
	Message* pUnknownMessageBody = (Message *)stMessage.msgbody();
	MY_ASSERT(pUnknownMessageBody != NULL, return -2);
	const ::google::protobuf::Descriptor* pDescriptor= pUnknownMessageBody->GetDescriptor();

	LOG_DEBUG("default", "receive server fd(%d) message name[%s] message head[%s] message body[%s]", vTCPClient.GetSocketFD(), pDescriptor->name().c_str(), stMessage.msghead().ShortDebugString().c_str(), pUnknownMessageBody->ShortDebugString().c_str());
#endif

	// 处理消息
	ProcessServerMessage(stMessage, vTCPClient, vAcceptPos);

	// 析构消息体
	((Message *)(stMessage.msgbody()))->~Message();

	return 0;
}

// 服务器注册
void CLoginServer::OnMessageRegisterServerRequest(CMessage &vMessage, ServerConnect &vTCPClient, int vAcceptPos)
{
	CMessageRegisterServerRequest *pRequest = (CMessageRegisterServerRequest *)vMessage.msgbody();
	MY_ASSERT(pRequest != NULL, return);

	if( (SERVER_FE)vMessage.msghead().srcfe() != SERVER_SCENE )
	{
		LOG_INFO("default", "server fd(%d) receive an unknown type register msg, close it", vTCPClient.GetSocketFD());
		vTCPClient.Close();
		RemoveAcceptFD(vAcceptPos);
		return;
	}

	if( (SERVER_FE)vMessage.msghead().srcid() > MAX_SCENE_NUM || 
			(SERVER_FE)vMessage.msghead().srcid() == 0 )
	{
		LOG_INFO("default", "server fd(%d) receive an unknown id register msg, close it", vTCPClient.GetSocketFD());
		vTCPClient.Close();
		RemoveAcceptFD(vAcceptPos);
		return;
	}

	mAcceptPosArray[vMessage.msghead().srcid() - 1] = vAcceptPos;

	// 存储对外IP端口, 以便在跳转服务器时使用
	memcpy(CLoginConfig::GetSingletonPtr()->mClientIPAddress[vMessage.msghead().srcid() - 1], pRequest->ipaddress().c_str(), pRequest->ipaddress().length());
	CLoginConfig::GetSingletonPtr()->mClientPort[vMessage.msghead().srcid() - 1] = pRequest->port();

	static CMessage stMessage;
	static CMessageRegisterServerResponse stResponse;
	stResponse.Clear();
	stMessage.Clear();
	stMessage.mutable_msghead()->set_messageid(ID_S2S_RESPONSE_REGISTER_SERVER);
	stMessage.set_msgbody((unsigned long long)&stResponse);

	Send2Scene(vMessage.msghead().srcid(), stMessage);
	LOG_DEBUG("default", "send Login response to scene %u", vMessage.msghead().srcid());
}

// 使网关服务器断开与客户端的连接
void CLoginServer::DisconnectClient(int vSocketFD)
{
	if( vSocketFD < 0 )
	{
		return;
	}

	PBNetHead stNetHead;
	stNetHead.add_clientfd(vSocketFD);
	stNetHead.set_clientstate(-1);
	char szBuff[DISCONNECT_MSG_LEN];
	char *pBuffer = szBuff;
	*(int *)pBuffer = stNetHead.ByteSize() + sizeof(short);
	pBuffer += sizeof(int);
	*(unsigned short *)pBuffer = stNetHead.ByteSize();
	pBuffer += sizeof(short);
	stNetHead.SerializeToArray((char *)pBuffer, DISCONNECT_MSG_LEN - sizeof(short) - sizeof(int));

	mOutMsgQueue.PutOneMsg((char *)&szBuff, sizeof(int) + sizeof(short) + stNetHead.ByteSize());
	LOG_INFO("default", "server disconnect client fd(%d)", vSocketFD);

	//--------------------------------------------------
	// if( GetGateConnect() != NULL )
	// {
	// 	GetGateConnect()->SendOneMsg((char *)&szBuff, sizeof(int) + sizeof(short) + stNetHead.ByteSize());
	// 	LOG_INFO("default", "server disconnect client fd(%d)", vSocketFD);
	// }
	//-------------------------------------------------- 
}

// 向逻辑服务器注册
//--------------------------------------------------
// void CLoginServer::RegisterToDB()
// {
// 	if( mRegisterToDBFlag )
// 	{
// 		return;
// 	}
// 
// 	if( GetDBConnect() != NULL && GetDBConnect()->GetSocketFD() < 0 )
// 	{
// 		return;
// 	}
// 
// 	static CMessage stMessage;
// 	static CMessageRegisterServerRequest stRequest;
// 	stMessage.Clear();
// 	stRequest.Clear();
// 	stMessage.mutable_msghead()->set_messageid(ID_S2S_REQUEST_REGISTER_SERVER);
// 	stMessage.set_msgbody((unsigned long long)&stRequest);
// 
// 	bool bRet = Send2DB(stMessage);
// 	if( !bRet )
// 	{
// 		return;
// 	}
// 
// 	LOG_DEBUG("default", "send register message to db");
// }
//-------------------------------------------------- 

//--------------------------------------------------
// // 注册到网关服务器
// void CLoginServer::RegisterToGate()
// {
// 	if( mRegisterToGateFlag )
// 	{
// 		return;
// 	}
// 
// 	if( GetGateConnect() != NULL && GetGateConnect()->GetSocketFD() < 0 )
// 	{
// 		return;
// 	}
// 
// 	PBNetHead stNetHead;
// 	stNetHead.add_clientfd((unsigned int)PB_TYPE_STREAM);
// 	stNetHead.set_clientstate(1);
// 
// 	char szBuff[DISCONNECT_MSG_LEN];
// 	char *pBuffer = szBuff;
// 	*(int *)pBuffer = stNetHead.ByteSize() + sizeof(short);
// 	pBuffer += sizeof(int);
// 	*(unsigned short *)pBuffer = stNetHead.ByteSize();
// 	pBuffer += sizeof(short);
// 	stNetHead.SerializeToArray(pBuffer, DISCONNECT_MSG_LEN - sizeof(int) - sizeof(short));
// 
// 	if( GetGateConnect() != NULL )
// 	{
// 		GetGateConnect()->SendOneMsg((char *)&szBuff, stNetHead.ByteSize() + sizeof(int) + sizeof(short));
// 		LOG_DEBUG("default", "send register message to gate");
// 	}
// }
//-------------------------------------------------- 

// 向网关服务器发送消息
bool CLoginServer::Send2Gate(char *vBuffer, int vBufferLen)
{
	return mOutMsgQueue.PutOneMsg(vBuffer, vBufferLen) == 0;
	//--------------------------------------------------
	// if( GetGateConnect() == NULL )
	// {
	// 	return false;
	// }
	// return GetGateConnect()->SendOneMsg(vBuffer, vBufferLen);
	//-------------------------------------------------- 
}

// 向场景服务器发送消息
bool CLoginServer::Send2Scene(int vNum, CMessage &vMessage)
{
	vMessage.mutable_msghead()->set_dstfe(SERVER_SCENE);
	vMessage.mutable_msghead()->set_dstid(vNum);
	vMessage.mutable_msghead()->set_srcfe(SERVER_LOGIN);
	vMessage.mutable_msghead()->set_srcid(1);

	static char szBuffer[MAX_CLIENT_MSG_BUFFER_LEN];
	int iLen = MAX_CLIENT_MSG_BUFFER_LEN;
	int iRet = PutServerMsgIntoBuffer((char *)&szBuffer, iLen, vMessage);
	if( iRet != 0 )
	{
		return false;
	}
	if( GetSceneConnect(vNum) == NULL )
	{
		return false;
	}
	bool bRet = GetSceneConnect(vNum)->SendOneMsg((char *)&szBuffer, iLen);
	if( !bRet )
	{
		return false;
	}

#ifdef _DEBUG_
	Message* pUnknownMessageBody = (Message *)vMessage.msgbody();
	MY_ASSERT(pUnknownMessageBody != NULL, return true);
	const ::google::protobuf::Descriptor* pDescriptor= pUnknownMessageBody->GetDescriptor();

	LOG_DEBUG("default", "Send2Scene(%d) name[%s] message head[%s] message body[%s]", vNum, pDescriptor->name().c_str(), vMessage.msghead().ShortDebugString().c_str(), pUnknownMessageBody->ShortDebugString().c_str());
#endif

	return true;
}

// 向DB服务器发送消息
//--------------------------------------------------
// bool CLoginServer::Send2DB(CMessage &vMessage)
// {
// 	vMessage.mutable_msghead()->set_dstfe(SERVER_DB);
// 	vMessage.mutable_msghead()->set_dstid(1);
// 	vMessage.mutable_msghead()->set_srcfe(SERVER_LOGIN);
// 	vMessage.mutable_msghead()->set_srcid(1);
// 
// 	static char szBuffer[MAX_CLIENT_MSG_BUFFER_LEN];
// 	int iLen = MAX_CLIENT_MSG_BUFFER_LEN;
// 	int iRet = PutServerMsgIntoBuffer((char *)&szBuffer, iLen, vMessage);
// 	if( iRet != 0 )
// 	{
// 		return false;
// 	}
// 	if( GetDBConnect() == NULL )
// 	{
// 		return false;
// 	}
// 	bool bRet = GetDBConnect()->SendOneMsg((char *)&szBuffer, iLen);
// 	if( !bRet )
// 	{
// 		return false;
// 	}
// 
// #ifdef _DEBUG_
// 	Message* pUnknownMessageBody = (Message *)vMessage.msgbody();
// 	MY_ASSERT(pUnknownMessageBody != NULL, return true);
// 	const ::google::protobuf::Descriptor* pDescriptor= pUnknownMessageBody->GetDescriptor();
// 
// 	LOG_DEBUG("default", "Send2DB name[%s] message head[%s] message body[%s]", pDescriptor->name().c_str(), vMessage.msghead().ShortDebugString().c_str(), pUnknownMessageBody->ShortDebugString().c_str());
// #endif
// 
// 	return true;
// }
//-------------------------------------------------- 

// 向客户端们发送消息
bool CLoginServer::Send2Clients(PBNetHead &vNetHead, CMessage &vMessage)
{
	CCSHead stCSHead;

	static char szBuffer[MAX_CLIENT_MSG_BUFFER_LEN];
	char *pBuffer = szBuffer;

	// 跳过总消息头
	pBuffer += sizeof(int);

	// 序列化PBNetHead
	*(unsigned short *)pBuffer = vNetHead.ByteSize();
	pBuffer += sizeof(short);
	vNetHead.SerializeToArray(pBuffer, MAX_CLIENT_MSG_BUFFER_LEN - sizeof(int) - sizeof(short));
	pBuffer += vNetHead.ByteSize();

	int iLen = MAX_CLIENT_MSG_BUFFER_LEN - sizeof(int) - sizeof(short) - vNetHead.ByteSize();

	// 序列化消息体
	int iRet = PutClientMsgIntoBuffer(pBuffer, iLen, vMessage, stCSHead);
	if( iRet != 0 )
	{
		return false;
	}
	
	int iTotalLen = iLen + sizeof(short) + vNetHead.ByteSize();
	pBuffer = szBuffer;

	// 总消息长度赋值
	*(int *)pBuffer = iTotalLen;

	bool bRet = Send2Gate(pBuffer, sizeof(int) + iTotalLen);

#ifdef _DEBUG_
	Message* pUnknownMessageBody = (Message *)vMessage.msgbody();
	MY_ASSERT(pUnknownMessageBody != NULL, return -2);
	const ::google::protobuf::Descriptor* pDescriptor= pUnknownMessageBody->GetDescriptor();

	LOG_DEBUG("default", "Send2Clients fd(%d) name[%s] message head[%s] message body[%s]", vNetHead.clientfd(0), pDescriptor->name().c_str(), vMessage.msghead().ShortDebugString().c_str(), pUnknownMessageBody->ShortDebugString().c_str());
#endif
}

// 向客户端发送消息
bool CLoginServer::Send2Client(int vClientFD, CMessage &vMessage)
{
	CCSHead stCSHead;
	PBNetHead stNetHead;
	stNetHead.set_clientstate(0);
	stNetHead.add_clientfd(vClientFD);

	static char szBuffer[MAX_CLIENT_MSG_BUFFER_LEN];
	char *pBuffer = szBuffer;

	// 跳过总消息头
	pBuffer += sizeof(int);

	// 序列化PBNetHead
	*(unsigned short *)pBuffer = stNetHead.ByteSize();
	pBuffer += sizeof(short);
	stNetHead.SerializeToArray(pBuffer, MAX_CLIENT_MSG_BUFFER_LEN - sizeof(int) - sizeof(short));
	pBuffer += stNetHead.ByteSize();

	int iLen = MAX_CLIENT_MSG_BUFFER_LEN - sizeof(int) - sizeof(short) - stNetHead.ByteSize();

	// 序列化消息体
	int iRet = PutClientMsgIntoBuffer(pBuffer, iLen, vMessage, stCSHead);
	if( iRet != 0 )
	{
		return false;
	}
	
	int iTotalLen = iLen + sizeof(short) + stNetHead.ByteSize();
	pBuffer = szBuffer;

	// 总消息长度赋值
	*(int *)pBuffer = iTotalLen;

	bool bRet = Send2Gate(pBuffer, sizeof(int) + iTotalLen);

#ifdef _DEBUG_
	Message* pUnknownMessageBody = (Message *)vMessage.msgbody();
	MY_ASSERT(pUnknownMessageBody != NULL, return -2);
	const ::google::protobuf::Descriptor* pDescriptor= pUnknownMessageBody->GetDescriptor();

	LOG_DEBUG("default", "Send2Client fd(%d) name[%s] message head[%s] message body[%s]", vClientFD, pDescriptor->name().c_str(), vMessage.msghead().ShortDebugString().c_str(), pUnknownMessageBody->ShortDebugString().c_str());
#endif
}

// 服务器注册回复
void CLoginServer::OnMessageRegisterServerResponse(CMessage &vMessage, ServerConnect &vTCPClient, int vAcceptPos)
{
	CMessageRegisterServerResponse *pResponse = (CMessageRegisterServerResponse *)vMessage.msgbody();
	MY_ASSERT(pResponse != NULL, return);

	//LOG_INFO("default", "register to db server succeed");
	//mRegisterToDBFlag = true;
	//ExecuteSql(SESSION_FOR_TEST, 0, 0, 1, SELECT, 1, "select * from test");
}

// 通过FD找到玩家
//--------------------------------------------------
// CPlayer *CLoginServer::GetPlayerFromFD(int vClientFD)
// {
// 	PLAYERFD_CONTAINER::iterator it = mFDContainer.find(vClientFD);
// 	if( it == mFDContainer.end() )
// 	{   
// 		return NULL;
// 	}   
// 
// 	return (CPlayer *)CObjectPool::GetSingletonPtr()->GetObject(it->second);
// }
//-------------------------------------------------- 

// 通过EntityID找到玩家
//--------------------------------------------------
// CPlayer *CLoginServer::GetPlayerFromEntityID(unsigned long long vEntityID)
// {
// 	PLAYER_CONTAINER::iterator it = mPlayerContainer.find(vEntityID);
// 	if( it == mPlayerContainer.end() )
// 	{
// 		return NULL;
// 	}
// 
// 	return (CPlayer *)CObjectPool::GetSingletonPtr()->GetObject(it->second);
// }
//-------------------------------------------------- 

// 一个accept连接被断连的回调函数
void CLoginServer::RemoveAcceptFD(int vPos)
{
	if( vPos < 0 )
	{
		return;
	}

	for( int i = 0 ; i < MAX_ACCEPT_NUM ; i++ )
	{
		if( vPos == CLoginServer::GetSingletonPtr()->mAcceptPosArray[i] )
		{
			CLoginServer::GetSingletonPtr()->mAcceptPosArray[i] = -1;
			break;
		}
	}
}

// 发送消息到DB执行SQL
//--------------------------------------------------
// void CLoginServer::ExecuteSql(ESessionType vSessionType, int vParam1, int vParam2, int vCallback, SQLTYPE vSqlType, int vObjectID, const char *pSql, ... )
// {
// 	va_list args;
// 	char szSql[MAX_SQL_LEN] = {0};
// 
// 	va_start(args, pSql);
// 	int len = vsnprintf(szSql, MAX_SQL_LEN, pSql, args);
// 	va_end(args);
// 
// 	unsigned long long nSessionID = 0;	
// 	if( vCallback )
// 	{
// 		CSession *pSession = (CSession *)CObjectPool::GetSingletonPtr()->CreateObject(OBJECT_TYPE_SESSION);
// 		MY_ASSERT(pSession != NULL, return);
// 		nSessionID = pSession->GetObjectID();
// 		pSession->mParam1 = vParam1;
// 		pSession->mParam2 = vParam2;
// 		pSession->mSessionType = vSessionType;
// 		mSessionContainer.insert(std::make_pair(nSessionID, pSession));
// 	}
// 
// 	static CMessageExecuteSqlRequest stRequest;
// 	static CMessage stMessage;
// 	stMessage.Clear();
// 	stRequest.Clear();
// 	stRequest.set_sessionid(nSessionID);
// 	stRequest.set_callback(vCallback);
// 	stRequest.set_sqltype(vSqlType);
// 	stRequest.set_sql(szSql, strlen(szSql));
// 	stMessage.mutable_msghead()->set_option(vObjectID % 8);
// 	stMessage.mutable_msghead()->set_messageid(ID_S2S_REQUEST_EXECUTESQL);
// 	stMessage.set_msgbody((unsigned long long)&stRequest);
// 	Send2DB(stMessage);
// }
//-------------------------------------------------- 

// 数据库服务器的回复消息
//--------------------------------------------------
// void CLoginServer::OnMessageExecuteSqlResponse(CMessage &vMessage, ServerConnect &vTCPClient, int vAcceptPos)
// {
// 	CMessageExecuteSqlResponse *pResponse = (CMessageExecuteSqlResponse *)vMessage.msgbody();
// 	MY_ASSERT(pResponse != NULL, return);
// 	
// 	unsigned long long nSessionID = pResponse->sessionid();			
// 	CSession *pSession = (CSession *)CObjectPool::GetSingletonPtr()->GetObject(nSessionID);
// 	MY_ASSERT(pSession != NULL, return);
// 
// 	switch(pSession->mSessionType)
// 	{
// 		case SESSION_FOR_NONE:
// 			// do nothing
// 			break;
// 		case SESSION_FOR_TEST:
// 			OnSessionForTest(pSession, pResponse);
// 			break;
// 	}
// }
//-------------------------------------------------- 

// 数据库测试专用
//--------------------------------------------------
// void CLoginServer::OnSessionForTest(CSession *vSession, CMessageExecuteSqlResponse *vResponse)
// {
// 	if( vResponse->resultcode())
// 	{
// 		LOG_DEBUG("default", "execute test sql successful: %llu", vSession->GetObjectID());
// 	}
// 	else
// 	{
// 		LOG_DEBUG("default", "execute test sql failed: %llu", vSession->GetObjectID());
// 	}
// 	
// 	mSessionContainer.erase(vSession->GetObjectID());
// 	CObjectPool::GetSingletonPtr()->DestroyObject(vSession->GetObjectID());
// }
//-------------------------------------------------- 

// 客户端登陆消息
int CLoginServer::OnMessageLoginLogicRequest(CMessage &vMessage, int vClientFD)
{
	CMessageLoginLogicRequest *pRequest = (CMessageLoginLogicRequest *)vMessage.msgbody();
	if( pRequest == NULL ) 
	{
		DisconnectClient(vClientFD);
		return -1;
	}
	unsigned int uServerID;
	string s = pRequest->platform().c_str();
	static const size_t npos = -1;
	size_t position = s.find("kk");
	if (position != string::npos){
		LOG_INFO("default", "receive kk flavor name[%s]", pRequest->platform().c_str());
//		LOG_INFO("default", "Found kk 000000000000000000000000" );
		uServerID = 4 % MAX_SCENE_NUM;
		LOG_INFO("default", "receive kk uServerID name[%d]", uServerID);
	}
	else
	{
//		LOG_INFO("default", "Not found kk 1111111111111111111111111");
//		LOG_INFO("default", "receive flavor name[%s]", pRequest->platform().c_str());
		uServerID = hash_string(pRequest->platform().c_str()) % MAX_SCENE_NUM;
	
//		LOG_INFO("default", "receive  uServerID name[%d]", uServerID);
	}
	
	if( CLoginConfig::GetSingletonPtr()->mClientPort[uServerID] == 0 )
	{
		DisconnectClient(vClientFD);
		return -1;
	}
	
	static CMessage stMessage;
	static CMessageLoginLogicResponse stResponse;
	stMessage.Clear();
	stResponse.Clear();
	stResponse.set_ipaddress(CLoginConfig::GetSingletonPtr()->mClientIPAddress[uServerID]);
	stResponse.set_port(CLoginConfig::GetSingletonPtr()->mClientPort[uServerID]);
	stMessage.mutable_msghead()->set_messageid(ID_S2C_RESPONSE_LOGINLOGIC);
	stMessage.set_msgbody((unsigned long long)&stResponse);
	Send2Client(vClientFD, stMessage);

	DisconnectClient(vClientFD);
	return 0;
}
