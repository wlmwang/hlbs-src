//--------------------------------------------------
// 本文件实现了一个固定数量的hashmap
// 由于end()指向了一个数组之前的地址
// 故请勿使用it = end(); it--或者it++之类操作
// 这样可能导致未知情况
//-------------------------------------------------- 
#ifndef _MY_HASHMAP_H_
#define _MY_HASHMAP_H_

#include "my_assert.h"
#include <string>
#include <cstdlib>
#include "server_ctrl.h"
#include "my_string.h"

namespace my
{
//--------------------------------------------------
// 存储hashmap所使用的单位数据结构
//-------------------------------------------------- 
template<class T>
class hash_base_node
{
	public:
		hash_base_node()
		{
			if( CServerCtrl::GetSingletonPtr()->mStatus == SERVER_STATUS_INIT )
			{
				initialize();
			}
		}

		~hash_base_node() {}

		int get_prev()
		{
			return prev_;
		}

		void set_prev(int node_id_)
		{
			prev_ = node_id_;
		}

		int get_next()
		{
			return next_;
		}

		void set_next(int node_id_)
		{
			next_ = node_id_;
		}

		T &get_node()
		{
			return node_;
		}

		void set_node(T &node)
		{
			node_ = node;
		}

		void initialize()
		{
			node_ = T();
			prev_ = -1;
			next_ = -1;
		}

		int get_this_num()
		{
			return this_num_;	
		}

		void set_this_num(int num)
		{
			this_num_ = num;
		}

		void initailize()
		{
			this_num_ = -1;
			node_ = T();
			prev_ = -1;
			next_ = -1;
		}

	private:
		int this_num_;		// 当前节点在数组中的位置
		T node_;			// 此为服务器中具体需要使用的实体
		int prev_;			// 前一个节点在数组中的位置
		int next_;			// 后一个节点在数组中的位置
};

//--------------------------------------------------
// hashmap数据结构的迭代器实现
// 它是一个指向T的迭代器
//-------------------------------------------------- 
template<class T>
class hash_base_node_iterator
{
	public:
		typedef hash_base_node_iterator<T> self;

		hash_base_node_iterator()
		{
			ptr_ = NULL;
		}

		hash_base_node_iterator(hash_base_node<T> *ptr)
		{
			ptr_ = ptr;
		}

		// 左操作++
		self& operator++()
		{
			ptr_ = (hash_base_node<T> *)next();
			return *this;
		}

		// 右操作++
		self operator ++(int)
		{
			self tmp(*this);
			++*this;
			return tmp;
		}

		// 左操作--
		self& operator--()
		{
			ptr_ = (hash_base_node<T> *)prev();
			return *this;
		}

		// 右操作--
		self operator --(int)
		{
			self tmp(*this);
			--*this;
			return tmp;
		}

		bool operator == (const self &x) const
		{
			return ptr_ == x.ptr_;
		}

		bool operator != (const self &x) const
		{
			return !(*this == x);
		}

		self &operator = (const self &x)
		{
			if (this != &x)
			{
				ptr_ = x.ptr_;
			}
			return *this;
		}

		T &operator* () const
		{
			return ptr_->get_node();
		}

		T *operator->() const
		{
			return &(ptr_->get_node());
		}

		hash_base_node<T> *get_ptr() const
		{
			return ptr_;
		}

	private:
		hash_base_node<T> *ptr_;

		// 使用偏移量来获取下一个节点
		void *next()
		{
			MY_ASSERT(ptr_ != NULL, return NULL);

			int this_num = ptr_->get_this_num();
			int next_num = ptr_->get_next();

			return (void *)((long long)ptr_ + sizeof(hash_base_node<T>) * (next_num - this_num));
		}

		// 使用偏移量来获取上一个节点
		void *prev()
		{
			MY_ASSERT(ptr_ != NULL, return NULL);

			int this_num = ptr_->get_this_num();
			int prev_num =  ptr_->get_prev();

			return (void *)((long long)ptr_ + sizeof(hash_base_node<T>) * (prev_num - this_num));
		}
};

//--------------------------------------------------
// 迭代器常量版
//-------------------------------------------------- 
template<class T>
class hash_base_node_const_iterator
{
	public:
		typedef hash_base_node_iterator<T> self;

		hash_base_node_const_iterator()
		{
			ptr_ = NULL;
		}

		hash_base_node_const_iterator(hash_base_node<T> *ptr)
		{
			ptr_ = ptr;
		}

		// 左操作++
		self& operator++()
		{
			ptr_ = (hash_base_node<T> *)next();
			return *this;
		}

		// 右操作++
		self operator ++(int)
		{
			self tmp(*this);
			++*this;
			return tmp;
		}

