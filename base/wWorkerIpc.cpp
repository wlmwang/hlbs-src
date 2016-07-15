
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#include "wWorkerIpc.h"
#include "wChannel.h"
#include "wChannelTask.h"

wWorkerIpc::wWorkerIpc(wWorker *pWorker) : mWorker(pWorker)
{
	mEpollEventPool.reserve(LISTEN_BACKLOG);
	memset((void *)&mEpollEvent, 0, sizeof(mEpollEvent));
}

int wWorkerIpc::PrepareRun()
{
	if (InitEpoll() < 0) exit(2);

	//worker自身channel[1]被监听
	if (mWorker->mMaster->mWorkerPool != NULL)
	{
		wChannel *pChannel = &mWorker->mMaster->mWorkerPool[mWorker->mSlot]->mCh;	//当前worker进程表项
		if (pChannel != NULL)
		{
			wTask *pTask = new wChannelTask(pChannel, mWorker);
			if (NULL != pTask)
			{
				pTask->Status() = TASK_RUNNING;
				if (AddToEpoll(pTask) >= 0)
				{
					AddToTaskPool(pTask);
				}
				else
				{
					SAFE_DELETE(pTask);
					exit(2);
				}
			}
		}
		else
		{
			LOG_ERROR(ELOG_KEY, "[system] worker pool slot(%d) illegal", mWorker->mSlot);
			exit(2);
		}
	}
}

int wWorkerIpc::Run()
{
	LOG_INFO(ELOG_KEY, "[system] worker ipc server start succeed");
	mStatus = SERVER_RUNNING;

	do {
		Recv();
	} while(IsRunning());
}

int wWorkerIpc::InitEpoll()
{
	mEpollFD = epoll_create(LISTEN_BACKLOG); //511
	if (mEpollFD < 0)
	{
		mErr = errno;
		LOG_ERROR(ELOG_KEY, "[system] epoll_create failed:%s", strerror(mErr));
		return -1;
	}
	return mEpollFD;
}

void wWorkerIpc::CleanEpoll()
{
	if (mEpollFD != -1)
	{
		close(mEpollFD);
	}
	mEpollFD = -1;
	memset((void *)&mEpollEvent, 0, sizeof(mEpollEvent));
	mEpollEventPool.clear();
}

int wWorkerIpc::AddToEpoll(wTask* pTask, int iEvents, int iOp)
{
	mEpollEvent.events = iEvents | EPOLLERR | EPOLLHUP; //|EPOLLET
	mEpollEvent.data.fd = pTask->IO()->FD();
	mEpollEvent.data.ptr = pTask;
	int iRet = epoll_ctl(mEpollFD, iOp, pTask->IO()->FD(), &mEpollEvent);
	if (iRet < 0)
	{
		mErr = errno;
		LOG_ERROR(ELOG_KEY, "[system] fd(%d) add into epoll failed: %s", pTask->IO()->FD(), strerror(mErr));
		return -1;
	}
	LOG_DEBUG(ELOG_KEY, "[system] %s fd %d events read %d write %d", iOp == EPOLL_CTL_MOD ? "mod":"add", 
		pTask->IO()->FD(), mEpollEvent.events & EPOLLIN, mEpollEvent.events & EPOLLOUT);
	return 0;
}

int wWorkerIpc::AddToTaskPool(wTask* pTask)
{
	W_ASSERT(pTask != NULL, return -1);

	mTaskPool.push_back(pTask);
	//epoll_event大小
	mTaskCount = mTaskPool.size();
	if (mTaskCount > mEpollEventPool.capacity())
	{
		mEpollEventPool.reserve(mTaskCount * 2);
	}
	LOG_DEBUG(ELOG_KEY, "[system] fd(%d) add into task pool", pTask->IO()->FD());
	return 0;
}

void wWorkerIpc::CleanTaskPool()
{
	if (mTaskPool.size() > 0)
	{
		vector<wTask*>::iterator it;
		for (it = mTaskPool.begin(); it != mTaskPool.end(); it++)
		{
	    	if ((*it)->IO()->TaskType() != TASK_UNIXS)
	    	{
	    		(*it)->DeleteIO();
	    	}
			SAFE_DELETE(*it);
		}
	}
	mTaskPool.clear();
	mTaskCount = 0;
}

