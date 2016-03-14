
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_DISPATCH_H_
#define _W_DISPATCH_H_

#include <map>
#include <vector>
#include <functional>

#include "wCore.h"
#include "wNoncopyable.h"

/**
 * 每种回调Func_t（mFunc调用参数不同）需不同的wDispatch
 */

template<typename T,typename IDX>
class wDispatch : private wNoncopyable
{
	public:
		struct Func_t
		{
			IDX mActIdx;
			
			/**
			 * T为function类型，例：function<void(void)>
			 * mFunc用类似std::bind函数绑定，例：bind(&wTcpTask::Get, this, std::placeholders::_1)
			 */
			T mFunc;
			Func_t()
			{
				mActIdx = "";
			}
			Func_t(IDX ActIdx, T Func)
			{
				mActIdx = ActIdx;
				mFunc = Func;
			}
		};

		wDispatch();
		virtual ~wDispatch();
		void Initialize();
		
		bool Register(string className, IDX ActIdx, struct Func_t vFunc);

		inline struct Func_t * GetFuncT(string className, IDX ActIdx)
		{
			typename map<string, vector<struct Func_t> >::iterator mp = mProc.find(className);
			if(mp != mProc.end())
			{
				typename vector<struct Func_t>::iterator itvf = mp->second.begin();
				for(; itvf != mp->second.end() ; itvf++)
				{
					if(itvf->mActIdx == ActIdx)
					{
						return &*itvf;
					}
				}
			}
			return 0;
		}

	protected:
		map<string, vector<struct Func_t> > mProc;	//注册回调方法
		int mProcNum;
};

#include "wDispatch.inl"

#endif