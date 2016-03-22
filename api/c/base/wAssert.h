
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_ASSERT_H_
#define _W_ASSERT_H_

#include "wCore.h"
#include "wLog.h"

#define W_ASSERT(a, b) \
	if(!(a)) \
	{  \
		b; \
	}

#endif
