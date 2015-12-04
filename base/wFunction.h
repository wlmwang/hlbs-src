
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_FUNCTION_H_
#define _W_FUNCTION_H_

#include "wType.h"

/**
 * 变为守护进程
 * @param  filename [互斥文件名]
 */
void InitDaemon(const char *filename);

/**
 * 创建共享内存
 * @param  filename [共享内存基于的文件名]
 * @param  pipe_id  [ftok附加参数]
 * @param  size     [共享内存大小]
 * @return          [共享内存在进程中映射地址]
 */
char *CreateShareMemory(const char *filename, int pipe_id, size_t size);

#endif
