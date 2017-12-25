/*************************************************************************
	> File Name: util.h
	> Author: zhuchao
	> Mail: zhuchao1995@hotmail.com 
	> Created Time: 日 12/24 16:47:59 2017
 ************************************************************************/
#ifndef MEM_POOL_UTIL_H
#define MEM_POOL_UTIL_H
namespace mempool {
class SpinLock {
private:
	typedef enum {Locked, Unlocked} LockState;
	boost::atomic<LockState> _state;
public:
	SpinLock() : _state(Unlocked) {
	}
	void lock() {
		while (_state.exchange(Locked, boost::memory_order_acquire) == Locked) {
		}
	}
	void unlock() {
		_state.store(Unlocked, boost::memory_order_release);
	}
};
}
#endif
