//--------------------------------------------------
// 本文件用于实现单键，该类是所有单键类的父类
//-------------------------------------------------- 

#ifndef _SINGLETON_H_
#define _SINGLETON_H_

#include <cstdlib>

template <class T>
class CSingleton 
{
public:
	CSingleton()
	{
		mSingletonPtr = static_cast<T *>(this);
	}

	virtual ~CSingleton() {}

	static T *GetSingletonPtr()
	{
		return mSingletonPtr;
	}

	static T *mSingletonPtr;
};

#endif
