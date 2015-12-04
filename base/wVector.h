
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

/**
 *  构建了一个缩减版的固定长度的vector容器
 */

#ifndef _W_VECTOR_H_
#define _W_VECTOR_H_

#include <cstdlib>
#include <algorithm>

#include "wAssert.h"

namespace W
{
	template <typename T, size_t count>
	class vector
	{
		public:
			typedef T *iterator;
			typedef const T *const_iterator;
			typedef T &reference;
			typedef const T &const_reference;
			typedef std::size_t size_type;

			vector()
			{
				initialize();
			}

			~vector() {}

			iterator begin()
			{
				return elems_;
			}

			const_iterator begin() const
			{
				return elems_;
			}

			iterator end()
			{
				return elems_ + size_;
			}

			const_iterator end() const
			{
				return elems_ + size_;
			}

			reference operator[] (size_type t)
			{
				MY_ASSERT(t < size_ && "[] out of range", ;);
				return elems_[t];
			}

			const_reference operator[] (size_type t) const
			{
				MY_ASSERT(t < size_ && "[] out of range", ;);
				return elems_[t];
			}

			reference front()
			{
				return elems_[0];
			}

			const_reference front() const
			{
				return elems_[0];
			}

			reference back()
			{
				return elems_[size_ - 1];
			}

			const_reference back() const
			{
				return elems_[size_ - 1];
			}

			size_type size()
			{
				return size_;
			}

			bool empty()
			{
				return size_ == 0;
			}

			size_type max_size()
			{
				return count;
			}

			iterator push_back(const T & t)
			{
				MY_ASSERT(size_ < capacity() && "push_back container is full", return elems_ + size_);
				elems_[size_++] = t;
				return elems_ + size_;
			}

			iterator erase(iterator pos)
			{
				if (pos < end()) 
				{
					if (pos+1 != end()) 
					{
						std::copy(pos+1, end(), pos);
					}
					size_--;
				}
				return pos;
			}

			iterator erase(iterator first, iterator last)
			{
				std::copy(last, end(), first);
				size_ -= (last - first);
				return first;
			}

			void pop_back()
			{
				size_--;	
			}

			size_t capacity()
			{
				return count;
			}

			void clear()
			{
				size_ = 0;
			}

			void initialize()
			{
				clear();	
			}

		private:
			T elems_[count];
			size_t size_;
	};
};

#endif
