//--------------------------------------------------
// 本文件用于控制服务器的启动以及共享内存分配
//-------------------------------------------------- 
#ifndef _SERVER_CTRL_H_
#define _SERVER_CTRL_H_

#include <cstdlib>
#include "singleton.h"
#include "log.h"

enum SERVER_STATUS
{
	SERVER_STATUS_UNKNOWN = 0,		// 未知状态
	SERVER_STATUS_INIT,				// 服务器的初始化状态
	SERVER_STATUS_RESUME,			// 重新载入内存模式
	SERVER_STATUS_QUIT,				// 服务器进入关闭状态
	SERVER_STATUS_RUNNING			// 正常运行状态模式
};

// 这个类直接构造即可，每一次启动都不相同
class CServerCtrl: public CSingleton<CServerCtrl>
{
	public:
		SERVER_STATUS mStatus;				// 服务器当前状态
		static char *mShmPtr;				// 服务器需要使用的共享内存位置
		char *mUnallocatedShmPtr;			// 未分配的内存指针
		size_t mUnallocatedShmSize;			// 未分配的内存大小
		
		static size_t GetMemorySize()
		{
			return sizeof(CServerCtrl);
		}

		CServerCtrl(int vInitFlag, size_t vSize)
		{
			if( vInitFlag )
			{
				mStatus = SERVER_STATUS_INIT;
			}
			else
			{
				mStatus = SERVER_STATUS_RESUME;
			}

			mUnallocatedShmPtr = mShmPtr + sizeof(CServerCtrl);
			mUnallocatedShmSize = vSize - sizeof(CServerCtrl);
		}

		~CServerCtrl() {}

		void *operator new(size_t vSize)
		{
			char *pTemp = NULL;
			if( mShmPtr == NULL )
			{
				return (void *)NULL;
			}
			pTemp = mShmPtr;

			return (void *)pTemp;
		}

		void operator delete(void *vMem) {}

		void *CreateSegment(size_t vSize)
		{
			if( vSize > mUnallocatedShmSize )
			{
				LOG_ERROR("default", "alloc memory failed: alloc(%d), remain(%d)", vSize, mUnallocatedShmSize);
				return (void *)NULL;
			}

			char *pAllocShm = mUnallocatedShmPtr;
			mUnallocatedShmSize -= vSize;
			mUnallocatedShmPtr += vSize;

			return (void *)pAllocShm;
		}
};

#endif
