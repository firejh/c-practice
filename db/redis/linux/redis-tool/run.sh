#!/bin/sh
#bin/redis-dump 127.0.0.1 10000 redis-instance-password 0
#bin/redis-dump-key 127.0.0.1 10000 redis-instance-password 0 list_programmer
bin/redis-migrate 127.0.0.1 10000 redis-instance-password 0  127.0.0.1 40000 redis-instance-password 0
