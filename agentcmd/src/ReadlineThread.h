
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _READLINE_THREAD_H_
#define _READLINE_THREAD_H_

#include "wCore.h"
#include "wThread.h"
#include "wReadline.h"

class AgentCmd;
class ReadlineThread : public wThread
{
	friend class AgentCmd;
	public:
		ReadlineThread(char *pPrompt,int iLen);
		virtual ~ReadlineThread();

		virtual int PrepareRun();
		virtual int Run();
		virtual bool IsBlocked();

		static char* Generator(const char *pText, int iState);
		static char** Completion(const char *pText, int iStart, int iEnd);
	protected:
		wReadline mReadline;
		char *mCmdLine;
};

#endif