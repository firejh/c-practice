#include "pool.h"

#include <stdlib.h>
#include <string.h>

typedef struct idleRedisContext {
	long last_active; // in second
	redisContext *context;
} idleRedisContext;

idleRedisContext * createIdleRedisContext(redisContext *c)
{
	idleRedisContext *ic = (idleRedisContext*)malloc(sizeof(idleRedisContext));
	if (ic) {
		ic->last_active = time(NULL);
		ic->context = c;
	}

	return ic;
}

void freeIdleRedisContext(idleRedisContext *ic)
{
	redisFree(ic->context);
	free(ic);
}

// redis.conf: tcp-keepalive 300 seconds
int testOnBorrow(idleRedisContext* context)
{
	long now = time(NULL);
	long diff = now - context->last_active;
	if (0 <= diff && diff <= 100) {
		return 0;
	}

	redisReply *reply = (redisReply*)redisCommand(context->context, "PING");
	int ret = -1;
    if (reply != NULL && reply->type == REDIS_REPLY_STATUS && strcmp(reply->str, "PONG") == 0) {
		ret = 0;
	}
    freeReplyObject(reply);

	return ret;
}

pool *poolCreate(const char *hostname, int port, int max_conn_num, int maxIdle, int idleTimeout, int wait)
{
    pool *p = (pool*)malloc(sizeof(pool));
    if (p == NULL) {
        return NULL;
    }

    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 500000;
    p->connectTimeout = timeout;
    p->hostname = hostname;
    p->port = port;
    p->maxIdle = maxIdle;
    p->maxActive = max_conn_num;
	p->idleTimeout = (long)idleTimeout;
	p->wait = wait;
	p->cond = NULL;
	p->closed = 0;
	p->active = 0;
	p->idle = listCreate();
    pthread_mutex_init(&(p->mu), NULL);

    return p;
}

// Close pool_releases the resources used by the pool.
void poolRelease(pool *p)
{
	if (!p) {
		return;
	}

	list* idle;
	pthread_mutex_lock(&p->mu);
	idle = p->idle;
	p->closed = 1;
	p->active -= listLength(idle);
	if (p->cond) {
		pthread_cond_broadcast(p->cond);
	}
	pthread_mutex_unlock(&p->mu);
	pthread_mutex_destroy(&p->mu);

	listNode *node;
	idleRedisContext *ic;
	while (node = listFirst(idle)) {
		ic = (idleRedisContext*)listNodeValue(node);
		listDelNode(idle, node);
		freeIdleRedisContext(ic);
	}
	listRelease(idle);
}

// pool_release decrements the active count and signals waiters. The caller must
// hold p.mu during the call.
void pool_release(pool *p)
{
	if (p)
	{
		p->active -= 1;
		if (p->cond) {
			pthread_cond_signal(p->cond);
		}
	}
}

// get prunes stale connections and returns a connection from the idle list or
// creates a new connection.
redisContext* poolGet(pool *p)
{
	if (!p) {
		return NULL;
	}

	unsigned long i = 0;
    unsigned long n = 0;
	long cur_sec = time(NULL);
	listNode *e = NULL;
	idleRedisContext* ic = NULL;
	redisContext *context;

    pthread_mutex_lock(&p->mu);
	// Prune stale connections.
	if (p->idleTimeout > 0) {
        n = listLength(p->idle);
		for (i = 0; i < n; i++) {
            e= listLast(p->idle);
			if (e == NULL) {
				break;
			}
			ic = (idleRedisContext*)listNodeValue(e);
			if (p->idleTimeout > (cur_sec - ic->last_active)) {
				break;
			}
			listDelNode(p->idle, e);
			pool_release(p);
			pthread_mutex_unlock(&p->mu);
			freeIdleRedisContext(ic);
			pthread_mutex_lock(&p->mu);
        	n = listLength(p->idle);
		}
	}

	for (;;) {
		// Get idle connection.
        n = listLength(p->idle);
		for (i = 0; i < n; i++) {
            e = listFirst(p->idle);
			if (e == NULL) {
				break;
			}
			ic = (idleRedisContext*)listNodeValue(e);
			listDelNode(p->idle, e);
			pthread_mutex_unlock(&p->mu);

			if (!testOnBorrow(ic)) {
				context = ic->context;
				free(ic);
				return context;
			}

			freeIdleRedisContext(ic);

			pthread_mutex_lock(&p->mu);
			pool_release(p);
        	n = listLength(p->idle);
		}

		// Check for pool closed before dialing a new connection.
		if (p->closed) {
			pthread_mutex_unlock(&p->mu);
			return NULL;
		}

		// Dial new connection if under limit.
		if (p->maxActive == 0 || p->active < p->maxActive) {
			p->active += 1;
			pthread_mutex_unlock(&p->mu);
			context = redisConnectWithTimeout(p->hostname, p->port, p->connectTimeout);
			if (!context || context->err) {
				if (context) {
					redisFree(context);
					context = NULL;
				}

				pthread_mutex_lock(&p->mu);
				pool_release(p);
				pthread_mutex_unlock(&p->mu);
			}

			return context;
		}

		if (!p->wait) {
			pthread_mutex_unlock(&p->mu);
			return NULL;
		}

		if (p->cond == NULL) {
			pthread_cond_init(p->cond, NULL);
		}

		pthread_cond_wait(p->cond, &p->mu);
	}

	return NULL;
}

poolStats poolStat(pool *p)
{
	poolStats stats;
	if (p) {
		pthread_mutex_lock(&p->mu);
		stats.active = p->active;
		stats.idle = listLength(p->idle);
		pthread_mutex_unlock(&p->mu);
	}

	return stats;
}


void poolPut(pool* p, redisContext* c)
{
	if (!p || !c) {
		return;
	}

	idleRedisContext *ic = NULL;
	listNode *node = NULL;

	pthread_mutex_lock(&p->mu);
	if (!(p->closed) && !(c->err)) {
		ic = createIdleRedisContext(c);
		listAddNodeHead(p->idle, ic);
		ic = NULL;
		if (listLength(p->idle) > p->maxIdle) {
			node = listLast(p->idle);
			ic = (idleRedisContext*)listNodeValue(node);
			listDelNode(p->idle, node);
		} else {
			ic = NULL;
		}
	}

	if (ic == NULL) {
		if (p->cond) {
			pthread_cond_signal(p->cond);
		}
		pthread_mutex_unlock(&p->mu);
		return;
	}

	pool_release(p);
	pthread_mutex_unlock(&p->mu);

	freeIdleRedisContext(ic);
}
