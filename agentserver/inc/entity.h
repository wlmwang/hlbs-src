#ifndef _ENTITY_H_
#define _ENTITY_H_

#include "object.h"
#include "core_type.h"
#include "timer.h"
#include <string.h>
class CObjectPool;

// 玩家实体
class CPlayer: public CObject
{
	public:
		CPlayer() {}
		~CPlayer() {}
		void Initialize() 
		{
			memset(mCharName, 0, sizeof(mCharName));
			mRoomID = 0;
			mClientFD = 0;
			mRoomPos = -1;
			mEntityID = 0;
			mPlayerType = -1;
			mSeatID = -1;
			mModelID = 0;
			mRoomIndex = -1;
		}
	
	public:
		unsigned long long mRoomID;										// 房间ID
		unsigned long long mEntityID;									// 实体ID
		//char mCharName[MAX_NAME_LEN+1];								// 角色名 这是原先客户端传来的名字
		unsigned int mClientFD;											// FD
		int mRoomPos;													// 房间中的位置
		int mPlayerType;												// 玩家类型 0 表演者 1 群众 2 观察者
		unsigned int mSeatID;											// 玩家座位ID
		unsigned int mModelID;											// 玩家模型ID
		char mCharName[MAX_NAME_LEN+1];									// 角色名 aaa~zzz
		int mRoomIndex;													// 角色在第几个房间		
};

enum ESessionType
{
	SESSION_FOR_NONE = 1,
	SESSION_FOR_TEST,
};

// 事件实体
class CSession: public CObject
{
	public:
		CSession() {}
		~CSession() {}
		void Initialize()
		{
			mParam1 = 0;
			mParam2 = 0;
			mSessionType = SESSION_FOR_NONE;
			mTimer = CTimer(EXECUTESQL_TIMEOUT);
		}
	public:
		unsigned long long mParam1;
		unsigned long long mParam2;
		CTimer mTimer;
		ESessionType mSessionType;
};

#endif
