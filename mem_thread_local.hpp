/*************************************************************************
	> File Name: mem_thread_local.hpp
	> Author: zhuchao
	> Mail: zhuchao1995@hotmail.com 
	> Created Time: 六 12/23 22:20:48 2017
 ************************************************************************/
#ifndef MEM_POOL_MEM_THREAD_MANAGER_HPP
#define MEM_POOL_MEM_THREAD_MANAGER_HPP
#include "mem_shard.hpp"
namespace mempool {
template <class T>
class MemThreadLocal {
public:
	typedef boost::shared_ptr<MemShard<T>> MemShardPointer;
	MemThreadLocal();
	virtual ~MemThreadLocal();

	/* recycle one shard from some thread. */
	MemShardPointer recycle();

	/* insert one shard into some thread. */
	void expand(MemShardPointer&);

	/* allocate memory and deallocate memory */
	MemShard<T>::pointer allocate(MemShard<T>::size_type n = 1);
	void deallocate(MemShard<T>::pointer p, MemShard<T>::size_type n = 1);

	/* create an entry and delete an entry */
	template <class... Args> MemShard<T>::pointer new_entry(Args&&... args);
	void delete_entry(MemShard<T>::pointer p);
private:
	struct shard_compare {
		bool operator() (const MemShardPointer& lhs, const MemShardPointer& rhs) const {
			return lhs->size() < rhs->size();
		}
	};
	struct Range {
		uintptr_t start_pos;
		uintptr_t end_pos;
		Range(uintptr_t sp, uintptr_t ep) : start_pos(sp), end_pos(ep) {
		}
	};
	typedef std::map<MemShardPointer, Range, shard_compare> MemShardMap;
	MemShardMap _mem_shards;
	size_t _shard_num;
};
template <class T>
MemThreadLocal<T>::MemThreadLocal() : _shard_num(0) {

}

template <class T>
MemThreadLocal<T>::~MemThreadLocal() {
	MemShardSet().swap(_mem_shards);
}

template <class T>
MemShardPointer MemThreadLocal<T>::recycle() {
	if (_mem_shards.size() <= 0 || !_mem_shards.begin()->empty()) {
		return NULL;
	}
	MemShardMap::iterator iter = _mem_shards.begin();
	MemShardMap::key_type ptr = iter->first;
	_mem_shards.erase(iter);

	return ptr;
}

template <class T>
void MemThreadLocal<T>::expand(MemShardPointer& ptr) {
	_mem_shards.insert(MemShardMap::value_type(ptr, Range(ptr->_start_pos, ptr->_end_pos)));
	++_shard_num;
}

template <class T>
MemShard<T>::pointer MemThreadLocal<T>::allocate(MemShard<T>::size_type n) {
	MemShardSet::iterator mem_shard = _mem_shards.begin();
	/**
	 * if there is no any shard, apply one shard from memory center.
	 */
	if (mem_shard == _mem_shards.end()) {
		MemShardPointer new_mem_shard = _mem_center->apply();
		if (new_mem_shard == NULL) {
			return NULL;
		}
		expand(new_mem_shard);
		mem_shard = _mem_shards.begin();
	}

	/**
	 * if shard is full, recycle some memory from memory center or apply
	 * a new shard.
	 */
	if (mem_shard != _mem_shards.end() && mem_shard->full()) {
		_mem_center->recycle(this);
		if (mem_shard->full()) {
			MemShardPointer new_mem_shard = _mem_center->apply();
			if (new_mem_shard == NULL) {
				return NULL;
			}
			expand(new_mem_shard);
			mem_shard = _mem_shards.begin();
		}
	}
	MemShard<T>::pointer ptr = (*mem_shard)->allocate(n);
	_mem_shards.insert(*mem_shard);
	_mem_shards.erase(mem_shard);

	return ptr;
}

template <class T>
void MemThreadLocal<T>::deallocate(MemShard<T>::pointer p, MemShard<T>::size_type n) {
	MemShardMap::iterator iter = _mem_shards.begin();
	MemShardMap::iterator result = _mem_shards.end();
	uintptr_t pos = reinterpret_cast<uintptr_t>(p);
	for (; iter != _mem_shards.end(); ++iter) {
		if (pos >= iter->second.start_pos && pos < iter->second.end_pos) {
			result = iter;
		}
	}

	if (result == _mem_shards.end()) {
		return;
	}
	result->first->deallocate(p, n);
	_mem_shards.insert(*result);
	_mem_shards.erase(result);
}

template <class T>
template <class... Args>
MemShard<T>::pointer MemThreadLocal<T>::new_entry(Args&&... args) {
	MemShard<T>::pointer p = allocate();
	if (ptr == NULL) {
		return NULL;
	}
	return new(p) MemShard<T>::value_type(std::forward<Args>(args)...);
}
template <class T>
void MemThreadLocal<T>::delete_entry(MemShard<T>::pointer p) {
	if (p != NULL) {
		deallocate(p);
	}
}
}
#endif