		// 左操作--
		self& operator--()
		{
			ptr_ = (hash_base_node<T> *)prev();
			return *this;
		}

		// 右操作--
		self operator --(int)
		{
			self tmp(*this);
			--*this;
			return tmp;
		}

		bool operator == (const self &x) const
		{
			return ptr_ == x.ptr_;
		}

		bool operator != (const self &x) const
		{
			return !(*this == x);
		}

		T &operator* () const
		{
			return ptr_->get_node();
		}

		T *operator->() const
		{
			return ptr_->get_node();
		}

		hash_base_node<T> *get_ptr() const
		{
			return ptr_;
		}

	private:
		// 使用偏移量来获取下一个节点
		void *next()
		{
			MY_ASSERT(ptr_ != NULL, return NULL);

			int this_num = ptr_->get_this_num();
			int next_num = ptr_->get_next();

			return (void *)((long long)ptr_ + sizeof(hash_base_node<T>) * (next_num - this_num));
		}

		// 使用偏移量来获取上一个节点
		void *prev()
		{
			MY_ASSERT(ptr_ != NULL, return NULL);

			int this_num = ptr_->get_this_num();
			int prev_num = ptr_->get_prev();

			return (void *)((long long)ptr_ + sizeof(hash_base_node<T>) * (prev_num - this_num));
		}

		hash_base_node<T> *ptr_;
};

template <typename T1, typename T2, size_t count>
class hash_map 
{
	public:
		typedef std::pair<T1, T2> value_type;

		// 每个节点的结构体
		struct node_value
		{
			T1 first;
			T2 second;
		};

		typedef struct node_value node_value;
		
		typedef hash_base_node_iterator<node_value> iterator;
		typedef hash_base_node_const_iterator<node_value> const_iterator;
		typedef node_value &reference;
		typedef const node_value &const_reference;
		typedef std::size_t size_type;
		typedef std::pair<iterator, bool> pair_ret;

		hash_map()
		{
			if( CServerCtrl::GetSingletonPtr()->mStatus == SERVER_STATUS_INIT )
			{
				initailize();
			}
		}

		~hash_map() {}

		// 所有hash生成函数的源头
		size_t hash_string(const char* s)
		{
			unsigned int hash = 5381;
			while (*s)
			{
				hash += (hash << 5) + (*s ++);
			}
			return size_t(hash & 0x7FFFFFFF);
		}

		size_t hash(char *v) const
		{
			return hash_string(v);
		}

		size_t hash(std::string &s) const
		{
			return hash_string(s.c_str());
		}

		size_t hash(const my::string<32> &s)
		{
			return hash_string(s.c_str());
		}

		size_t hash(unsigned int num)
		{
			return (size_t)num;
		}

		size_t hash(unsigned long long num)
		{
			return (size_t)num;
		}

		size_t hash(int num)
		{
			return (size_t)num;
		}

		size_t hash(long long num)
		{
			return (size_t)num;
		}

		// 初始化过程
		void initailize()
		{
			// 初始化所有数据
			for (int i = 0; i < count; i++) 
			{
				// 清空数据存储数组
				node_list_[i].initailize();
				bucket_[i].first_ = -1;
				bucket_[i].last_ = -1;
			}

			// 初始化头尾元素
			size_ = count;

			node_list_[0].set_next(1);
			node_list_[count - 1].set_prev(count - 2);

			// 初始化中间节点
			for (int i = 1; i < count - 1; i++) 
			{
				node_list_[i].set_prev(i - 1);
				node_list_[i].set_next(i + 1);
			}

			// 清空被使用队列
			used_.first_ = -1;
			used_.last_ = -1;

			// 设置空闲队列	
			free_.first_ = 0;
			free_.last_ = count - 1;

			// 初始化当前节点位置
			for (int i = 0; i < count; i++)
			{
				node_list_[i].set_this_num(i);
			}
		}

		iterator begin()
		{
			iterator it(&(node_list_[used_.first_]));
			return it;
		}

		const_iterator begin() const
		{
			const_iterator it(&(node_list_[used_.first_]));
			return it;
		}

		// 这个位置实际上是越界的
		// 由于我们使用一些的别的变量填充了这一地址
		// 避免了不能访问这个地址这种情况
		// 所以把它看成是迭代器的终结地址
		iterator end()
		{
			iterator it(&(node_list_[-1]));
			return it;
		}

		const_iterator end() const
		{
			const_iterator it(&(node_list_[-1]));
			return it;
		}

		size_type max_size() const
		{
			return count;
		}

		size_type size() const
		{
			return count - size_;
		}

		bool empty() const
		{
			return size_ == count;
		}

