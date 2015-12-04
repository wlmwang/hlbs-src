#ifndef _LOGIN_CONFIG_H_
#define _LOGIN_CONFIG_H_

#include "server_ctrl.h"
#include <string.h>

//--------------------------------------------------
// 配置文件读取的数据结构
//-------------------------------------------------- 
class CLoginConfig: public CSingleton<CLoginConfig>
{
	public:
		char mLoginIPAddress[MAX_IP_ADDR_LEN];
		char mClientIPAddress[MAX_SCENE_NUM][MAX_IP_ADDR_LEN];
		unsigned int mLoginPort;
		unsigned int mClientPort[MAX_SCENE_NUM];

		CLoginConfig()
		{
			if( CServerCtrl::GetSingletonPtr()->mStatus == SERVER_STATUS_INIT )
			{
				Initialize();
			}
		}

		~CLoginConfig() {}

		// 初始化
		void Initialize()
		{
			memset(mLoginIPAddress, 0, MAX_IP_ADDR_LEN);
			mLoginPort = 0;
			memset(mClientIPAddress, 0, MAX_IP_ADDR_LEN * MAX_SCENE_NUM);
			for ( int i = 0; i < MAX_SCENE_NUM; i++ ) 
			{
				mClientPort[i] = 0;
			}
		}

		void *operator new(size_t vSize)
		{
			return (void *)(CServerCtrl::GetSingletonPtr()->CreateSegment(vSize));
		}

		void operator delete(void *pMem)
		{

		}

		static size_t GetMemorySize()
		{
			return sizeof(CLoginConfig);
		}
};

#endif
