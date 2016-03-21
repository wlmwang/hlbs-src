
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

template <typename T>
wMaster<T>::wMaster()
{
	Initialize();
}

template <typename T>
wMaster<T>::~wMaster() 
{
	for (int i = 0; i < mWorkerNum; ++i)
	{
		SAFE_DELETE_VEC(mWorkerPool[i]);	//delete []mWorkerPool[i];
	}
	SAFE_DELETE_VEC(mWorkerPool);	//delete []mWorkerPool;

	SAFE_DELETE(mShmAddr);
	SAFE_DELETE(mMutex);
}

template <typename T>
wWorker* wMaster<T>::NewWorker(int iSlot) 
{
	return new wWorker(iSlot); 
}

template <typename T>
void wMaster<T>::Initialize()
{
	mSlot = 0;
	mWorkerPool = NULL;
	mShmAddr = NULL;
	mMutex = NULL;
	mUseMutex = 1;
	mMutexHeld = 0;
	mDelay = 500;
	mPid = getpid();
	mNcpu = sysconf(_SC_NPROCESSORS_ONLN);
	mProcess = PROCESS_SINGLE;
}

template <typename T>
void wMaster<T>::PrepareRun()
{
	//
}

template <typename T>
void wMaster<T>::Run()
{
	//
}

template <typename T>
void wMaster<T>::PrepareStart()
{
	mWorkerNum = mNcpu;	//默认 worker 数量等于cpu数量
	mPid = getpid();

	PrepareRun();	//初始化服务器
}

template <typename T>
void wMaster<T>::SingleStart()
{
	mProcess = PROCESS_SINGLE;
	Run();
}

template <typename T>
void wMaster<T>::MasterStart()
{
	mProcess = PROCESS_MASTER;
	if (mWorkerNum > MAX_PROCESSES)
	{
		LOG_ERROR(ELOG_KEY, "[runtime] no more than %d processes can be spawned:", mWorkerNum);
		return;
	}
	
	InitSignals();
	CreatePidFile();
	
	//初始化workerpool内存空间
	mWorkerPool = new wWorker*[mWorkerNum];
	for(int i = 0; i < mWorkerNum; ++i)
	{
		mWorkerPool[i] = NewWorker(i);
	}
	
	//信号阻塞
	wSigSet stSigset;
	stSigset.AddSet(SIGCHLD);
	stSigset.AddSet(SIGALRM);
	stSigset.AddSet(SIGIO);
	stSigset.AddSet(SIGINT);
	stSigset.AddSet(SIGQUIT);
	stSigset.AddSet(SIGTERM);
	stSigset.AddSet(SIGHUP);	//RECONFIGURE
	stSigset.AddSet(SIGUSR1);	//

    if (stSigset.Procmask() == -1) 
    {
    	mErr = errno;
        LOG_ERROR(ELOG_KEY, "[runtime] sigprocmask() failed:%s", strerror(mErr));
		return;
    }
    stSigset.EmptySet();
	
	//防敬群锁
	if(mUseMutex == 1)
	{
		mShmAddr = new wShm(WAIT_MUTEX, 'a', sizeof(wShmtx));
		mShmAddr->CreateShm();

		mMutex = new wShmtx();
		mMutex->Create(mShmAddr);
	}
	
    //启动worker进程
    WorkerStart(mWorkerNum, PROCESS_RESPAWN);

	//master进程 信号处理
	while (true)
	{	
		HandleSignal();
		Run();
	}
}

