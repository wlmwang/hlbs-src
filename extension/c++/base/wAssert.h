
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _W_ASSERT_H_
#define _W_ASSERT_H_

#include "wCore.h"

#define W_ASSERT(a, b) \
	if(!(a)) \
	{  \
		b; \
	}

#endif
