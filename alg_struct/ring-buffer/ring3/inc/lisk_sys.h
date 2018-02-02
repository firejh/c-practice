/**
 * FILE		:lisk_sys.h
 * DESC		:sys info
 * AUTHOR	:v0.3.0 written by Alex Stocks
 * DATE		:on Oct 3, 2012
 * MOD		:modified by * on June 2, 2012
 **/

#ifndef __LISK_SYS_H__
#define __LISK_SYS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <lisk_type.h>

/**
 * description	: get cpu number<br/><br/>
 *
 * out-@ret		: ret is cpu num
 **/
n4 get_cpu_num(vd);
/**
 * description	: set the processor id on whitch current
 *				  process is willing to run <br/><br/>
 *
 * in-@cpu_id	: target cpu index. its start index is 0.
 *
 * out-@ret		: the ret is 0 if successful. otherwise is -1 instead.
 **/
n4 set_cpu_affinity(n4 cpu_id);

#ifdef __cplusplus
}
#endif

#endif

