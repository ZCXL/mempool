/*************************************************************************
	> File Name: mem_shard.hpp
	> Author: zhuchao
	> Mail: zhuchao1995@hotmail.com 
	> Created Time: 六 12/23 21:38:49 2017
 ************************************************************************/
#ifndef MEM_POOL_MEM_SHARD_HPP
#define MEM_POOL_MEM_SHARD_HPP
#include <boost/thread.hpp>
#include "common.h"
namespace mempool {
template <class T>
class MemThreadManager;
template <class T>
class MemShard {
public:
	friend class MemThreadManager<T>;
	/* Member types */
	typedef T value_type;
	typedef T* pointer;
	typedef T& reference;
	typedef const T* const_pointer;
	typedef const T& const_reference;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	/* Construct functions */
	MemShard(size_type size, size_t alignment = 4);
	MemShard(const MemShard& memshard);
	MemShard(MemShard&& memshard);

	/* Deconstruct function */
	virtual ~MemShard();

	/* allocate memory and deallocate memory */
	pointer allocate(size_type n = 1);
	void deallocate(pointer p, size_type n = 1);

	/* create an entry and delete an entry */
	template <class... Args> pointer new_entry(Args&&... args);
	void delete_entry(pointer p);

	size_t capacity() const;
	size_t size() const;
	bool empty() const;
	bool full() const;
private:
	size_type _max_size;
	size_type _num_used;
	typedef std::set<uintptr_t, pos_compare> MemSet;
	MemSet _unused_set;
	MemSet _used_set;
	bool _has_error;
	void* _start_pointer;
	size_t _mem_size;
	size_t _alignment;
	size_t _aligned_size;
	uintptr_t _start_pos;
	uintptr_t _end_pos;
};
template <class T>
MemShard<T>::MemShard(MemShard<T>::size_type size, size_t alignment) : _max_size(size), _num_used(0),
													_start_pointer(NULL), _has_error(false),
													_start_pos(0), _end_pos(0),
													_mem_size(0), _alignment(alignment) {
	_aligned_size = (((sizeof(value_type) + _alignment - 1)) / _alignment) * _alignment;
	_mem_size =  aligned_size * _max_size;
	_start_pointer = sys_alloc(_mem_size, &_mem_size, alignment);
	if (_start_pointer == NULL) {
		_has_error = true;
	} else {
		_start_pos = reinterpret_cast<uintptr_t>(_start_pointer);
		_end_pos = reinterpret_cast<uintptr_t>(_start_pointer + _mem_size);
		for (int pos = _start_pos; pos < _end_pos;) {
			_unused_set.insert(pos);
			i += aligned_size;
		}
	}
}

template <class T>
MemShard<T>::MemShard(const MemShard& memshard) : MemShard(memshard._max_size) {

}

template <class T>
MemShard<T>::MemShard(MemShard&& memshard) {
	_max_size = memshard._max_size;
	_num_unsed = memshard._num_unsed;
	_unused_set.insert(memshard._unused_set.begin(), memshard.unused_set.end());
	_used_set.insert(memshard._used_set.begin(), memshard._used_set.end());
	_has_error = memshard._has_error;
	_start_pointer = memshard._start_pointer;
	_start_pos = memshard._start_pos;
	_end_pos = memshard._end_pos;
	_mem_size = mem_shard._mem_size;
	_alignment = mem_shard._alignment;
	_aligned_size = mem_shard._aligned_size;
}

template <class T>
MemShard<T>::~MemShard() {
	if (_start_pointer != NULL) {
		free(_start_pointer);
	}
}

template <class T>
MemShard<T>::pointer MemShard<T>::allocate(MemShard<T>::size_type n) {
	MemSet::iterator iter = _unused_set.begin();
	MemSet::iterator start_iter = iter;
	MemSet::iterator end_iter = iter + 1;
	size_type count = 0;

	/* Search a blcok memory which size is n * sizeof(T) */
	for (; iter != _unused_set.end(); ++iter) {
		if ((*iter - *start_iter) / _aligned_size != count) {
			start_iter = iter;
			end_iter = iter + 1;
			count = 0;
		} else {
			++count;
			++end_iter;
		}

		if (count == n) {
			break;
		}
	}

	/* Check wheather this is a cluster memory or not */
	if (count < n) {
		return NULL;
	}

	uintptr_t start_pos = *start_iter;
	for (iter = start_iter; iter != end_iter; ++iter) {
		_used_set.insert(*iter);
		_unused_set.erase(iter);
	}
	_num_used += n;

	return reinterpret_cast<pointer>(start_pos);
}

template <class T>
void MemShard<T>::deallocate(MemShard<T>::pointer p, MemShard<T>::size_type n) {
	uintptr_t pos = reinterpret_cast<uintptr_t>(p);
	if (pos < _start_pos || ps > _end_pos) {
		return;
	}
	MemSet::iterator iter = _used_set.find(pos);
	if (iter == _used_set.end()) {
		return;
	}
	for (int i = 0; i < n && iter != _used_set.end(); ++i, ++iter) {
		_unused_set.insert(*iter);
		_used_set.erase(iter);
		--_num_used;
	}
}

template <class T>
template <class... Args>
MemShard<T>::pointer MemShard<T>::new_entry(Args... args) {
	pointer p = allocate();
	if (p == NULL) {
		return NULL;
	}
	return new (p) value_type(std::forward<Args>(args)...);
}

template <class T>
void MemShard<T>::delete_entry(MemShard<T>::pointer p) {
	if (p != NULL) {
		p->~value_type();
		deallocate(p);
	}
}

template <class T>
size_t MemShard<T>::capacity() const {
	return _max_size;
}

template <class T>
size_t MemShard<T>::size() const {
	return _num_used;
}

template <class T>
size_t MemShard<T>::full() const {
	return _num_used == _max_size;
}

template <class T>
size_t MemShard<T>::empty() const {
	return _num_used == 0;
}
}
#endif
