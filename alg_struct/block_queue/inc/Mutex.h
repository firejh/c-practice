// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_MUTEX_H
#define MUDUO_BASE_MUTEX_H

#include <pthread.h>

class noncopyable	{
	protected:
		noncopyable() {}
		~noncopyable() {}
	private:  // emphasize the following members are private
		noncopyable( const noncopyable& );
		const noncopyable& operator=( const noncopyable& );
};

class MutexLock : noncopyable	{
	public:
		MutexLock() {
			pthread_mutex_init(&mutex_, NULL);
		}

		~MutexLock()	{
			pthread_mutex_destroy(&mutex_);
		}

		// internal usage
		void lock()	{
			pthread_mutex_lock(&mutex_);
		}

		void unlock()	{
			pthread_mutex_unlock(&mutex_);
		}

		pthread_mutex_t* getPthreadMutex() /* non-const */	{
			return &mutex_;
		}

	private:
		pthread_mutex_t mutex_;
};

class MutexLockGuard : noncopyable	{
	public:
		explicit MutexLockGuard(MutexLock& mutex) : mutex_(mutex)	{
			mutex_.lock();
		}

		~MutexLockGuard()	{
			mutex_.unlock();
		}

	private:

		MutexLock& mutex_;
};

#endif  // MUDUO_BASE_MUTEX_H

