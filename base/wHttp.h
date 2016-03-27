
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_HTTP_H_
#define _W_HTTP_H_

#include "wCore.h"
#include "wIO.h"
#include "wLog.h"
#include "wMisc.h"
#include "wSocket.h"

class wHttp : public wSocket
{
	public:	
		wHttp();
		virtual ~wHttp();
		void Initialize();
		
		virtual ssize_t RecvBytes(char *vArray, size_t vLen);
		virtual ssize_t SendBytes(char *vArray, size_t vLen);
		
		int ParseRequest();
		int ParseHeadLine();
		int ParseBody();
};

#endif
