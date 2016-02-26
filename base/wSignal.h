
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_SIGNAL_H_
#define _W_SIGNAL_H_

#include <signal.h>
#include "wType.h"
#include "wLog.h"
#include "wNoncopyable.h"

/*
typedef struct {
    int     signo;
    char   *signame;	//信号的字符串表现形式，如"SIGIO"
    char   *name;		//信号的名称，如"stop"
    void  (*handler)(int signo);	    //信号处理函数
} signal_t;
*/

class wSignal : private wNoncopyable
{
	public:
		//SIG_DFL(采用缺省的处理方式)，也可以为SIG_IGN
		wSignal(__sighandler_t  func)
		{
			sigemptyset(&mSigAct.sa_mask);
			mSigAct.sa_sigaction = func;
			mSigAct.sa_flags = 0;
		}

		//添加屏蔽集
		int AddMaskSet(int signo)
		{
			return sigaddset(&mSigAct.sa_mask, signo);
		}

		//添加信号处理
		int AddSignal(int signo, struct sigaction *oact = NULL)
		{
			return sigaction(signo, &mSigAct, oact);
		}

		virtual ~wSignal() {}

	private:
		struct sigaction mSigAct;
};

#endif