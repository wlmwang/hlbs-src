
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_MASTER_H_
#define _W_MASTER_H_

#include <map>
#include <vector>

#include "wType.h"
#include "wLog.h"
#include "wNoncopyable.h"
#include "wProcess.h"
#include "wFile.h"

template <typename T>
class wMaster : public wSingleton<T>
{
	public:
		wMaster();
		~wMaster();

		void MasterStart();

		void SingleStart();

		int CreatePidFile()
		{
		    size_t len;
		    u_char vPid[NGX_INT64_LEN + 2];

		    mPidFile.Open(PID_PATH, O_RDWR| O_CREAT);

		    if (mPidFile.FD() == -1) 
		    {
		    	return -1;
		    }

	        len = snprintf(vPid, NGX_INT64_LEN + 2, "%P%N", getpid()) - vPid;

	        if (mPidFile.Write(vPid, len, 0) == -1) 
	        {
	            return -1;
	        }

	        mPidFile.Close();

		    return 0;
		}

		void DeletePidFile()
		{
		    if (mPidFile.Unlink() == -1) 
		    {
		    	LOG_ERROR(ELOG_KEY, "unlink \"%s\" failed", mPidFile.c_str());
		    }
		}

	private:
		wFile mPidFile;
		map<int, wWorker* > mWorkerPool;
};