template <typename T>
void wMaster<T>::HandleSignal()
{
	struct itimerval itv;
    int delay = 0;
	int sigio = 0;
	int iLive = 1;

	if(delay) 
	{
		if(g_sigalrm) 
		{
			sigio = 0;
			delay *= 2;
			g_sigalrm = 0;
		}
		
		LOG_ERROR(ELOG_KEY, "[runtime] termination cycle: %d", delay);

		itv.it_interval.tv_sec = 0;
		itv.it_interval.tv_usec = 0;
		itv.it_value.tv_sec = delay / 1000;
		itv.it_value.tv_usec = (delay % 1000 ) * 1000;

		//设置定时器，以系统真实时间来计算，送出SIGALRM信号
		if(setitimer(ITIMER_REAL, &itv, NULL) == -1) 
		{
			mErr = errno;
			LOG_ERROR(ELOG_KEY, "[runtime] setitimer() failed: %s", strerror(mErr));
		}
	}

	//阻塞方式等待信号量
	wSigSet stSigset;
	stSigset.Suspend();
	
	//SIGCHLD有worker退出
	if(g_reap)
	{
		LOG_ERROR(ELOG_KEY, "[runtime] reap children");
		
		g_reap = 0;
		ProcessGetStatus();
		iLive = ReapChildren();
	}
	
	//worker都退出,且收到了SIGTERM信号或SIGINT信号(g_terminate ==1)
	//或SIGQUIT信号(g_quit == 1),则master退出
	if (!iLive && (g_terminate || g_quit)) 
	{
		//ngx_master_process_exit(cycle);
		//MasterProcessExit();
	}
	
	//收到SIGTERM信号或SIGINT信号(g_terminate ==1)
	//通知所有worker退出，并且等待worker退出 
	if(g_terminate)
	{
		if(delay == 0) 
		{
			delay = 50;     //设置延时
		}
		if (sigio) 
		{
			sigio--;
			return;
		}
		sigio = mWorkerNum;

		if (delay > 1000) 
		{
			//延时已到，给所有worker发送SIGKILL信号，强制杀死worker
			SignalWorker(SIGKILL);
		} 
		else 
		{
			//给所有worker发送SIGTERM信号，通知worker退出
			SignalWorker(SIGTERM);
		}
		return;
	}

	if(g_quit)
	{
		SignalWorker(SIGQUIT);
		//关闭所有监听socket
		return;
	}
	
	//收到SIGHUP信号
	if (g_reconfigure) 
	{
		LOG_DEBUG(ELOG_KEY, "[runtime] reconfiguring");
		g_reconfigure = 0;
		
		PrepareStart();	//重新初始化配置
		WorkerStart(mWorkerNum, PROCESS_JUST_SPAWN);	//重启worker
		
		/* allow new processes to start */
		usleep(100*1000);	//100ms
		
		iLive = 1;
		SignalWorker(SIGTERM);	//关闭原来worker进程
	}
}

template <typename T>
int wMaster<T>::ReapChildren()
{
	const char *sProcessTitle = "worker process";
	pid_t pid;
	
	int iLive = 0;
	for (int i = 0; i < mWorkerNum; i++) 
    {
        if (mWorkerPool[i]->mPid == -1)
        {
            continue;
        }
		
		if(mWorkerPool[i]->mExited)	//已退出
		{
			//非分离，就同步文件描述符
			if(!mWorkerPool[i]->mDetached)
			{
				mWorkerPool[i]->mCh.Close();	//关闭channel
				
				struct ChannelReqClose_t stCh;
				stCh.mFD = -1;
				stCh.mPid = mWorkerPool[i]->mPid;
				stCh.mSlot = i;
				PassCloseChannel(&stCh);
			}
			
			//重启
			if(mWorkerPool[i]->mRespawn || !mWorkerPool[i]->mExiting
				|| !g_terminate || !g_quit)
			{
				pid = SpawnWorker(mWorkerPool[i]->mData, sProcessTitle, i);
				if(pid == -1)
				{
					LOG_ERROR(ELOG_KEY, "[runtime] could not respawn %d", i);
					continue;
				}
				
				struct ChannelReqOpen_t stCh;
				stCh.mSlot = mSlot;
				stCh.mPid = mWorkerPool[mSlot]->mPid;
				stCh.mFD = mWorkerPool[mSlot]->mCh[0];
				PassOpenChannel(&stCh);
				
				iLive = 1;
				continue;
			}
			
            if (i != mWorkerNum - 1) 
			{
                mWorkerPool[i]->mPid = -1;
            }
		}
		else if (mWorkerPool[i]->mExiting || !mWorkerPool[i]->mDetached) 
		{
			iLive = 1;
		}
    }
	return iLive;
}

