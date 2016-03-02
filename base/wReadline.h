
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_READLINE_H_
#define _W_READLINE_H_

#include <readline/readline.h>
#include <readline/history.h>

#include "wType.h"

class wReadline
{
	public:
		typedef char** (*CompletionFunc_t)(const char *pText, int iStart, int iEnd);	//Tab键能补齐的函数类型

		wReadline();
		~wReadline();
		void Initialize();
		
		char *ReadCmdLine();
		virtual char *StripWhite(char *pOrig);
		virtual bool IsUserQuitCmd(char *pCmd);
	
		bool SetCompletion(CompletionFunc_t pFunc);
		
		void SetPrompt(char* cStr, int iLen)
		{
			memcpy(mPrompt, cStr, iLen);
		}
	protected:
		char mPrompt[32];
		char *mLineRead;
		char *mStripLine;
		
		CompletionFunc_t mFunc;

		static const char *mQuitCmd[];
		static const unsigned char mQuitCmdNum;
};


#endif
