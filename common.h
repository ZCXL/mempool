/*************************************************************************
	> File Name: common.h
	> Author: zhuchao
	> Mail: zhuchao1995@hotmail.com 
	> Created Time: 五 12/22 23:43:46 2017
 ************************************************************************/
#ifndef MEM_POOL_COMMON_H
#define MEM_POOL_COMMON_H
#include <stdint.h>
#include <set>
#include <map>
#include <list>
#include <pthread.h>
#include <boost/shared_ptr.hpp>
namespace mempool {
struct pos_compare {
	bool operator() (const uintptr_t& lhs, const uintptr_t& rhs) const {
		return lhs < rhs;
	}
};
#endif
