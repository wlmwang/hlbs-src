
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_DISPATCH_H_
#define _W_DISPATCH_H_

#include <string>
#include <map>
#include <vector>
#include <functional>
#include "wType.h"

/**
 * T: function<int(string)>
 * mFunc: bind(&AgentCmd::GetCmd, wCallObj, std::placeholders::_1)
 */
template<typename T,typename P1 = void,typename P2 = void,typename P3 = void,typename P4 = void,typename P5 = void>
struct Func_t
{
	string mActName;
	T mFunc;
	int mArgc;
	P1 mArg1;
	P2 mArg2;
	P3 mArg3;
	P4 mArg4;
	P5 mArg5;
	Func_t()
	{
		mActName = "";
		mArgc = 0;
	}
};

class wDispatch
{
	public:
		wDispatch();
		virtual ~wDispatch();
		void Initialize();
		
		
		bool RegisterCtl(string className, wDispatch *pDispatch);
		bool RegisterAct(string className, string actionName, Func_t vFunc);
		
		Func_t* GetFuncT(string className, string actionName);
		virtual int Dispatch(string className, string actionName);
	protected:
		map<string, wDispatch*> mDispatch;
		map<string, vector<Func_t> > mProc;
		int mProcNum;
};

wDispatch::wDispatch()
{
	mProcNum = 0;
}

wDispatch::~wDispatch()
{
	//
}

void wDispatch::RegAct()
{
	if(map<string, wDispatch*>::iterator it = mDispatch.begin();it != mDispatch.end(); it++)
	{
		it->second()->RegAct();
	}
}

bool wDispatch::RegisterCtl(string className, wDispatch *pDispatch)
{
	pair<map<string, wDispatch*>::iterator, bool> itRet;
	itRet = mDispatch.insert(pair<string, wDispatch*>(className, pDispatch));
	return itRet->second;
}

Func_t* GetFuncT(string className, string actionName)
{
	vector<Func_t> vf;
	map<string, vector<Func_t> >::iterator mp = mProc.find(className);
	if(mp != mProc.end())
	{
		vf = mp->second;
		Func_t ft;
		bool bFlag = false;
		for(vector<Func_t>::iterator itvf = vf.begin(); itvf != vf.end() ; itvf++)
		{
			if(itvf->mActName == actionName)
			{
				ft = *itvf;
				bFlag = true;
				break;
			}
		}
		if(bFlag)
		{
			return &ft;
		}
	}
	return 0;
}

bool wDispatch::RegisterAct(string className, string actionName, Func_t vFunc)
{
	wDispatch *pDispatch = NULL;
	map<string, wDispatch*>::iterator md = mDispatch.find(className);
	if(md == mDispatch.end())
	{
		return false;
	}
	pDispatch = md->second;	//对象

	vector<Func_t> vf;
	map<string, vector<Func_t> >::iterator mp = mProc.find(className);
	if(mp != mProc.end())
	{
		vf = mp->second;
		mProc.erase(mp);
		
		for(vector<Func_t>::iterator itvf = vf.begin(); itvf != vf.end() ; itvf++)
		{
			if(itvf->mActName == actionName)
			{
				itvf = vf.erase(itvf);
				itvf--;
			}
		}
	}
	mProcNum ++;
	vf.push_back(vFunc);
	pair<map<string, vector<Func_t> >::iterator, bool> itRet;
	itRet = mProc.insert(pair<string, vector<Func_t> >(className, vf));
	return itRet->second;
}

int wDispatch::Dispatch(string className, string actionName)
{
	wDispatch *pDispatch = NULL;
	map<string, wDispatch*>::iterator md = mDispatch.find(className);
	if(md == mDispatch.end())
	{
		return -1;
	}
	pDispatch = md->second;
	
	vector<Func_t> vf;
	map<string, vector<Func_t> >::iterator mp = mProc.find(className);
	if(mp != mProc.end())
	{
		vf = mp->second;
		Func_t ft;
		bool bFlag = false;
		for(vector<Func_t>::iterator itvf = vf.begin(); itvf != vf.end() ; itvf++)
		{
			if(itvf->mActName == actionName)
			{
				ft = *itvf;
				bFlag = true;
				break;
			}
		}
		
		if(bFlag)
		{
			switch(ft.mArgc)
			{
				case 0:
					ft.mFunc();
					break;
				case 1:
					ft.mFunc(mArg1);
					break;
				case 2:
					ft.mFunc(mArg1,mArg2);
					break;
				case 3:
					ft.mFunc(mArg1,mArg2,mArg3);
					break;
				case 4:
					ft.mFunc(mArg1,mArg2,mArg3,mArg4);
					break;
				case 5:
					ft.mFunc(mArg1,mArg2,mArg3,mArg4,mArg5);
					break;
			}
			return 0;
		}
	}
	return -1;
}

#endif