
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

typedef struct {
    int     signo;
    //信号的字符串表现形式，如"SIGIO"
    char   *signame;
    //信号的名称，如"stop"
    char   *name;
    //信号处理函数
    void  (*handler)(int signo);
} signal_t;


class wSignal : private wNoncopyable
{
	public:
		wSignal(string sName);
		virtual ~wSignal();

		int InitSignals()
		{
		    signal_t      *sig;
		    struct sigaction   sa;

		    for (sig = signals; sig->signo != 0; sig++) 
		    {
		        memzero(&sa, sizeof(struct sigaction));
		        sa.sa_handler = sig->handler;
		        sigemptyset(&sa.sa_mask);
		        if (sigaction(sig->signo, &sa, NULL) == -1) 
		        {
		        	//log...
		        }
		    }

		    return 0;
		}
};

#endif