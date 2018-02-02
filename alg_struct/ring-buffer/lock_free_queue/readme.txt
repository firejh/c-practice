
Concurrent queue:
push thread 2, push num 100000001, use time 39.333206 sec
push thread 1, push num 100000001, use time 39.333244 sec
pop thread 1, pop num 100000001, use time 39.356873 sec
pop thread 2, pop num 100000001, use time 39.356834 sec


Mpmc:
push thread 2, push num 100000000, use time 43.740673 sec
push thread 1, push num 100000001, use time 43.740696 sec
pop thread 2, pop num 100000001, use time 43.765747 sec
pop thread 1, pop num 100000001, use time 43.765793 sec

bounded_queue:
push thread 2, push num 100000001, use time 1300.619141 sec
push thread 1, push num 100000000, use time 1300.619263 sec
pop thread 2, pop num 100000001, use time 1300.619141 sec
pop thread 1, pop num 100000001, use time 1300.619141 sec
