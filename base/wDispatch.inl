
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

template<typename T,typename IDX>
wDispatch<T,IDX>::wDispatch()
{
	Initialize();
}

template<typename T,typename IDX>
void wDispatch<T,IDX>::Initialize()
{
	mProcNum = 0;
}

template<typename T,typename IDX>
wDispatch<T,IDX>::~wDispatch()
{
	//
}

template<typename T,typename IDX>
bool wDispatch<T,IDX>::Register(string className, IDX ActIdx, struct Func_t vFunc)
{
	vector<struct Func_t> vf;
	typename map<string, vector<struct Func_t> >::iterator mp = mProc.find(className);
	if(mp != mProc.end())
	{
		vf = mp->second;
		mProc.erase(mp);

		typename vector<struct Func_t>::iterator itvf = vf.begin();
		for(; itvf != vf.end() ; itvf++)
		{
			if(itvf->mActIdx == ActIdx)
			{
				itvf = vf.erase(itvf);
				itvf--;
			}
		}
	}
	vf.push_back(vFunc);
	//pair<map<string, vector<struct Func_t> >::iterator, bool> itRet;
	//itRet = 
	mProc.insert(pair<string, vector<struct Func_t> >(className, vf));
	mProcNum++;
	return true;
	//return itRet.second;
}