template <typename T>
void wMaster<T>::WorkerStart(int n, int type)
{
	const char *sProcessTitle = "worker process";
	pid_t pid;
	
	//同步channel fd消息结构
	struct ChannelReqOpen_t stCh;
	memset(&stCh, 0, sizeof(struct ChannelReqOpen_t));
	
	for (int i = 0; i < mWorkerNum; ++i)
	{
		//创建worker进程
		pid = SpawnWorker((void *) &i, sProcessTitle, type);
	
		stCh.mSlot = mSlot;
        stCh.mPid = mWorkerPool[mSlot]->mPid;
        stCh.mFD = mWorkerPool[mSlot]->mCh[0];
        PassOpenChannel(&stCh);
	}
}

template <typename T>
void wMaster<T>::SignalWorker(int iSigno)
{
	int other = 0;
	int size = 0;
	
	struct ChannelReqCmd_s* pCh;
	struct ChannelReqQuit_t stChOpen;
	struct ChannelReqTerminate_t stChClose;
	switch(iSigno)
	{
		case SIGQUIT:
			pCh = &stChOpen;
			size = sizeof(struct ChannelReqQuit_t);
			break;
			
		case SIGTERM:
			pCh = &stChClose;
			size = sizeof(struct ChannelReqTerminate_t);
			break;
			
		default:
			other = 1;
	}
	pCh->mFD = -1;
	
	char *pStart = new char[size + sizeof(int)];
	*(int *)pStart = size;
	
	for (int i = 0; i < mWorkerNum; i++) 
    {
        if (mWorkerPool[i]->mDetached || mWorkerPool[i]->mPid == -1) 
		{
            continue;
        }
        
		if (mWorkerPool[i]->mExiting && iSigno == SIGQUIT)
        {
            continue;
        }
		
        if(other)
		{
	        LOG_DEBUG(ELOG_KEY, "[runtime] pass signal channel s:%i pid:%P to:%P", 
	        	pCh->mSlot, pCh->mPid, mWorkerPool[i]->mPid);

	        /* TODO: EAGAIN */
			memcpy(pStart + sizeof(int), (char *)pCh, size);
			mWorkerPool[i]->mCh.SendBytes(pStart, size + sizeof(int));
		}
					   
		LOG_ERROR(ELOG_KEY, "[runtime] kill (%d, %d)", mWorkerPool[i]->mPid, iSigno);
		
        if (kill(mWorkerPool[i]->mPid, iSigno) == -1) 
		{
            mErr = errno;
			
			LOG_ERROR(ELOG_KEY, "[runtime] kill(%d, %d) failed:%s", mWorkerPool[i]->mPid, iSigno, strerror(mErr));
            if (mErr == ESRCH) 
			{
                mWorkerPool[i]->mExited = 1;
                mWorkerPool[i]->mExiting = 0;
				
                g_reap = 1;
            }
            continue;
        }
    }
    
    SAFE_DELETE_VEC(pStart);
}

template <typename T>
void wMaster<T>::PassOpenChannel(struct ChannelReqOpen_t *pCh)
{
	int size = sizeof(struct ChannelReqOpen_t);
	
	char *pStart = new char[size + sizeof(int)];
	*(int *)pStart = size;

	for (int i = 0; i < mWorkerNum; i++) 
    {
		//当前分配到worker进程表项索引（无需发送给自己）
        if (i == mSlot|| mWorkerPool[i]->mPid == -1|| mWorkerPool[i]->mCh[0] == FD_UNKNOWN)
        {
            continue;
        }

        LOG_DEBUG(ELOG_KEY, "[runtime] pass open channel s:%d pid:%d fd:%d to s:%i pid:%d fd:%d", 
        	pCh->mSlot, pCh->mPid, pCh->mFD, i, mWorkerPool[i]->mPid, mWorkerPool[i]->mCh[0]);
        
        /* TODO: EAGAIN */

		memcpy(pStart + sizeof(int), (char*) pCh, size);
		mWorkerPool[i]->mCh.SendBytes(pStart, size + sizeof(int));
    }

    SAFE_DELETE_VEC(pStart);
}

