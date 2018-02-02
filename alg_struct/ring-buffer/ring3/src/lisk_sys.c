/**
 * FILE		:lisk_sys.c
 * DESC		:system or os info
 * AUTHOR	:v0.3.0 written by Alex Stocks
 * DATE		:on Oct 3, 2012
 * MOD		:modified by * on June 2, 2012
 **/

#include <lisk_sys.h>
#include <lisk_public.h>

#include <unistd.h>
#define __USE_GNU
#include <sched.h>

inline n4 get_cpu_num(vd)
{
	return sysconf(_SC_NPROCESSORS_ONLN);
}

n4 set_cpu_affinity(n4 cpu_id)
{
	n4			num;
	n4			ret;
	cpu_set_t	mask;

	num = get_cpu_num();
	cpu_id %= num;
	// pinfo("cpu num = %d, cpu idx = %d", num, cpu_id);
	CPU_ZERO(&mask);
	CPU_SET(cpu_id, &mask);

	//0 is current process
	ret = sched_setaffinity(0, sizeof(mask), &mask);
	if (IS_EQ(ret, -1)) {
		RET_INT(-1, "sched_setaffinity() = %d", ret);
	}

	RET_INT(0, nil_str);
}

