
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _COMMON_CMD_H_
#define _COMMON_CMD_H_

//Dispatch
#define REG_FUNC(ActIdx, vFunc) wDispatch<function<int(char*, int)>, int>::Func_t {ActIdx, std::bind(vFunc, this, std::placeholders::_1, std::placeholders::_2)}
#define DEC_DISP(dispatch) wDispatch<function<int(char*, int)>, int> dispatch
#define DEC_FUNC(func) int func(char *pBuffer, int iLen)

#endif