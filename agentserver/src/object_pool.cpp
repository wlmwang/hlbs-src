#include "object_pool.h"
#include "server_ctrl.h"
#include "log.h"

template<> CObjectPool *CSingleton<CObjectPool>::mSingletonPtr = NULL;

void *CObjectPool::operator new(size_t vSize)
{
	return (void *)(CServerCtrl::GetSingletonPtr()->CreateSegment(vSize));
}

void CObjectPool::operator delete(void *pMem)
{

}

CObjectPool::CObjectPool()
{
	if( CServerCtrl::GetSingletonPtr()->mStatus == SERVER_STATUS_INIT )
	{
		Initialize();
	}
}

CObjectPool::~CObjectPool()
{

}

// 初始化
void CObjectPool::Initialize()
{
	// 各实体池初始化
	mPlayerPool.initialize();
	mSessionPool.initialize();

	unsigned long long nPlayerType = ((unsigned long long)OBJECT_TYPE_PLAYER) << 32;

	// 这次分配的是下次需要分配给该实体的ID
	for( int i = 0 ; i < MAX_PLAYER_NUM ; i++ )
	{
		mPlayerPool.get_base_node(i).get_node().mObjectID = nPlayerType + i;
	}

	unsigned long long nSessionType = ((unsigned long long)OBJECT_TYPE_SESSION) << 32;

	for( int i = 0 ; i < MAX_PLAYER_NUM ; i++ )
	{
		mSessionPool.get_base_node(i).get_node().mObjectID = nSessionType + i;
	}
}

// 创建实体
void *CObjectPool::CreateObject(EObjectType vType)
{
	CObject *pObject = NULL;
	switch(vType)
	{
		case OBJECT_TYPE_PLAYER:
			pObject = mPlayerPool.get_one_node();
			if( pObject != NULL )
			{
				pObject->Initialize();
			}
			break;
		case OBJECT_TYPE_SESSION:
			pObject = mSessionPool.get_one_node();
			if( pObject != NULL )
			{
				pObject->Initialize();
			}
			break;
		default:
			break;
	}
	if( pObject != NULL )
	{
		LOG_DEBUG("default", "create object type %d id %llu", vType, pObject->GetObjectID());
	}
	else
	{
		LOG_ERROR("default", "create object failed, type %d", vType);
	}
	return pObject;
}

// 通过ID销毁实体
void CObjectPool::DestroyObject(unsigned long long vObjectID)
{
	EObjectType nType = (EObjectType)(unsigned int)(vObjectID >> 32);
	unsigned int n32BitID = (unsigned int)(vObjectID & 0xFFFFFFFF);
	unsigned int nCapacity, nArrayID;
	unsigned long long nPlayerType, nSessionType;
	LOG_DEBUG("default", "Destroy object type %d id %llu", nType, vObjectID);
	switch(nType)
	{
		case OBJECT_TYPE_PLAYER:
			nCapacity = (unsigned int)mPlayerPool.capacity();
			nArrayID = n32BitID % nCapacity;
			// 每次把ObjectID加上capacity()的数量
			// 如果已经溢出则重新回到最小值
			n32BitID += nCapacity;
			if( n32BitID < nCapacity )
			{
				n32BitID = nArrayID;
			}
			nPlayerType = ((unsigned long long)OBJECT_TYPE_PLAYER) << 32;
			mPlayerPool.get_base_node(nArrayID).get_node().mObjectID = nPlayerType + n32BitID;
			mPlayerPool.get_base_node(nArrayID).get_node().Initialize();
			mPlayerPool.free_one_node(nArrayID);
			break;
		case OBJECT_TYPE_SESSION:
			nCapacity = (unsigned int)mSessionPool.capacity();
			nArrayID = n32BitID % nCapacity;
			n32BitID += nCapacity;
			if( n32BitID < nCapacity )
			{
				n32BitID = nArrayID;
			}
			nSessionType = ((unsigned long long)OBJECT_TYPE_SESSION) << 32;
			mSessionPool.get_base_node(nArrayID).get_node().mObjectID = nSessionType + n32BitID;
			mSessionPool.get_base_node(nArrayID).get_node().Initialize();
			mSessionPool.free_one_node(nArrayID);
			break;
		default:
			return;
	}
}

// 通过ID获取实体
void *CObjectPool::GetObject(unsigned long long vObjectID)
{
	unsigned int nType = (unsigned int)(vObjectID >> 32);
	unsigned int nArrayID = (unsigned int)(vObjectID & 0xFFFFFFFF);
	switch(nType)
	{
		case OBJECT_TYPE_PLAYER:
			return (void *)mPlayerPool.get_used_data(nArrayID%(unsigned int)mPlayerPool.capacity());
		case OBJECT_TYPE_SESSION:
			return (void *)mSessionPool.get_used_data(nArrayID%(unsigned int)mSessionPool.capacity());
		default:
			return (void *)NULL;
	}
}

// 获取相应的类型中还没有被使用的数量
size_t CObjectPool::GetFreeSize(EObjectType vType)
{
	switch(vType)
	{
		case OBJECT_TYPE_PLAYER:
			return mPlayerPool.get_free_size();
		case OBJECT_TYPE_SESSION:
			return mSessionPool.get_free_size();
		default:
			return (size_t)0;
	}
}

// 获取相应的类型中总共有的数量
size_t CObjectPool::GetTotalSize(EObjectType vType)
{
	switch(vType)
	{
		case OBJECT_TYPE_PLAYER:
			return mPlayerPool.capacity();
		case OBJECT_TYPE_SESSION:
			return mSessionPool.capacity();
		default:
			return (size_t)0;
	}
}

// 获取该类占用内存大小
size_t CObjectPool::GetMemorySize()
{
	return sizeof(CObjectPool);
}