template <typename T>
void wMaster<T>::PassCloseChannel(struct ChannelReqClose_t *pCh)
{
	int size = sizeof(struct ChannelReqClose_t);

	char *pStart = new char[size + sizeof(int)];
	*(int *)pStart = size;
    
	for (int i = 0; i < mWorkerNum; i++) 
    {
		if (mWorkerPool[i]->mExited || mWorkerPool[i]->mPid == -1|| mWorkerPool[i]->mCh[0] == FD_UNKNOWN)
		{
			continue;
		}

        LOG_DEBUG(ELOG_KEY, "[runtime] pass close channel s:%i pid:%d to:%d", 
        	pCh->mSlot, pCh->mPid, mWorkerPool[i]->mPid);
        
        /* TODO: EAGAIN */
		memcpy(pStart + sizeof(int), (char *)pCh, size);
		mWorkerPool[i]->mCh.SendBytes(pStart, size + sizeof(int));
    }
	
    SAFE_DELETE_VEC(pStart);
}

template <typename T>
pid_t wMaster<T>::SpawnWorker(void* pData, const char *title, int type)
{
	int s;
	if(type >= 0)
	{
		s = type;
	}
	else
	{
		for (s = 0; s < mWorkerNum; ++s)
		{
			if(mWorkerPool[s]->mPid == -1)
			{
				break;
			}
		}
	}
	mSlot = s;

	wWorker *pWorker = mWorkerPool[mSlot];
	if(pWorker->InitChannel() < 0)
	{
		mErr = errno;
		LOG_ERROR(ELOG_KEY, "[runtime] socketpair() failed while spawning: %s", strerror(mErr));
		return -1;
	}

	//设置第一个描述符的异步IO通知机制（FIOASYNC现已被O_ASYNC标志位取代）
	u_long on = 1;
    if (ioctl(pWorker->mCh[0], FIOASYNC, &on) == -1) 
    {
    	mErr = errno;
        LOG_ERROR(ELOG_KEY, "[runtime] ioctl(FIOASYNC) failed while spawning %s:%s", title, strerror(mErr));
        pWorker->Close();
        return -1;
    }

    //设置将要在文件描述符channel[0]上接收SIGIO 或 SIGURG事件信号的进程标识
    if (fcntl(pWorker->mCh[0], F_SETOWN, mPid) == -1) 
    {
    	mErr = errno;
        LOG_ERROR(ELOG_KEY, "[runtime] fcntl(F_SETOWN) failed while spawning %s:%s", title, strerror(mErr));
        pWorker->Close();
        return -1;
    }

    pid_t pid = fork();
    switch (pid) 
    {
	    case -1:
	    	mErr = errno;
	        LOG_ERROR(ELOG_KEY, "[runtime] fork() failed while spawning %s:%s", title, strerror(mErr));
	        pWorker->Close();
	        return -1;
			
	    case 0:
	    	//worker进程
	        pWorker->InitWorker(mWorkerNum, mWorkerPool, mUseMutex, mShmAddr, mMutex, mDelay);
	        pWorker->PrepareStart(mSlot, type, pData);
	        pWorker->Start();
	        _exit(0);	//TODO 进程退出
	        break;

	    default:
	        break;
    }

    LOG_DEBUG(ELOG_KEY, "[runtime] start %s %d", title, pid);
    
    //更新进程表
    pWorker->mSlot = mSlot;
    pWorker->mPid = pid;
	
	if(type >= 0)
	{
		return pid;
	}
	
    switch (type)
    {
    	case PROCESS_NORESPAWN:
    		pWorker->mRespawn = 1;
    		break;

    	case PROCESS_RESPAWN:
    		pWorker->mRespawn = 1;
    		break;
    	/*
    	case PROCESS_DETACHED:
    		pWorker->mDetached = 1;
    	*/
    }
	
    return pid;
}

