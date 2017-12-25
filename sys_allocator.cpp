/*************************************************************************
  > File Name: sys_allocator.cpp
  > Author: zhuchao
  > Mail: zhuchao1995@hotmail.com 
  > Created Time: 日 12/24 10:23:33 2017
 ************************************************************************/
#include "sys_allocator.h"
namespace mempool {
	void* SbrkSysAllocator::alloc(size_t size, size_t* actual_size, size_t alignment) {
#if !defined(HAVE_SBRK) || defined(__UCLIBC__)
		return NULL;
#else
		if (static_cast<ptrdiff_t>(size + alignment) < 0) {
			return NULL;
		}
		size = ((size + alignment - 1) / alignment) * alignment;
		if (actual_size) {
			*actual_size = size;
		}
		/* prevent memory from overflow */
		if (reinterpret_cast<intptr_t>(sbrk(0)) + size < size) {
			return NULL;
		}
		void* result = sbrk(size);
		if (result == reinterpret_cast<void*>(-1)) {
			return NULL;
		}
		uintptr_t ptr = reinterpret_cast<uintptr_t>(result);
		if ((ptr & (alignment - 1)) == 0) {
			return result;
		}
		size_t extra = alignment - (ptr & (alignment - 1));
		void* r2 = sbrk(extra);
		if (reinterpret_cast<uintptr_t>(r2) == (ptr + size)) {
			return reinterpret_cast<void*>(ptr + extra);
		}
		result = sbrk(size + alignment - 1);
		if (result == reinterpret_cast<void*>(-1)) {
			return NULL;
		}
		ptr = reinterpret_cast<uintptr_t>(result);
		if ((ptr & (alignment - 1)) != 0) {
			ptr += alignment - (ptr & (alignment - 1));
		}
		return reinterpret_cast<void*>(ptr);
#endif
	}

	void* MmapSysAllocator::alloc(size_t size, size_t* actual_size, size_t alignment) {
#ifndef HAVE_MMAP
		return NULL;
#else
		// Enforce page alignment
		if (pagesize == 0) pagesize = getpagesize();
		if (alignment < pagesize) alignment = pagesize;
		size_t aligned_size = ((size + alignment - 1) / alignment) * alignment;
		if (aligned_size < size) {
			return NULL;
		}
		size = aligned_size;

		// "actual_size" indicates that the bytes from the returned pointer
		// p up to and including (p + actual_size - 1) have been allocated.
		if (actual_size) {
			*actual_size = size;
		}

		// Ask for extra memory if alignment > pagesize
		size_t extra = 0;
		if (alignment > pagesize) {
			extra = alignment - pagesize;
		}

		// Note: size + extra does not overflow since:
		//            size + alignment < (1<<NBITS).
		// and        extra <= alignment
		// therefore  size + extra < (1<<NBITS)
		void* result = mmap(NULL, size + extra,
				PROT_READ|PROT_WRITE,
				MAP_PRIVATE|MAP_ANONYMOUS,
				-1, 0);
		if (result == reinterpret_cast<void*>(MAP_FAILED)) {
			return NULL;
		}

		// Adjust the return memory so it is aligned
		uintptr_t ptr = reinterpret_cast<uintptr_t>(result);
		size_t adjust = 0;
		if ((ptr & (alignment - 1)) != 0) {
			adjust = alignment - (ptr & (alignment - 1));
		}

		// Return the unused memory to the system
		if (adjust > 0) {
			munmap(reinterpret_cast<void*>(ptr), adjust);
		}
		if (adjust < extra) {
			munmap(reinterpret_cast<void*>(ptr + adjust + size), extra - adjust);
		}

		ptr += adjust;
		return reinterpret_cast<void*>(ptr);
#endif  // HAVE_MMAP
	}

	void DefaultAllocator::add_allocator(SysAllocator* allocator, int index, const char* name) {
		if (index < 0 || index > MAX_ALLOCATOR_SIZE - 1) {
			return;
		}
		_allocators[i] = allocator;
		_names[i] = name;
	}

	void* DefaultAllocator::alloc(size_t size, size_t* actual_size, size_t alignment) {
		void* result = NULL;
		for (int i = 0; i < MAX_ALLOCATOR_SIZE; ++i) {
			if (_failed[i] || _allocators[i] == NULL) {
				continue;
			}
			result = _allocators[i]->alloc(size, actual_size, alignment);
			if (result != NULL) {
				return result;
			}
			_failed[i] = true;
		}

		return NULL;
	}

	/**
	 * Init sys allocators
	 */
	bool sys_allocator_is_init = false;
	static const char[] sbrk_name = "SbrkSysAllocator";
	static const char[] mmap_name = "MmapSysAllocator";
	SysAllocator* sys_allocator = NULL;
	void init_sys_allocator(void) {
		if (sys_allocator_is_init || sys_allocator != NULL) {
			return;
		}
		SbrkSysAllocator* sbrk_allocator = new (sbrk_space.buf)SbrkSysAllocator();
		MmapSysAllocator* mmap_allocator = new (mmap_space.buf)MmapSysAllocator();
		DefaultAllocator* default_allocator = new(default_space.buf)DefaultAllocator();
		default_allocator->add_allocator(sbrk_allocator, 0, sbrk_name);
		default_allocator->add_allocator(mmap_allocator, 1, mmap_allocator);

		sys_allocator = default_allocator;
		sys_allocator_is_init = true;
	}

	void* sys_alloc(size_t bytes, size_t* actual_size, size_t alignment) {
		if (!sys_allocator_is_init) {
			init_sys_allocator();
		}
		if (alignment < sizeof(MemoryAligner)) {
			alignment = sizeof(MemoryAligner);
		}
		size_t actual_size_storage;
		if (actual_size == NLL) {
			actual_size = &actual_size_storage;
		}
		void *result = sys_allocator->alloc(bytes, actual_size, alignment);

		return result;
	}

	bool sys_release(void* start, size_t length) {
		return false;
	}

	void sys_commit(void* start, size_t length) {
		return;
	}
}
