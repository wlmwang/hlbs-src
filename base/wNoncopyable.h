
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_NONCOPYABLE_h_
#define _W_NONCOPYABLE_h_

class wNoncopyable
{

	protected:

		wNoncopyable() {};
		
		~wNoncopyable() {};

	private:

		wNoncopyable(const wNoncopyable&);
		
		const wNoncopyable & operator= (const wNoncopyable &);

};

#endif