		pair_ret insert(const value_type &value)
		{
			// 设置返回值
			pair_ret ret;
			ret.first = iterator();
			ret.second = false;

			// 确认hashmap是否还有空间
			MY_ASSERT(size_ > 0 && "hashmap is full", return ret);

			// 生成hash值
			size_t hash_value = hash(value.first);
			// 由hash值生成相应的数组位置
			int pos = ((int)hash_value) % count;
			if (pos < 0) 
			{
				pos += count;
			}
			
			// 如果对应的桶中没有节点，则直接插入
			int node_pos = bucket_[pos].first_;
			if (node_pos != -1)
			{
				// 如果桶中已经有节点，则遍历节点，查看是否有相同的key值
				int last_pos = bucket_[pos].last_;
				for (; node_pos != last_pos; node_pos = node_list_[node_pos].get_next()) 
				{
					// 如果已经有相同的key值
					if (node_list_[node_pos].get_node().first == value.first) 
					{
						ret.first = iterator(&(node_list_[node_pos]));
						return ret;
					}
				}

				// 桶中的最后一个节点
				if (node_list_[node_pos].get_node().first == value.first) 
				{
					ret.first = iterator(&(node_list_[node_pos]));
					return ret;
				}
			}

			// 没有这个key值，在used链表中插入这个值
			int first_free_pos = free_.first_;
			ret.first = iterator(&(node_list_[first_free_pos]));

#ifdef _DEBUG_
			// 绝不可能发生的情况
			MY_ASSERT(first_free_pos != -1, return ret);
#endif

			// 对节点赋值
			node_list_[first_free_pos].get_node().first = value.first;
			node_list_[first_free_pos].get_node().second = value.second;
			int next_first_free_pos = node_list_[first_free_pos].get_next();

			// 将它插入桶链表
			// 如果桶链表中没有节点
			if(node_pos == -1)
			{
				bucket_[pos].first_ = first_free_pos;
				bucket_[pos].last_ = first_free_pos;
				// 它是整个hashmap中的首个节点
				if (size() == 0) 
				{
#ifdef _DEBUG_
					// 绝不可能发生的情况
					MY_ASSERT(used_.first_ == -1 && used_.last_ == -1, return ret);
#endif
					used_.first_ = first_free_pos;
					used_.last_ = first_free_pos;
					node_list_[first_free_pos].set_prev(-1);
					node_list_[first_free_pos].set_next(-1);
				}
				else
				{
					// 添加到used_队列的第一个位置
					// 并且不是used_队列的最后一个
#ifdef _DEBUG_
					// 绝不可能发生的情况
					MY_ASSERT(used_.first_ != -1, return ret);
#endif

					// 添加到used_队列之中
					node_list_[used_.first_].set_prev(first_free_pos);
					node_list_[first_free_pos].set_next(used_.first_);
					node_list_[first_free_pos].set_prev(-1);
					used_.first_ = first_free_pos;
				}
			}
			else
			{
				int first_bucket_node = bucket_[pos].first_;
#ifdef _DEBUG_
				// 绝不可能发生的情况
				MY_ASSERT(first_bucket_node != -1, return ret);
#endif
				// 添加到桶中链表的第一个位置
				bucket_[pos].first_ = first_free_pos;

				node_list_[first_free_pos].set_prev(node_list_[first_bucket_node].get_prev());
				node_list_[first_free_pos].set_next(first_bucket_node);
				node_list_[first_bucket_node].set_prev(first_free_pos);

				// 如果是原来的桶中的第一个节点是
				// 整个used_队列上的头节点
				if (first_bucket_node == used_.first_) 
				{
					used_.first_ = first_free_pos;
				}
			}

			// 从free_队列上删除
			// 这是唯一一个节点了
			if (next_first_free_pos == -1) 
			{
#ifdef _DEBUG_
				// 绝不可能发生的情况
				MY_ASSERT(size_ == 1, return ret);
#endif
				free_.first_ = -1;
				free_.last_ = -1;
			}
			else
			{
				free_.first_ = next_first_free_pos;
				node_list_[next_first_free_pos].set_prev(-1);
			}

			size_--;
			ret.second = true;
			return ret;
		}

		iterator find(const T1& key)
		{
			iterator ret = end();

			// 如果一个节点都没有，直接返回
			MY_ASSERT(size() != 0, return ret);

			// 生成hash值
			size_t hash_value = hash(key);
			// 由hash值生成相应的数组位置
			int pos = ((int)hash_value) % count;
			if (pos < 0) 
			{
				pos += count;
			}

			int node_pos = bucket_[pos].first_;

			// 如果一个节点都没有
			if (node_pos == -1) 
			{
				return ret;
			}

			int last_pos = bucket_[pos].last_;

			// 遍历桶上的所有节点
			for (; node_pos != last_pos; node_pos = node_list_[node_pos].get_next())
			{
				// 如果找到相同的键值
				if (node_list_[node_pos].get_node().first == key)
				{
					ret = iterator(&(node_list_[node_pos]));
					return ret;
				}
			}

			// 桶中的最后一个节点
			if (node_list_[node_pos].get_node().first == key)
			{
				ret = iterator(&(node_list_[node_pos]));
				return ret;
			}

			return ret;
		}

