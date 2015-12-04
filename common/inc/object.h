//--------------------------------------------------
// 这是所有实体类的父类
//-------------------------------------------------- 
#ifndef _OBJECT_H_
#define _OBJECT_H_

class CObjectPool;

class CObject
{
	public:
		CObject() {}

		virtual ~CObject() {}

		unsigned long long GetObjectID()
		{
			return mObjectID;
		}

		// 使用CObjectPool来操作实体ID，而不允许其他类操作
		friend class CObjectPool;

		virtual void Initialize() = 0;

	private:
		// 该ID的前32位是实体的类型ID
		// 后32位为实体的实体ID
		unsigned long long mObjectID;
};

#endif
