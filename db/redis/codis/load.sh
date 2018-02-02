#!/usr/bin/env bash
# ******************************************************
# DESC    : flume devops script
# AUTHOR  : Alex Stocks
# VERSION : 1.0
# LICENCE : LGPL V3
# EMAIL   : alexstocks@foxmail.com
# MOD     : 2017-08-16 22:20
# FILE    : codis_load.sh
# ******************************************************

proxy_ip=192.168.150.79
proxy_port=19000
proxy_http_port=11000
redis_num=4
redis_host=192.168.150.79
redis_port=6379

usage() {
    echo "Usage: $0 start dash/proxy/redis"
    echo "       $0 stop  dash/proxy/redis"
    exit
}

start_dashboard() {
	nohup bin/codis-config dashboard --addr=:18088 --http-log=log/dashboard_http.log >log/dashboard.nohup.out 2>&1 &
    PID=`ps aux | grep codis-config | grep dashboard | grep -v grep | awk '{print $2}'`
    if [ "$PID" != "" ];
    then
        for p in $PID
        do
            echo "start dashboard ( pid =" $p ")"
        done
    fi
    echo "you can got your zk config in /zk path"
}

stop_dashboard() {
    PID=`ps aux | grep codis-config | grep dashboard | grep -v grep | awk '{print $2}'`
    if [ "$PID" != "" ];
    then
        for ps in $PID
        do
            echo "kill dashboard ( pid =" $ps ")"
            kill $ps
        done
    fi
}

start_proxy() {
	nohup bin/codis-proxy -c config.ini -L ./log/proxy.log  --cpu=1 --addr=${proxy_ip}:${proxy_port} --http-addr=${proxy_ip}:${proxy_http_port} >log/proxy.nohup.out &
    PID=`ps aux | grep codis-proxy | grep -v grep | awk '{print $2}'`
    if [ "$PID" != "" ];
    then
        for p in $PID
        do
            echo "start proxy ( pid =" $p ")"
        done
    fi
}

stop_proxy() {
    PID=`ps aux | grep codis-proxy | grep -v grep | awk '{print $2}'`
    if [ "$PID" != "" ];
    then
        for ps in $PID
        do
            echo "kill proxy ( pid =" $ps ")"
            kill $ps
        done
    fi
}

start_redis() {
    port=$redis_port
    for ((idx = 0; idx < $redis_num; idx ++))
    do
        nohup bin/codis-server --bind $redis_host --port $port >log/redis_${port}.nohup.out  &
		PID=`ps aux | grep codis-server | grep $port | grep -v grep | awk '{print $2}'`
		if [ "$PID" != "" ];
		then
			for p in $PID
			do
				echo "start codis-server $port ( pid =" $p ")"
			done
		fi

        ((port ++))
    done

    # add redis master
	num=$redis_num
	((num/=2))
    port=$redis_port
    for ((idx = 1; idx <= $num; idx ++))
    do
	    bin/codis-config server add $idx $redis_host:$port master
        ((port += 2))
    done

	sleep 2

    # add redis slave
    port=$redis_port
	((port ++))
    for ((idx = 1; idx <= $num; idx ++))
    do
	    bin/codis-config server add $idx $redis_host:$port slave
        ((port += 2))
    done

	init_slot
}

stop_redis() {
    port=$redis_port
    for ((idx = 0; idx < $redis_num; idx ++))
    do
		PID=`ps aux | grep codis-server | grep $port | grep -v grep | awk '{print $2}'`
		if [ "$PID" != "" ];
		then
			for ps in $PID
			do
				echo "stop codis-server $port ( pid =" $ps ")"
                kill $ps
			done
		fi
        sleep 2
        ((port ++))
    done
}

init_slot() {
	bin/codis-config -c config.ini slot init
	bin/codis-config -c config.ini slot range-set 0   300  1 online
	bin/codis-config -c config.ini slot range-set 301 1023 2 online
}

opt=$1
case C"$opt" in
    Cstart)
        role=$2
        case C"$role" in
            Cdash)
                start_dashboard
                ;;
            Cproxy)
                start_proxy
                ;;
            Credis)
                start_redis
                ;;
            C*)
                usage
                ;;
        esac
        ;;
    Cstop)
        role=$2
        case C"$role" in
            Cdash)
                stop_dashboard
                ;;
            Cproxy)
                stop_proxy
                ;;
            Credis)
                stop_redis
                ;;
            C*)
                usage
                ;;
        esac
        ;;
    C*)
        usage
        ;;
esac