template <typename T>
int wMaster<T>::CreatePidFile()
{
	if (mPidFile.FileName().size() <= 0)
	{
		mPidFile.FileName() = "master.pid";
	}
    if (mPidFile.Open(O_RDWR| O_CREAT) == -1) 
    {
    	mErr = errno;
    	LOG_ERROR(ELOG_KEY, "[runtime] create pid(%s) file failed: %s", mPidFile.FileName().c_str(), strerror(mErr));
    	return -1;
    }
	string sPid = Itos((int) mPid);
    if (mPidFile.Write(sPid.c_str(), sPid.size(), 0) == -1) 
    {
    	mErr = errno;
		LOG_ERROR(ELOG_KEY, "[runtime] write process pid to file failed: %s", strerror(mErr));
        return -1;
    }
	
    mPidFile.Close();
    return 0;
}

template <typename T>
void wMaster<T>::DeletePidFile()
{
    if (mPidFile.Unlink() == -1) 
    {
    	mErr = errno;
    	LOG_ERROR(ELOG_KEY, "unlink %s failed:%s", mPidFile.FileName().c_str(), strerror(mErr));
		return;
    }
	return;
}

template <typename T>
void wMaster<T>::InitSignals()
{
	wSignal::signal_t *pSig;
	wSignal stSignal;
	for (pSig = g_signals; pSig->mSigno != 0; ++pSig)
	{
		if(stSignal.AddSig_t(pSig) == -1)
		{
			mErr = errno;
			LOG_ERROR(ELOG_KEY, "[runtime] sigaction(%s) failed(ignored):(%s)", pSig->mSigname, strerror(mErr));
		}
	}
}

template <typename T>
void wMaster<T>::ProcessGetStatus()
{    
	const char *process = "unknown process";
    
	int one = 0;
	pid_t	pid;
	int status;
    while(true)
	{
        pid = waitpid(-1, &status, WNOHANG);

        if(pid == 0) 
		{
            return;
        }
		
        if(pid == -1) 
		{
            mErr = errno;
			
            if(mErr == EINTR) 
			{
                continue;
            }

            if(mErr == ECHILD && one) 
			{
                return;
            }

            if(mErr == ECHILD) 
			{
				LOG_ERROR(ELOG_KEY, "[runtime] waitpid() failed:%s", strerror(mErr));
                return;
            }
			
			LOG_ERROR(ELOG_KEY, "[runtime] waitpid() failed:%s", strerror(mErr));
            return;
        }
		
        one = 1;
		int i;
		for(i = 0; i < mWorkerNum; ++i)
		{
			if (mWorkerPool[i]->mPid == pid)
			{
                mWorkerPool[i]->mStat = status;
                mWorkerPool[i]->mExited = 1;
                process = mWorkerPool[i]->mName;
                break;
			}
		}
		
        if(WTERMSIG(status)) 
		{
			LOG_ERROR(ELOG_KEY, "[runtime] %s %d exited on signal %d%s", process, pid, WTERMSIG(status), WCOREDUMP(status) ? " (core dumped)" : "");
        }
		else
		{
			LOG_ERROR(ELOG_KEY, "[runtime] %s %d exited with code %d", process, pid, WTERMSIG(status));
        }
		
		//退出后不重启
        if(WEXITSTATUS(status) == 2 && mWorkerPool[i]->mRespawn) 
		{
			LOG_ERROR(ELOG_KEY, "[runtime] %s %d exited with fatal code %d and cannot be respawned", process, pid, WTERMSIG(status));			
            mWorkerPool[i]->mRespawn = 0;
        }
    }
}
