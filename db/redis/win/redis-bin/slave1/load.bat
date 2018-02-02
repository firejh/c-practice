@echo on
rem redis server start script
rem v1.0 written by AlexStocks on 2015/07/18 15:19pm
rem --requirepass redis-instance-password 

mkdir data
mkdir log
taskkill /f /im redis-slave1.exe
start /b bin\redis-slave1.exe ./conf/redis.conf --port 5001 --databases 4 --maxmemory 800m --appendonly yes --appendfilename appendonly1.aof --logfile log\server1.log --dir data --dbfilename dump1.rdb --appendfsync everysec --slaveof 127.0.0.1 5000