		void erase(iterator it)
		{
			erase(it, true);
		}

		size_type erase(const T1 &key)
		{
			// 已经遍历过，无需再次遍历
			erase(find(key), false);
			return size();
		}

		void clear()
		{
			initailize();
		}

	private:
		void erase(iterator it, bool if_find)
		{
			// 主要做四件事
			// 1. 从桶中删除
			// 2. 从used_删除
			// 3. 初始化数据
			// 4. 加入到free_中
			// 如果为终止迭代器，则直接退出
			if (it == end())
			{
				return;
			}

			T1 key = it->first;
			// 生成hash值
			size_t hash_value = hash(key);
			// 由hash值生成相应的数组位置
			int pos = ((int)hash_value) % count;
			if (pos < 0) 
			{
				pos += count;
			}

			// 桶指针
			bucket_type *bucket_ptr = &(bucket_[pos]);
			// 数组指针
			hash_base_node<node_value> *array_ptr = it.get_ptr();
			// 该节点在数组中的位置
			int array_pos = array_ptr->get_this_num();
			MY_ASSERT(array_pos >= 0 && array_pos < count, return);

			if(if_find)
			{
				// 在桶内搜索，防止其不在used_队列中
				int find_flag = 0;
				int node_pos;
				for (node_pos = bucket_ptr->first_; node_pos != bucket_ptr->last_; node_pos = node_list_[node_pos].get_next()) 
				{
					if (node_list_[node_pos].get_node().first == key)
					{
						find_flag = 1;
						break;
					}
				}

				// 最后一个元素
				if (node_pos != -1 && 
						node_pos == bucket_ptr->last_ &&
						node_list_[node_pos].get_node().first == key) 
				{
					find_flag = 1;
				}

				// 没找到
				if (find_flag = 0)
				{
#ifdef _DEBUG_
					MY_ASSERT("erase: iterator not used", return);
#endif
					return;
				}
			}

			// 桶中只有一个节点
			if (bucket_ptr->first_ == array_pos && bucket_ptr->last_ == array_pos) 
			{
				bucket_ptr->first_ = -1;
				bucket_ptr->last_ = -1;
			}
			// 如果它是桶中的头结点
			else if(bucket_ptr->first_ == array_pos)
			{
				bucket_ptr->first_ = array_ptr->get_next();
			}
			// 如果它是桶中的尾结点
			else if(bucket_ptr->last_ == array_pos)
			{
				bucket_ptr->last_ = array_ptr->get_prev();
			}
			else
			{
				// 什么都不做
			}

			// 清理used链表
			if (used_.first_ == array_pos && used_.last_ == array_pos) 
			{
				used_.first_ = -1;
				used_.last_ = -1;
			}
			else if (used_.first_ == array_pos)
			{
				used_.first_ = array_ptr->get_next();
			}
			else if (used_.last_ == array_pos)
			{
				used_.last_ = array_ptr->get_prev();
			}
			else
			{
				// 什么事都别干
			}

			// 清理used_里的链表
			if (array_ptr->get_next() != -1)
			{
				node_list_[array_ptr->get_next()].set_prev(array_ptr->get_prev());
			}

			if (array_ptr->get_prev() != -1)
			{
				node_list_[array_ptr->get_prev()].set_next(array_ptr->get_next());
			}
			
			// 重新初始化数据
			array_ptr->initailize();
			array_ptr->set_this_num(array_pos);

			// 加入到free_之尾节点处
			if (free_.last_ == -1) 
			{
				free_.first_ = array_pos;
				free_.last_ = array_pos;
			}
			else
			{
				node_list_[free_.last_].set_next(array_pos);
				array_ptr->set_prev(free_.last_);
				free_.last_ = array_pos;
			}

			size_++;
		}

		// 对应的桶的结构体
		typedef struct
		{
			int first_;
			int last_;
		}bucket_type;

		bucket_type bucket_[count];					// 由key值的存入的对应的桶结构
		hash_base_node<node_value> node_list_[count];	// hashmap具体的数据存储结构
		size_t size_;								// node_list_中还没有被使用的个数, 即其中链表的数量
		bucket_type free_;							// 存储所有空闲列表的头尾位置的结构
		bucket_type used_;							// 存储所有被使用的节点列表的头尾位置的结构
};
};

#endif
