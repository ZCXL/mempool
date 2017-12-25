/*************************************************************************
	> File Name: mem_center.hpp
	> Author: zhuchao
	> Mail: zhuchao1995@hotmail.com 
	> Created Time: 日 12/24 12:49:45 2017
 ************************************************************************/
#ifndef MEM_POOL_MEM_CENTER_HPP
#define MEM_POOL_MEM_CENTER_HPP
#include "common.h"
#include "util.h"

namespace mempool {
template <class T>
class MemThreadLocal;
template <class T>
class MemCenter {
public:
	typedef boost::shared_ptr<MemShard<T>> MemShardPointer;
	MemCenter(size_t obj_num = 1 << 10, size_t alignment = 4);
	virtual ~MemCenter();
	MemShardPointer apply();
	void recycle(MemThreadLocal<T>*);
private:
	typedef std::list<MemShardPointer> ThreadShardList;
	typedef std::unordered_map<pthread_t, ThreadShardList> ThreadShardsMap;
	ThreadShardsMap _thread_shards_map;
	SpinLock _lock;

	size_t _alignment;
	size_t _obj_num;
};
template <class T>
MemCenter<T>::MemCenter(size_t obj_num, size_t alignment) :
	_obj_num(obj_num), _alignment(alignment) {
}

template <class T>
MemCenter<T>::~MemCenter() {
}

template <class T>
MemCenter<T>::MemShardPointer MemCenter<T>::apply() {
	_lock.lock();
	pthread_t pid = pthread_self();
	ThreadShardsMap::iterator iter = _thread_shards_map.find(pid);
	if (iter == _thread_shards_map.end()) {
		std::pair<ThreadShardsMap::value_type, bool> result =
					_thread_shards_map.insert(ThreadShardsMap::value_type(pid, ThreadShardList()));
		if (!result->second) {
			_lock.unlock();
			return NULL;
		}
		iter = result->first;
	}
	MemShardPointer mem_shard(new MemShard(_obj_num, _alignment));
	(iter->second).insert(mem_shard);
	_lock.unlock();

	return mem_shard;
}

template <class T>
void MemCenter<T>::recycle(MemThreadLocal<T>*) {
}
}
#endif
