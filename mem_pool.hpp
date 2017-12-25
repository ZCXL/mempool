/*************************************************************************
	> File Name: mem_pool.hpp
	> Author: zhuchao
	> Mail: zhuchao1995@hotmail.com 
	> Created Time: 五 12/22 22:17:04 2017
 ************************************************************************/
#ifndef MEM_POOL_HPP
#define MEM_POOL_HPP
#include "common.h"
namespace mempool {
template <class T, size_t BlockSize = 4096>
class MemPool {
public:
	/* Member types */
	typedef T value_type;
	typedef T* pointer;
	typedef T& reference;
	typedef const T* const_pointer;
	typedef const T& const_reference;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	/* Construct functions */
	MemPool();
	MemPool(const MemPool& mempool);
	MemPool(MemPool&& mempool);

	/* Deconstruct function */
	virtual ~MemPool();

	/* Remove assignment function */
	MemPool& operator=(const MemPool& mempool) = delete;
	MemPool& operator=(MemPool&& mempoll) = delete;

	/* Get address of object */
	inline pointer address(reference x) const {
		return &x;
	}

	inline const_pointer address(const_reference x) const {
		return &x;
	}

	/* allocate memory and deallocate memory */
	pointer allocate(size_type n = 1);
	void deallocate(pointer p);

	/* create an entry and delete an entry */
	template <class... Args> pointer new_entry(Args&&... args);
	void delete_entry(pointer p);
private:
	struct PosHash {
		uint32_t _shard_pos;
		uint32_t _entry_pos;
	};
	std::unorderd_map<uint32_t, MemShard> _shards;
	std::stack<PosHash> _unused_stack;
	std::list<PosHash> _used_list;
	void* _start_pos;
	void* _end_pos;

	static_assert(BlockSize >= 2 * sizeof(value_type), "BlockSize is too small");
};
template <class T, size_t BlockSize>
MemPool<T, BlockSize>::MemPool() {
	
}

template <class T, size_t BlockSize>
MemPool<T, BlockSize>::pointer MemPool<T, BlockSize>::alloc(
		MemPool<T, BlockSize>::size_type n, MemPool<T, BlockSize>::const_pointer hint) {



}
template <class T, size_t BlockSize>
void MemPool<T, BlockSize>::alloc(MemPool<T, BlockSize>::pointer p) {

}
}
#endif
