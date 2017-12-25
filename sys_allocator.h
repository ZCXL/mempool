/*************************************************************************
	> File Name: sys_allocator.h
	> Author: zhuchao
	> Mail: zhuchao1995@hotmail.com 
	> Created Time: 日 12/24 09:45:06 2017
 ************************************************************************/
#ifndef MEM_POOL_SYS_ALLOCATOR_H
#define MEM_POOL_SYS_ALLOCATOR_H
#include <config.h>
#include <stddef.h>
#ifdef HAVE_STDINT_H
#include <stdint.h>
#elif defined HAVE_INTTYPES_H
#include <inttypes.h>
#else
#include <sys/types.h>
#endif
#ifdef HAVE_MMAP
#include <sys/mman.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <new>
namespace mempool {
class SysAllocator {
public:
	SysAllocator() {}
	virtual ~SysAllocator() {}
	virtual void* alloc(size_t size, size_t* actual_size, size_t alignment) = 0;
};
class SbrkSysAllocator : public SysAllocator {
public:
	SbrkSysAllocator() : SysAllocator() {
	}
	void* alloc(size_t size, size_t* actual_size, size_t alignment);
};
static union {
	char buf[sizeof(SbrkSysAllocator)];
	void *ptr;
} sbrk_space;
class MmapSysAllocator : public SysAllocator {
public:
	MmapSysAllocator() : SysAllocator() {
	}
	void* alloc(size_t size, size_t* actual_size, size_t alignment);
};
static union {
	char buf[sizeof(MmapSysAllocator)];
	void *ptr;
} mmap_space;
class DefualtAllocator : public SysAllocator {
public:
	#define MAX_ALLOCATOR_SIZE 2
	DefualtAllocator() : SysAllocator() {
		for (int i = 0; i < MAX_ALLOCATOR_SIZE; ++i) {
			_allocators[i] = NULL;
			_names[i] = NULL;
			_failed[i] = false;
		}
	}
	void add_allocator(SysAllocator* allocator, int index, const char* name);
	void* alloc(size_t size, size_t* actual_size, size_t alignment);
private:
	SysAllocator* _allocators[MAX_ALLOCATOR_SIZE];
	const char* _names[MAX_ALLOCATOR_SIZE];
	bool _failed[MAX_ALLOCATOR_SIZE];
};
static union {
	char buf[sizeof(DefaultSysAllocator)];
	void *ptr;
} default_space;
union MemoryAligner {
	void* p;
	double d;
	size_t s;
};
extern void* system_alloc(size_t bytes, size_t* actual_bytes, size_t alignment = 0);
extern bool system_release(void* start, size_t length);
extern void system_commit(void* start, size_t length);
}
#endif
