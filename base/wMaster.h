
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_MASTER_H_
#define _W_MASTER_H_

#include <map>
#include <vector>

#include "wType.h"
#include "wLog.h"
#include "wSingleton.h"
#include "wFile.h"

template <typename T>
class wMaster : public wSingleton<T>
{
	public:
		wMaster()
		{
			//
		}

		virtual ~wMaster()
		{
			//
		}

		void Initialize()
		{
			mWorkerNum = sysconf(_SC_NPROCESSORS_ONLN);
			mSlot = 0;
			mPid = getpid();

			//初始化worker
			for(int i = 0; i < mWorkerNum; ++i)
			{
				mWorkerPool[i] = new wWorker();
			}
		}

		void MasterStart()
		{
			pid_t pid;

			wChannel ch;
			memset(&ch, sizeof(wChannel));

			ch.command = CMD_OPEN_CHANNEL;	//打开channel，新的channel

			for (int i = 0; i < mWorkerNum; ++i)
			{
				pid = SpawnWorker(i, "worker process", 0);
			
		        //ch.pid = mWorkerPool[mSlot].mPid;
		        //ch.slot = mSlot;
		        //ch.fd = mWorkerPool[mSlot].mCh[0];
		        //pass_open_channel(cycle, &ch);	//发送此ch[0]到所有一创建的worker进程。
			}
		}

		pid_t SpawnWorker(int i, const char *title, int type)
		{
			int s = 0;
			for (s = 0; s <= mWorkerNum; ++s)
			{
				if(mWorkerPool[s].mPid == -1)
				{
					break;
				}
			}
			if (s == mWorkerNum)
			{
				LOG_ERROR(ELOG_KEY, "[runtime] no more than %d processes can be spawned:", mWorkerNum);
				return -1;
			}

			mSlot = s;	//当前第几个

			wWorker *pWorker = mWorkerPool[mSlot];	//取出进程表

			pWorker->InitChannel();

			//设置第一个描述符的异步IO通知机制（FIOASYNC现已被O_ASYNC标志位取代）
			u_long on = 1;
	        if (ioctl(pWorker->mCh[0], FIOASYNC, &on) == -1) 
	        {
	            LOG_ERROR(ELOG_KEY, "[runtime] ioctl(FIOASYNC) failed while spawning \"%s\":", title, strerror(errno));
	            pWorker->mCh.Close();
	            return -1;
	        }

	        //设置将要在文件描述符channel[0]上接收SIGIO 或 SIGURG事件信号的进程或进程组的标识
	        if (fcntl(pWorker->mCh[0], F_SETOWN, mPid) == -1) 
	        {
	            LOG_ERROR(ELOG_KEY, "[runtime] fcntl(F_SETOWN) failed while spawning \"%s\":", title, strerror(errno));
	            pWorker->mCh.Close();
	            return NGX_INVALID_PID;
	        }
	        
	        pid_t pid = fork();

		    switch (pid) 
		    {
		    case -1:
		        LOG_ERROR(ELOG_KEY, "[runtime] fork() failed while spawning \"%s\":", title, strerror(errno));
		        pWorker->mCh.Close();
		        return -1;

		    case 0:
		        pWorker->mPid = ngx_getpid();	//更新当前pid为worker进程pid
		        pWorker->mSlot = s;
		        pWorker->PrepareStart(i);
		        pWorker->Start();
		        break;

		    default:
		        break;
		    }

		    LOG_DEBUG(ELOG_KEY, "[runtime] start %s %P", title, pid);
		    
		    //更新进程表
		    pWorker->mExited = 0;
		    switch (type)
		    {
		    	case PROCESS_RESPAWN:
		    		pWorker->mRespawn = 1;
		    }
	        return pid;
		}

		void SingleStart()
		{
			//
		}

		int CreatePidFile()
		{
		    size_t len;
		    u_char vPid[NGX_INT64_LEN + 2];

		    mPidFile.Open(PID_PATH, O_RDWR| O_CREAT);

		    if (mPidFile.FD() == -1) 
		    {
		    	return -1;
		    }

	        len = snprintf(vPid, NGX_INT64_LEN + 2, "%P%N", getpid()) - vPid;

	        if (mPidFile.Write(vPid, len, 0) == -1) 
	        {
	            return -1;
	        }

	        mPidFile.Close();

		    return 0;
		}

		void DeletePidFile()
		{
		    if (mPidFile.Unlink() == -1) 
		    {
		    	LOG_ERROR(ELOG_KEY, "unlink \"%s\" failed", mPidFile.c_str());
		    }
		}

	private:
		wFile mPidFile;
		
		pid_t mPid;	//master进程id
		int mWorkerNum;	//worker总数量
		int mSlot;	//进程表分配到数量
		wWorker *mWorkerPool;	//进程表
};

#endif