
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_CORE_H_
#define _W_CORE_H_

#include "wLinux.h"

#include "wDef.h"

#define ALIGNMENT	sizeof(unsigned long)    /* platform word */

#define ALIGN(d, a)		(((d) + (a - 1)) & ~(a - 1))
#define ALIGN_PTR(p, a)	(u_char *) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))

#define SAFE_DELETE(x) { if(x) { delete(x); (x) = NULL; } }
#define SAFE_DELETE_VEC(x) { if(x) { delete [] (x); (x) = NULL; } }
#define SAFE_FREE(x) { if(x) { free(x); (x) = NULL; } }

using namespace std;

extern char **environ;

#endif
