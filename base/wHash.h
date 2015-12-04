
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_HASH_H_
#define _W_HASH_H_

unsigned int hash_string(const char* s)
{
	unsigned int hash = 5381;
	while (*s)
	{
		hash += (hash << 5) + (*s ++);
	}
	return hash & 0x7FFFFFFF;
}

#endif
