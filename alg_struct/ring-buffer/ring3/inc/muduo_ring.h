/**
 * FILE		: lock_ring.h
 * DESC		: locked ring
 * AUTHOR	: v1.0 written by Alex Stocks
 * DATE		: on Apr 1, 2014
 * LICENCE	:
 * MOD		:
 **/

#ifndef __LOCK_RING_H__
#define __LOCK_RING_H__

#include <deque>
#include <vector>
#include <pthread.h>
#include <semaphore.h>

class mutex_lock
{
	public:
		mutex_lock()
		{
			pthread_mutex_init(&mutex_, NULL);
		}

		~mutex_lock()
		{
			pthread_mutex_destroy(&mutex_);
		}

		void lock()
		{
			pthread_mutex_lock(&mutex_);
		}

		void unlock()
		{
			pthread_mutex_unlock(&mutex_);
		}

		pthread_mutex_t* getPthreadMutex() /* non-const */
		{
			return &mutex_;
		}

	private:
		pthread_mutex_t mutex_;
};

class mutex_lock_guard
{
	public:
		explicit mutex_lock_guard(mutex_lock& mutex) : mutex_(mutex)
		{
			mutex_.lock();
		}

		~mutex_lock_guard()
		{
			mutex_.unlock();
		}

	private:
		mutex_lock& mutex_;
};

class condition
{
	public:
		explicit condition(mutex_lock& mutex) : mutex_(mutex)
		{
			pthread_cond_init(&pcond_, NULL);
		}

		~condition()
		{
			pthread_cond_destroy(&pcond_);
		}

		void wait()
		{
			pthread_cond_wait(&pcond_, mutex_.getPthreadMutex());
		}

		void notify()
		{
			pthread_cond_signal(&pcond_);
		}

	private:
		mutex_lock&		mutex_;
		pthread_cond_t	pcond_;
};

//*
template<typename T>
class block_queue
{
	public:
		block_queue()
			: mutex_(),
			notEmpty_(mutex_),
			queue_()
		{
		}

		int push(T *x)
		{
			if (x)
			{
				mutex_lock_guard lock(mutex_);
				queue_.push_back(*x);
				notEmpty_.notify();

				return 0;
			}

			return -1;
		}

		int pop(T *x)
		{
			if (x)
			{
				mutex_lock_guard lock(mutex_);
				// always use a while-loop, due to spurious wakeup
				while (queue_.empty())
				{
					notEmpty_.wait();
				}
				T front(queue_.front());
				queue_.pop_front();
				*x = front;

				return 0;
			}

			return -1;
		}

		int size(unsigned* size)
		{
			if (size)
			{
				mutex_lock_guard lock(mutex_);
				*size = queue_.size();

				return 0;
			}

			return -1;
		}

	private:
		mutable mutex_lock	mutex_;
		condition			notEmpty_;
		std::deque<T>		queue_;
};
//*/

class semaphore
{
	public:
		semaphore(void)
		{
			sem_init(&sem_, 0, 0);
		}

		~semaphore(void)
		{
			sem_destroy(&sem_);
		}

		void wait(void)
		{
			do {
				int ret = sem_wait(&sem_);
				if (!ret) {
					break;
				}
			} while(1);
		}

		void notify(void)
		{
			do {
				int ret = sem_post(&sem_);
				if (!ret) {
					break;
				}
			} while(1);
		}
	private:
		sem_t	sem_;
};

//*

#include <lisk_public.h>
#include <lisk_deque.h>

template<typename T>
class block_queue2
{
	public:
		block_queue2(void)
			: mutex_(),
			cond_(mutex_),
			queue_(nil)
		{
			n4 ret = deque_init(queue_, sizeof(T));
			if (IS_NZR(ret))	{
				queue_ = nil;
			}
		}

		~block_queue2(void)
		{
			if (IS_NNL(queue_))	{
				deque_uninit(queue_);
				queue_ = nil;
			}
		}

		int push(T *x)
		{
			if (queue_ && x)
			{
				mutex_lock_guard lock(mutex_);
				deque_push_back(queue_, (vp)(x));
				cond_.notify();

				return 0;
			}

			return -1;
		}

		int pop(T *x)
		{
			if (x)
			{
				mutex_lock_guard lock(mutex_);
				while (IS_ZR(deque_empty(queue_))) {
					cond_.wait();
				}

				return deque_pop_front(queue_, (vp)(x));
			}

			return -1;
		}

		int size(unsigned* size)
		{
			if (size)
			{
				mutex_lock_guard lock(mutex_);
				return deque_size(queue_, size);
			}

			return -1;
		}

	private:
		mutable mutex_lock	mutex_;
		condition			cond_;
		void*				queue_;
};
//*/

#endif

