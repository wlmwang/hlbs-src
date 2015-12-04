#ifndef _TIMER_H_
#define _TIMER_H_

#include "server_ctrl.h"

#define TIME_OF_KEEP_ALIVE 30000

//--------------------------------------------------
// 简单定时器，单位为毫秒
//-------------------------------------------------- 
class CTimer
{
	public:
		CTimer()
		{
			if( CServerCtrl::GetSingletonPtr()->mStatus == SERVER_STATUS_INIT )
			{
				mTimer = 0;
				mTimeRecord = 0;
			}
		}

		CTimer(int vTimer): mTimer(vTimer), mTimeRecord(vTimer) {}

		~CTimer() {}

		bool CheckTimer(int vInterval)
		{
			int vPassTime = mTimer - vInterval;
			if( vPassTime <= 0 )
			{
				// 补差值时间
				mTimer = mTimeRecord + vPassTime;
				return true;
			}
			else
			{
				mTimer = vPassTime;
				return false;
			}
		}
	private:
		int mTimer;
		int mTimeRecord;
};

#endif
