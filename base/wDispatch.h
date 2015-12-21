
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */


/**
 * 每种回调Func_t（mFunc调用参数不同）需不同的wDispatch
 */

#ifndef _W_DISPATCH_H_
#define _W_DISPATCH_H_

#include <string>
#include <map>
#include <vector>
#include <functional>
#include "wType.h"

template<typename T>
class wDispatch
{
	public:
		struct Func_t
		{
			string mActName;
			/**
			 * T为function类型，例：function<void(void)>
			 * mFunc用类似std::bind函数绑定，例：bind(&wTcpTask::Get, this, std::placeholders::_1)
			 */
			T mFunc;
			Func_t()
			{
				mActName = "";
			}
			Func_t(string ActName, T Func)
			{
				mActName = ActName;
				mFunc = Func;
			}
		};

		wDispatch();
		virtual ~wDispatch();
		void Initialize();
		void RegAct();

		bool RegisterCtl(string className, wDispatch *pDispatch);
		bool RegisterAct(string className, string actionName, struct Func_t vFunc);

		inline struct Func_t * GetFuncT(string className, string actionName)
		{
			typename map<string, vector<struct Func_t> >::iterator mp = mProc.find(className);
			if(mp != mProc.end())
			{
				typename vector<struct Func_t>::iterator itvf = mp->second.begin();
				for(; itvf != mp->second.end() ; itvf++)
				{
					if(itvf->mActName == actionName)
					{
						return &*itvf;
					}
				}
			}
			return 0;
		}

	protected:
		map<string, wDispatch*> mDispatch;	//路由对象
		
		map<string, vector<struct Func_t> > mProc;	//注册回调方法
		int mProcNum;
};

template<typename T>
wDispatch<T>::wDispatch()
{
	Initialize();
}

template<typename T>
void wDispatch<T>::Initialize()
{
	mProcNum = 0;
}

template<typename T>
wDispatch<T>::~wDispatch()
{
	//
}

template<typename T>
void wDispatch<T>::RegAct()
{
	typename map<string, wDispatch*>::iterator it = mDispatch.begin();
	for(; it != mDispatch.end(); it++)
	{
		it->second()->RegAct();
	}
}

template<typename T>
bool wDispatch<T>::RegisterCtl(string className, wDispatch *pDispatch)
{
	//pair<map<string, wDispatch*>::iterator, bool> itRet;
	//itRet = 
	mDispatch.insert(pair<string, wDispatch*>(className, pDispatch));
	return true;
	//return itRet.second;
}

template<typename T>
bool wDispatch<T>::RegisterAct(string className, string actionName, struct Func_t vFunc)
{
	wDispatch *pDispatch = NULL;
	typename map<string, wDispatch*>::iterator md = mDispatch.find(className);
	if(md == mDispatch.end())
	{
		return false;
	}
	pDispatch = md->second;	//路由对象

	vector<struct Func_t> vf;
	typename map<string, vector<struct Func_t> >::iterator mp = mProc.find(className);
	if(mp != mProc.end())
	{
		vf = mp->second;
		mProc.erase(mp);

		typename vector<struct Func_t>::iterator itvf = vf.begin();
		for(; itvf != vf.end() ; itvf++)
		{
			if(itvf->mActName == actionName)
			{
				itvf = vf.erase(itvf);
				itvf--;
			}
		}
	}
	mProcNum++;
	vf.push_back(vFunc);
	//pair<map<string, vector<struct Func_t> >::iterator, bool> itRet;
	//itRet = 
	mProc.insert(pair<string, vector<struct Func_t> >(className, vf));
	return true;
	//return itRet.second;
}

#endif