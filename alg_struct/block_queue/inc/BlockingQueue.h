// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_BLOCKINGQUEUE_H
#define MUDUO_BASE_BLOCKINGQUEUE_H


#include <Mutex.h>
#include <Condition.h>
#include <deque>
using namespace std;

template<typename T>
class BlockingQueue : noncopyable	{
	public:
		BlockingQueue()
			: mutex_(),
			notEmpty_(mutex_),
			queue_()	{
	}

		void put(const T& x)	{
			MutexLockGuard lock(mutex_);
			queue_.push_back(x);
			notEmpty_.notify(); // TODO: move outside of lock
		}

		T take()	{
			MutexLockGuard lock(mutex_);
			// always use a while-loop, due to spurious wakeup
			while (queue_.empty())	{
				notEmpty_.wait();
			}
			T front(queue_.front());
			queue_.pop_front();
			return front;
		}

		size_t size() const	{
			MutexLockGuard lock(mutex_);
			return queue_.size();
		}

	private:
		mutable MutexLock mutex_;
		Condition         notEmpty_;
		std::deque<T>     queue_;
};

#endif

