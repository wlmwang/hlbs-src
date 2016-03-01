
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#include "wFile.h"

void wFile::Initialize()
{
	mFileName = "";
	mFD = 0;
	mOffset = 0;
	memset(&mInfo, 0, sizeof(struct stat));
}

int wFile::Open(const char *pFilename, int flags, mode_t mode)
{
	mFileName = pFilename;
	mFD = open(pFilename, flags, mode);
	if (mFD == -1)
	{
		mErrno = errno;
		return -1;
	}
	fstat(mFD, &mInfo);
	return mFD;
}

int wFile::Close()
{
	if(close(mFD) == -1)
	{
		mErrno = errno;
		return -1;
	}
	return 0;
}

int wFile::Unlink()
{
	if(unlink(mFileName.c_str()) == -1)
	{
		mErrno = errno;
		return -1;
	}
	return 0;
}

off_t wFile::Lseek(off_t offset, int whence)
{
	mOffset = lseek(mFD, offset, whence);
	return mOffset;
}

ssize_t wFile::Read(char *pBuf, size_t nbyte, off_t offset)
{
	ssize_t  n;
	n = pread(mFD, pBuf, nbyte, offset);
	if (n == -1)
	{
		mErrno = errno;
		LOG_ERROR(ELOG_KEY, "pread() \"%s\" failed", mFileName.c_str());
		return -1;
	}

	mOffset += n;
	return n;
}

ssize_t wFile::Write(const char *pBuf, size_t nbytes, off_t offset)
{
    ssize_t n, written;

    written = 0;
    while (true) 
    {
        n = pwrite(mFD, pBuf + written, nbytes, offset);

        if (n == -1) 
        {
			mErrno = errno;

            if (mErrno == EINTR) 
            {
                LOG_DEBUG(ELOG_KEY, "pwrite() was interrupted");
                continue;
            }

            LOG_ERROR(ELOG_KEY, "pwrite() \"%s\" failed", mFileName.c_str());
            return -1;
        }

        mOffset += n;
        written += n;

        if ((size_t) n == nbytes) 
        {
            return written;
        }

        offset += n;
        nbytes -= n;
    }
}
		