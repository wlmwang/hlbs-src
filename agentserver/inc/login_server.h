#ifndef _STREAM_SERVER_H_
#define _STREAM_SERVER_H_
 
#include "timer.h"
#include "singleton.h"
#include "tcp_ctrl.h"
#include "core_type.h"
#include "message_decoder.h"
#include "my_hashmap.h"
#include "object_pool.h"
#include "my_string.h"
#include "msg_queue.h"
#include "net_head_pb.hxx.pb.h"
#include "servermessage.hxx.pb.h"

class CLoginServer: public CSingleton<CLoginServer>, public CMessageDecoder
{
	public:
		typedef CTCPClient<MAX_TCP_BUFFER_LEN, MAX_TCP_BUFFER_LEN> ServerConnect;
		void *operator new(size_t vSize);
		void operator delete(void *pMem);
		CLoginServer();
		~CLoginServer();
		void Initialize();
		void PrepareToRun(); 
		void Run(); 
		static size_t GetMemorySize();		
		static char *mInBufferPtr;
		static char *mOutBufferPtr;

	private:
		// 检查服务器运行状态
		void CheckServerStatus();
		// 检查服务器时间
		void CheckTimer();
		// 处理消息
		static int DecodeMessage(char *vBuffer, int vBufferLen, ServerConnect &vTCPClient, int vIsFromClient, int vAcceptPos);
		// 检查客户端消息
		void CheckClientMessage();
		// 处理客户端来的消息
		int DecodeClientMessage(char *vBuffer, int vBufferLen);
		// 处理服务器来的消息
		int DecodeServerMessage(char *vBuffer, int vBufferLen, ServerConnect &vTCPClient, int vAcceptPos);
		// 使网关服务器断开与客户端的连接
		void DisconnectClient(int vSocketFD);
		//--------------------------------------------------
		// // 向网关服务器注册
		// void RegisterToGate();
		// // 向数据库服务器注册
		// void RegisterToDB();
		//inline ServerConnect *GetDBConnect() { return mConnectCtrl.GetClientConnect(CONNECT_TO_DBSERVER); }
		// inline ServerConnect *GetGateConnect() { return mConnectCtrl.GetClientConnect(CONNECT_TO_GATESERVER); }
		//-------------------------------------------------- 
		inline ServerConnect *GetSceneConnect(int num) { return mConnectCtrl.GetAcceptConnect(mAcceptPosArray[num - 1]); }
		// 向网关服务器发送消息
		bool Send2Gate(char *vBuffer, int vBufferLen);
		// 向场景服务器发送消息
		bool Send2Scene(int vNum, CMessage &vMessage);
		// 向DB服务器发送消息
		//bool Send2DB(CMessage &vMessage);
		// 向客户端发送消息
		bool Send2Client(int vClientFD, CMessage &vMessage);
		// 向客户端们发送消息
		bool Send2Clients(PBNetHead &vNetHead, CMessage &vMessage);
		// 一个accept连接被断连的回调函数
		static void RemoveAcceptFD(int vPos);
		// 真正的客户端消息处理函数
		int ProcessClientMessage(CMessage &vMessage, int vClientFD);
		// 真正的服务器消息处理函数
		int ProcessServerMessage(CMessage &vMessage, ServerConnect &vTCPClient, int vAcceptPos);

		// 服务器注册回复
		void OnMessageRegisterServerResponse(CMessage &vMessage, ServerConnect &vTCPClient, int vAcceptPos);
		// 数据库服务器的回复消息
		void OnMessageExecuteSqlResponse(CMessage &vMessage, ServerConnect &vTCPClient, int vAcceptPos);
		// 数据库测试专用
		//--------------------------------------------------
		// void OnSessionForTest(CSession *vSession, CMessageExecuteSqlResponse *vResponse);
		//-------------------------------------------------- 
		// 客户端登陆消息
		int OnMessageLoginLogicRequest(CMessage &vMessage, int vClientFD);

		// 通过FD找到玩家
		//--------------------------------------------------
		// CPlayer *GetPlayerFromFD(int vClientFD);
		//-------------------------------------------------- 
		// 通过EntityID找到玩家
		//--------------------------------------------------
		// CPlayer *GetPlayerFromEntityID(unsigned long long vEntityID);
		//-------------------------------------------------- 
		// 服务器注册
		void OnMessageRegisterServerRequest(CMessage &vMessage, ServerConnect &vTCPClient, int vAcceptPos);
		// 发送消息到DB执行SQL
		//void ExecuteSql(ESessionType vSessionType, int vParam1, int vParam2, int vCallback, SQLTYPE vSqlType, int vObjectID, const char *pSql, ... );
		
	private:
		enum StreamListenConnect
		{
			LISTEN_TO_ALL = 0,
		};

		//--------------------------------------------------
		// enum StreamClientConnect
		// {
		// 	//CONNECT_TO_DBSERVER = 0,
		// 	CONNECT_TO_GATESERVER,
		// };
		//-------------------------------------------------- 

		enum StreamAcceptConnect
		{
			MAX_ACCEPT_NUM = MAX_SCENE_NUM,
		};
		// 有两个主动连接，一个连向gate，一个连向db
		CTCPCtrl<MAX_TCP_BUFFER_LEN, MAX_TCP_BUFFER_LEN, 1, 0, MAX_SCENE_NUM * 2> mConnectCtrl;
		int mAcceptPosArray[MAX_ACCEPT_NUM];

		unsigned long long mLastTicker;
		CTimer mReconnectTimer;
		CTimer mRegisterTimer;
		CMsgQueue mInMsgQueue;
		CMsgQueue mOutMsgQueue;
		//--------------------------------------------------
		// bool mRegisterToGateFlag;
		// bool mRegisterToDBFlag;
		//-------------------------------------------------- 

		//--------------------------------------------------
		// // 一个对应EntityID的玩家容器
		// typedef my::hash_map<unsigned long long, unsigned long long, MAX_PLAYER_NUM> PLAYER_CONTAINER;
		// PLAYER_CONTAINER mPlayerContainer;
		//-------------------------------------------------- 

		//--------------------------------------------------
		// // 一个对应玩家FD的容器
		// typedef my::hash_map<unsigned int, unsigned long long, MAX_PLAYER_NUM> PLAYERFD_CONTAINER;
		// PLAYERFD_CONTAINER mFDContainer;
		//-------------------------------------------------- 

		//--------------------------------------------------
		// // 事件的存储容器
		// typedef my::hash_map<unsigned long long, CSession *, MAX_PLAYER_NUM> SESSION_CONTAINER;
		// SESSION_CONTAINER mSessionContainer;
		//-------------------------------------------------- 
};

#endif
