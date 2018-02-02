@echo on
rem redis server start script
rem v1.0 written by AlexStocks on 2015/07/18 15:19pm
rem --requirepass redis-instance-password 

mkdir data
mkdir log
taskkill /f /im redis-server1.exe
start /b bin\redis-server1.exe ./conf/redis.conf --port 5000 --databases 4 --maxmemory 800m --appendonly yes --appendfilename appendonly1.aof --logfile log\server1.log --dir data --dbfilename dump1.rdb --appendfsync everysec
