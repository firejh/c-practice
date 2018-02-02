/******************************************************
# DESC    : redis connection pool
# AUTHOR  : Alex Stocks
# VERSION : 1.0
# LICENCE : Apache License 2.0
# EMAIL   : alexstocks@foxmail.com
# MOD     : 2017-10-25 16:07
# FILE    : pool.h
******************************************************/

#ifndef __POOL_H
#define __POOL_H

#include <pthread.h>
#include <sys/time.h>

#include "list.h"
#include "hiredis.h"

typedef struct pool {
    // Dial is an application supplied function for creating and configuring a
    // connection.
    //
    // The connection returned from Dial must not be in a special state
    // (subscribed to pubsub channel, transaction started, ...).
    // for connect
    struct timeval connectTimeout;
    const char* hostname;
    int port;

    // TestOnBorrow is an optional application supplied function for checking
    // the health of an idle connection before the connection is used again by
    // the application. Argument t is the time that the connection was returned
    // to the pool. If the function returns an error, then the connection is
    // closed.
    // TestOnBorrow func(c Conn, t time.Time) error

    // Maximum number of idle connections in the pool.
    int maxIdle;

    // Maximum number of connections allocated by the pool at a given time.
    // When zero, there is no limit on the number of connections in the pool.
    int maxActive;

    // Close connections after remaining idle for this duration. If the value
    // is zero, then idle connections are not closed. Applications should set
    // the timeout to a value less than the server's timeout.
    long idleTimeout;

    // If Wait is true and the pool is at the MaxActive limit, then Get() waits
    // for a connection to be returned to the pool before returning.
    int wait;

    // mu protects fields defined below.
    pthread_mutex_t mu;
    pthread_cond_t* cond;
    int closed;
    int active;

    // Stack of idleConn with most recently used at the front.
    list* idle;
} pool;


// NewPool creates a new pool.
//
// @hostname: host address, like "127.0.0.1"
// @port: host port
// @max_conn_num: maximum connection number
// @maxIdle: max idle connection number
// @idleTimeout: connection maximum idle time in second
// @wait: If Wait is non-zero and the pool is at the MaxActive limit,
//        then get_redis_context waits for a connection to be returned
//        to the pool before returning.
pool *poolCreate(const char *hostname, int port, int max_conn_num, int maxIdle, int idleTimeout, int wait);

// Close pool_releases the resources used by the pool.
void poolRelease(pool *p);

// Get gets a connection. The application must close the returned connection.
// This method always returns a valid connection so that applications can defer
// error handling to the first use of the connection. If there is an error
// getting an underlying connection, then the connection Err, Do, Send, Flush
// and Receive methods return that error.
// PoolStats contains pool statistics.
//
// If there is an error getting an undlerlying connection, the return value is nil.
redisContext* poolGet(pool *p);

// put a redis connection context into pool
void poolPut(pool* p, redisContext* c);

typedef struct poolStats {
    // active is the number of connections in the pool.
    // The count includes idle connections and connections in use.
    int active;
    // idle is the number of idle connections in the pool.
    int idle;
} poolStats;

// Stats returns pool's statistics.
poolStats poolStat(pool *p);

#endif

