//--------------------------------------------------
// 本文件实现了空闲队列数据结构，用于管理服务器实体
//-------------------------------------------------- 

#ifndef _FREE_LIST_H_
#define _FREE_LIST_H_

#include "my_vector.h"
#include <cstdlib>
#include "server_ctrl.h"

namespace my
{
//--------------------------------------------------
// 单个节点的数据结构
// 由于链表实现在数组之上
// 故可以直接以数组中的序号为数组ID
// 适用范围为服务器中单类实体的数量不大于int的范围
//-------------------------------------------------- 
template<class T>  	
class base_node
{
	public:
		base_node()
		{
			if( CServerCtrl::GetSingletonPtr()->mStatus == SERVER_STATUS_INIT )
			{
				initialize();
			}
		}

		~base_node() {}

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

		void clear_list()
		{
			prev_ = -1;
			next_ = -1;
		}

	private:
		T node_;	// 此为服务器中具体需要使用的实体
		int prev_;	// 前一个节点在数组中的位置
		int next_;	// 后一个节点在数组中的位置
};

//--------------------------------------------------
// 空闲链表数据结构的实现
// 所有在链表的中元素都没有被使用
// 被使用之后释放的元素仍然加入到链表中
// 其中链表的指针使用数组的相对ID来代替
// 以方便在服务器崩溃时得到内存中的内容
//-------------------------------------------------- 
template <class T, int count>
class free_list
{
	public:
		free_list()
		{
			if( CServerCtrl::GetSingletonPtr()->mStatus == SERVER_STATUS_INIT)
			{
				initialize();
			}
		}

		~free_list() {}

		// 将所有的元素加入到空闲队列中来
		void initialize()
		{
			// 设置每个数组元素为默认值
			for (int i = 0; i < count; i++) 
			{
				node_list_[i].initialize();
			}

			// 初始化头尾元素
			first_ = 0;
			last_ = count - 1;
			size_ = count;

			node_list_[0].set_next(1);
			node_list_[count - 1].set_prev(count - 2);

			// 初始化中间节点
			for (int i = 1; i < count - 1; i++) 
			{
				node_list_[i].set_prev(i - 1);
				node_list_[i].set_next(i + 1);
			}
		}

		size_t capacity()	
		{
			return (size_t)count;
		}

		// 获取并使用一个空闲节点
		T *get_one_node()
		{
			MY_ASSERT(size_ >= 1 && size_ <= capacity() && "get_one_node failed", return NULL);

#ifdef _DEBUG_
			// 这种情况在理论上是不可能出现的
			MY_ASSERT(first_ >= 0, return NULL);
#endif

			int now_first_num = first_;

			// 如果空闲队列中只有一个节点
			if (size_ == 1) 
			{
				first_ = -1;
				last_ = -1;
			}
			else
			{
				int next_first_num = node_list_[first_].get_next();

#ifdef _DEBUG_
				// 这种情况在理论上是不可能出现的
				MY_ASSERT(next_first_num >= 0, return NULL);
#endif

				node_list_[next_first_num].set_prev(-1);
				first_ = next_first_num;
			}

			size_--;
			node_list_[now_first_num].set_next(-1);
			node_list_[now_first_num].set_prev(-1);
			
			// 节点的初始地址即为实体的初始地址
			return (T *)&(node_list_[now_first_num]);
		}

		// 释放一个节点到空闲链表的具体实现
		void free_one_node(int array_id_)
		{
			MY_ASSERT(array_id_ >= 0 && array_id_ < capacity() && "free_one_node failed", return);

			MY_ASSERT(size_ >= 0 && size_ < capacity() && "free_one_node failed", return);

			MY_ASSERT(node_list_[array_id_].get_next() == -1 && node_list_[array_id_].get_prev() == -1 && first_ != array_id_ && "free_one_node is already free", return);

			// 如果原来已经没有节点
			if (size_ == 0) 
			{
				first_ = array_id_;

				// 释放实体后，数据清空
				node_list_[array_id_].clear_list();
			}
			else
			{
#ifdef _DEBUG_
				// 这种情况在理论上是不可能出现的
				MY_ASSERT(last_ >= 0, return);
#endif
				node_list_[last_].set_next(array_id_);

				// 释放实体后，数据清空
				node_list_[array_id_].clear_list();
				node_list_[array_id_].set_prev(last_);
			}
			
			last_ = array_id_;
			size_++;
		}

		// 释放一个节点到空闲链表
		void free_one_node(T *object_id_)
		{
			unsigned long long distance_address = (unsigned long long)object_id_  - (unsigned long long)node_list_;
			MY_ASSERT(distance_address % sizeof(base_node<T>) == 0 && "free_one_node failed", return);

			free_one_node(distance_address / sizeof(base_node<T>));
		}

		// 获取当前空闲的节点数量
		size_t get_free_size()
		{
			return size_;
		}

		base_node<T> &get_base_node(int array_id_)
		{
			MY_ASSERT(array_id_ >= 0 && array_id_ < capacity() && "get_base_node failed", ;);
			return node_list_[array_id_];
		}

		// 获取一个正在使用的节点数据
		T *get_used_data(int array_id_)
		{
			MY_ASSERT(array_id_ >= 0 && array_id_ < capacity() && "get_base_node failed", return NULL);
			MY_ASSERT(node_list_[array_id_].get_next() == -1 && node_list_[array_id_].get_prev() == -1 && first_ != array_id_ && "get_used_node is already free", return NULL);
			return &(node_list_[array_id_].get_node());
		}

	private:
		base_node<T> node_list_[count];		// 空闲队列所在的数组
		size_t size_;						// 空闲链表中的节点个数
		int first_;							// 空闲队列的第一个节点所在的位置
		int last_;							// 空闲队列的最后一个节点所在的位置
};
};

#endif
