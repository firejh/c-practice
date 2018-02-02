/******************************************************
# DESC    :
# AUTHOR  : Alex Stocks
# VERSION : 1.0
# LICENCE : Apache License 2.0
# EMAIL   : alexstocks@foxmail.com
# MOD     : 2017-09-21 13:17
# FILE    : muduo_bouned_queue.h
******************************************************/

#ifndef __BOUNDED_BLOCKING_QUEUE_H__
#define __BOUNDED_BLOCKING_QUEUE_H__

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

class Condition : noncopyable	{
	public:
		explicit Condition(MutexLock& mutex) : mutex_(mutex)	{
		pthread_cond_init(&pcond_, NULL);
	}

		~Condition()	{
			pthread_cond_destroy(&pcond_);
		}

		void wait()	{
			pthread_cond_wait(&pcond_, mutex_.getPthreadMutex());
		}

		// returns true if time out, false otherwise.
		bool waitForSeconds(int seconds);

		void notify()	{
			pthread_cond_signal(&pcond_);
		}

		void notifyAll()	{
			pthread_cond_broadcast(&pcond_);
		}

	private:
		MutexLock& mutex_;
		pthread_cond_t pcond_;
};

#include <algorithm>
#include <cstddef>
#include <cassert>
#include <stdexcept>
#include <iostream>

template <typename T>
class circular_buffer
{
public:
  explicit circular_buffer(size_t capacity);
  circular_buffer(const circular_buffer<T> &rhs);
  circular_buffer(circular_buffer<T>&& rhs);
  ~circular_buffer() { if (buffer_) delete[] buffer_; }

  circular_buffer<T>& operator=(circular_buffer<T> rhs);

  size_t size() const { return (write_pos_ + capacity_ - read_pos_) % capacity_; }
  size_t capacity() const { return capacity_; }

  bool empty() const { return read_pos_ == write_pos_; }
  bool full() const { return (write_pos_ + 1) % capacity_ == read_pos_; }

  bool push_back(T item);
  void pop_front();
  const T& front();

  friend void swap(circular_buffer<T> &lhs, circular_buffer<T> &rhs)
  {
    std::swap(lhs.buffer_, rhs.buffer_);
    std::swap(lhs.capacity_, rhs.capacity_);
    std::swap(lhs.read_pos_, rhs.read_pos_);
    std::swap(lhs.write_pos_, rhs.write_pos_);
  }

private:
  T* buffer_;
  size_t capacity_;
  size_t read_pos_;
  size_t write_pos_;

  circular_buffer();
};

template<typename T>
circular_buffer<T>::circular_buffer()
  : buffer_(nullptr)
  , capacity_(0)
  , read_pos_(0)
  , write_pos_(0)
{
}

template<typename T>
circular_buffer<T>::circular_buffer(size_t capacity)
  : circular_buffer()
{
  if (capacity < 1) throw std::length_error("Invalid capacity");

  buffer_ = new T[capacity];
  capacity_ = capacity;
}

template<typename T>
circular_buffer<T>::circular_buffer(const circular_buffer<T> &rhs)
  : buffer_(new T[rhs.capacity_])
  , capacity_(rhs.capacity_)
  , read_pos_(rhs.read_pos_)
  , write_pos_(rhs.write_pos_)
{
  std::copy(rhs.buffer_, rhs.buffer_ + capacity_, buffer_);
}

template<typename T>
circular_buffer<T>::circular_buffer(circular_buffer<T>&& rhs)
  : circular_buffer()
{
  swap(*this, rhs);
}

template<typename T>
circular_buffer<T>&
circular_buffer<T>::operator=(circular_buffer<T> rhs)
{
  swap(*this, rhs);
  return *this;
}

template<typename T>
bool
circular_buffer<T>::push_back(T item)
{
  if (full()) {
    return false;
  }

  buffer_[write_pos_] = item;
  write_pos_ = (write_pos_+1) % capacity_;
  return true;
}

template<typename T>
const T&
circular_buffer<T>::front()
{
  static T t;
  if (empty()) {
    return t;
  }

  return buffer_[read_pos_];
}

template<typename T>
void
circular_buffer<T>::pop_front()
{
  if (empty()) {
    return;
  }

  read_pos_ = (read_pos_+1) % capacity_;
}

template<typename T>
class BoundedBlockingQueue : noncopyable
{
 public:
  explicit BoundedBlockingQueue(int maxSize)
    : mutex_(),
      notEmpty_(mutex_),
      notFull_(mutex_),
      queue_(maxSize)
  {
  }

  void put(const T& x)
  {
    MutexLockGuard lock(mutex_);
    while (queue_.full())
    {
      notFull_.wait();
    }
    assert(!queue_.full());
    queue_.push_back(x);
    notEmpty_.notify();
  }

  T take()
  {
    MutexLockGuard lock(mutex_);
    while (queue_.empty())
    {
      notEmpty_.wait();
    }
    assert(!queue_.empty());
    T front(queue_.front());
    queue_.pop_front();
    notFull_.notify();
    return front;
  }

  bool empty() const
  {
    MutexLockGuard lock(mutex_);
    return queue_.empty();
  }

  bool full() const
  {
    MutexLockGuard lock(mutex_);
    return queue_.full();
  }

  size_t size() const
  {
    MutexLockGuard lock(mutex_);
    return queue_.size();
  }

  size_t capacity() const
  {
    MutexLockGuard lock(mutex_);
    return queue_.capacity();
  }

 private:
  mutable MutexLock     mutex_;
  Condition             notEmpty_;
  Condition             notFull_;
  circular_buffer<T>    queue_;
};

#endif

