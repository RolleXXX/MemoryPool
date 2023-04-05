#pragma once

#ifndef MEMPOOL_MG
#define MEMPOOL_MG
#include "MemoryAlloc.h"

template <class T>
class MemoryPool {
public:
	MemoryPool() {
		vec.emplace_back(thread_pool(0));//主线程是0序号
	}

	void addVec(int id) {
		vec.emplace_back(id);
	}

	template<class U, class ...Args>
	std::shared_ptr<T> make_shared(int id = 0, Args&&... args) {
		if (id == 0) {
			return vec[0].pool.make_shared<U>(std::forward<Args>(args)...);
		}
		for (int i = 1; i < vec.size(); i++) {
			if (id == i) {
				return vec[i].pool.make_shared<U>(std::forward<Args>(args)...);

			}
		}
	}

private:
	struct thread_pool
	{
		int m_id;
		MemoryAlloc<T> pool;

		thread_pool(int id) :m_id(id), pool() {

		}
	};

	std::vector<thread_pool>vec;

};

#endif;





