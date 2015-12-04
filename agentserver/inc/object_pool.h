//--------------------------------------------------
// 本文件目的在于创建了一个实体池
//-------------------------------------------------- 
#ifndef _OBJECT_POOL_H_
#define _OBJECT_POOL_H_

#include "singleton.h"
#include "entity.h"
#include "free_list.h"

enum EObjectType
{
	OBJECT_TYPE_UNKOWN = 1,		// 未知类型
	OBJECT_TYPE_PLAYER,			// 玩家类型
	OBJECT_TYPE_SESSION,		// 事件类型
};

class CObjectPool: public CSingleton<CObjectPool>
{
	public:
		void *operator new(size_t vSize);
		void operator delete(void *pMem);
		CObjectPool();
		~CObjectPool();
		// 创建实体
		void *CreateObject(EObjectType vType);
		// 通过ID销毁实体
		void DestroyObject(unsigned long long vObjectID);
		// 通过ID获取实体
		void *GetObject(unsigned long long vObjectID);
		// 初始化
		void Initialize();
		// 获取相应的类型中还没有被使用的数量
		size_t GetFreeSize(EObjectType vType);
		// 获取相应的类型中总共有的数量
		size_t GetTotalSize(EObjectType vType);
		// 获取该类占用内存大小
		static size_t GetMemorySize();
	private:
		// 玩家实体池
		my::free_list<CPlayer, MAX_PLAYER_NUM> mPlayerPool;
		// 事件实体池
		my::free_list<CSession, MAX_PLAYER_NUM> mSessionPool;
};

#endif