int wWorkerIpc::RemoveEpoll(wTask* pTask)
{
	int iFD = pTask->IO()->FD();
	mEpollEvent.data.fd = iFD;
	if (epoll_ctl(mEpollFD, EPOLL_CTL_DEL, iFD, &mEpollEvent) < 0)
	{
		mErr = errno;
		LOG_ERROR(ELOG_KEY, "[system] epoll remove socket fd(%d) error : %s", iFD, strerror(mErr));
		return -1;
	}
	return 0;
}

std::vector<wTask*>::iterator wWorkerIpc::RemoveTaskPool(wTask* pTask)
{
    std::vector<wTask*>::iterator it = std::find(mTaskPool.begin(), mTaskPool.end(), pTask);
    if (it != mTaskPool.end())
    {
    	if ((*it)->IO()->TaskType() != TASK_UNIXS)
    	{
    		(*it)->DeleteIO();
    	}
    	SAFE_DELETE(*it);
        it = mTaskPool.erase(it);
    }
    mTaskCount = mTaskPool.size();
    return it;
}

void wWorkerIpc::Recv()
{
	int iRet = epoll_wait(mEpollFD, &mEpollEventPool[0], mTaskCount, mTimeout /*10ms*/);
	if (iRet < 0)
	{
		mErr = errno;
		LOG_ERROR(ELOG_KEY, "[system] epoll_wait failed: %s", strerror(mErr));
		return;
	}
	
	int iFD = FD_UNKNOWN;
	int iLenOrErr;
	wTask *pTask = NULL;
	for (int i = 0 ; i < iRet ; i++)
	{
		pTask = (wTask *)mEpollEventPool[i].data.ptr;
		iFD = pTask->IO()->FD();
		
		if (iFD == FD_UNKNOWN)
		{
			LOG_DEBUG(ELOG_KEY, "[system] socket FD is error, fd(%d), close it", iFD);
			if (RemoveEpoll(pTask) >= 0)
			{
				RemoveTaskPool(pTask);
			}
			continue;
		}
		if (!pTask->IsRunning())	//多数是超时设置
		{
			LOG_DEBUG(ELOG_KEY, "[system] task status is quit, fd(%d), close it", iFD);
			if (RemoveEpoll(pTask) >= 0)
			{
				RemoveTaskPool(pTask);
			}
			continue;
		}
		if (mEpollEventPool[i].events & (EPOLLERR | EPOLLPRI))	//出错(多数为sock已关闭)
		{
			mErr = errno;
			LOG_ERROR(ELOG_KEY, "[system] epoll event recv error from fd(%d), close it: %s", iFD, strerror(mErr));
			if (RemoveEpoll(pTask) >= 0)
			{
				RemoveTaskPool(pTask);
			}
			continue;
		}
		
		if (mEpollEventPool[i].events & EPOLLIN)
		{
			//套接口准备好了读取操作
			if ((iLenOrErr = pTask->TaskRecv()) < 0)
			{
				if (iLenOrErr == ERR_CLOSED)
				{
					LOG_DEBUG(ELOG_KEY, "[system] tcp socket closed by client");
				}
				else if (iLenOrErr == ERR_MSGLEN)
				{
					LOG_ERROR(ELOG_KEY, "[system] recv message invalid len");
				}
				else
				{
					LOG_ERROR(ELOG_KEY, "[system] EPOLLIN(read) failed or tcp socket closed: %s", strerror(pTask->IO()->Errno()));
				}
				if (RemoveEpoll(pTask) >= 0)
				{
					RemoveTaskPool(pTask);
				}
			}
		}
		else if (mEpollEventPool[i].events & EPOLLOUT)
		{
			//清除写事件
			if (pTask->WritableLen() <= 0)
			{
				AddToEpoll(pTask, EPOLLIN, EPOLL_CTL_MOD);
				continue;
			}
			//套接口准备好了写入操作
			if (pTask->TaskSend() < 0)	//写入失败，半连接，对端读关闭
			{
				LOG_ERROR(ELOG_KEY, "[system] EPOLLOUT(write) failed or tcp socket closed: %s", strerror(pTask->IO()->Errno()));
				if (RemoveEpoll(pTask) >= 0)
				{
					RemoveTaskPool(pTask);
				}
			}
		}
	}
}
