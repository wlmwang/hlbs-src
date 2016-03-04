
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "wSignal.h"

wSignal::wSignal() {}

wSignal::wSignal(__sighandler_t  func)
{
	mSigAct.sa_handler = func;
	mSigAct.sa_flags = 0;
	sigemptyset(&mSigAct.sa_mask);
}

wSignal::~wSignal() {}

int wSignal::AddMaskSet(int signo)
{
	return sigaddset(&mSigAct.sa_mask, signo);
}

int wSignal::AddHandler(__sighandler_t  func)
{
	mSigAct.sa_handler = func;
	mSigAct.sa_flags = 0;
	return sigemptyset(&mSigAct.sa_mask);
}

//添加信号处理
int wSignal::AddSigno(int signo, struct sigaction *oact = NULL)
{
	return sigaction(signo, &mSigAct, oact);
}

int wSignal::AddSig_t(const signal_t *pSig)
{
	int code = AddHandler(pSig->mHandler);
	if (code < 0)
	{
		return code;
	}
	return AddSigno(pSig->mSigno);
}
		
void SignalHandler(int signo)
{
	LOG_DEBUG(ELOG_KEY, "[runtime] received signal: %d", signo);
}
