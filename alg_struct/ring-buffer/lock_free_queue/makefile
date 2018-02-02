all:
	g++ -Wall -lpthread -rdynamic -D_GLIBCXX_USE_NANOSL -D_GLIBCXX_USE_NANOSLEEP -D_GLIBCXX_USE_SCHED_YIELD -std=c++11 -I./ -O2 -o concurrentqueue concurrentqueue.cc
	g++ -Wall -lpthread -rdynamic -D_GLIBCXX_USE_NANOSL -D_GLIBCXX_USE_NANOSLEEP -D_GLIBCXX_USE_SCHED_YIELD -std=c++11 -I./ -O2 -o mpmc-bounded-queue mpmc-bounded-queue.cc

clean:
	rm concurrentqueue
	rm mpmc-bounded-queue
