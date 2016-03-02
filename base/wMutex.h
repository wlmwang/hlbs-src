
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_MUTEX_H_
#define _W_MUTEX_H_

#include <pthread.h>

#include "wType.h"
#include "wNoncopyable.h"
#include "wCond.h"

//互斥锁
class wMutex : private wNoncopyable
{
	friend class wCond;
	public:
		/**
		 *  PTHREAD_MUTEX_FAST_NP: 普通锁，同一线程可重复加锁，解锁一次释放锁。不提供死锁检测，尝试重新锁定互斥锁会导致死锁
		 *  PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP：同一线程可重复加锁，解锁同样次数才可释放锁
		 *  PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP：同一线程不能重复加锁，加上的锁只能由本线程解锁
		 *  
		 *  PTHREAD_PROCESS_PRIVATE：单进程锁，默认设置
		 *  PTHREAD_PROCESS_SHARED：进程间，共享锁（锁变量需进程间共享。如在共享内存中）
		 */
		wMutex(int kind = PTHREAD_MUTEX_FAST_NP, int pshared = PTHREAD_PROCESS_PRIVATE) 
		{
			pthread_mutexattr_init(&mAttr);
			pthread_mutexattr_settype(&mAttr, kind);
			pthread_mutexattr_setpshared(&mAttr, pshared);
			if (pthread_mutex_init(&mMutex, &mAttr) < 0)
			{
				LOG_ERROR(ELOG_KEY, "pthread_mutex_init failed: %s", strerror(errno));
				exit(-1);
			}
		}
		
		~wMutex()
		{
			pthread_mutexattr_destroy(&mAttr);
			pthread_mutex_destroy(&mMutex);
		}
		
		/**
		 *  阻塞获取锁
		 *  0		成功
		 *  EINVAL	锁不合法，mMutex 未被初始化
		 *  EDEADLK	重复加锁错误
		 * 	...
		 */
		inline int Lock()
		{
			return pthread_mutex_lock(&mMutex);
		}
		
		/**
		 *  非阻塞获取锁
		 *  0		成功
		 *  EBUSY	锁正在使用
		 *  EINVAL	锁不合法，mMutex 未被初始化
		 *  EAGAIN	Mutex的lock count(锁数量)已经超过 递归索的最大值，无法再获得该mutex锁
		 *  EDEADLK	重复加锁错误
		 */
		inline int TryLock()
		{
			return pthread_mutex_trylock(&mMutex);
		}
		
		/**
		 *  释放锁
		 *  0		成功
		 *  EPERM	当前线程不是该 mMutex 锁的拥有者
		 */
		inline int Unlock()
		{
			return pthread_mutex_unlock(&mMutex);
		}
		
	protected:
		pthread_mutex_t mMutex;
		pthread_mutexattr_t mAttr;
};

//读写锁
class RWLock : private wNoncopyable
{
	public:
		RWMutex(int pshared = PTHREAD_PROCESS_PRIVATE) 
		{
			pthread_rwlockattr_init(&mAttr);
			pthread_rwlockattr_setpshared(&mAttr, pshared);
			pthread_rwlock_init(&mRWlock, &mAttr);
		}
		
		~RWMutex()
		{
			pthread_rwlockattr_destroy(&mAttr);
			pthread_rwlock_destroy(&mRWlock);
		}
		
		/**
		 *  阻塞获取读锁
		 *  0		成功
		 *  EINVAL	锁不合法，mRWlock 未被初始化
		 *  EAGAIN	Mutex的lock count(锁数量)已经超过 递归索的最大值，无法再获得该mutex锁
		 *  EDEADLK	重复加锁错误
		 * 	...
		 */
		inline int RDLock()
		{
			if(pthread_rwlock_rdlock(&mRWlock) == -1)	//可能被锁打断
			{
				mErrno = errno;
				return -1;
			}
			return 0;
		}

		/**
		 *  阻塞获取写锁
		 *  0		成功
		 *  EINVAL	锁不合法，mRWlock 未被初始化
		 *  EAGAIN	Mutex的lock count(锁数量)已经超过 递归索的最大值，无法再获得该mutex锁
		 *  EDEADLK	重复加锁错误
		 * 	...
		 */
		inline int RDLock()
		{
			if(pthread_rwlock_wrlock(&mRWlock) == -1)	//可能被锁打断
			{
				mErrno = errno;
				return -1;
			}
			return 0;
		}
		
		/**
		 *  非阻塞获取读锁
		 *  0		成功
		 *  EBUSY	锁正在使用
		 *  EINVAL	锁不合法，mRWlock 未被初始化
		 *  EAGAIN	Mutex的lock count(锁数量)已经超过 递归索的最大值，无法再获得该mutex锁
		 *  EDEADLK	重复加锁错误
		 */
		inline int TryRDLock()
		{
			return pthread_rwlock_tryrdlock(&mMutex);
		}
		
		/**
		 *  非阻塞获取写锁
		 *  0		成功
		 *  EBUSY	锁正在使用
		 *  EINVAL	锁不合法，mRWlock 未被初始化
		 *  EAGAIN	Mutex的lock count(锁数量)已经超过 递归索的最大值，无法再获得该mutex锁
		 *  EDEADLK	重复加锁错误
		 */
		inline int TryWRLock()
		{
			return pthread_rwlock_trywrlock(&mMutex);
		}
		
		/**
		 *  释放锁
		 *  0		成功
		 *  EPERM	当前线程不是该 mRWlock 锁的拥有者
		 */		
		inline int Unlock()
		{
			return pthread_rwlock_unlock(&mRWlock);
		}
		
	protected:
		pthread_rwlock_t mRWlock;
		pthread_rwlockattr_t mAttr;
		int mErrno;
};

#endif