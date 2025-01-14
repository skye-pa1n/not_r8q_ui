/*
 * tracepoint hook handling
 *
 * Copyright (C) 2018 Samsung Electronics Co., Ltd
 * Park Bumgyu <bumgyu.park@samsung.com>
 */

#include "../sched.h"

#include "ems.h"

#include <trace/events/sched.h>
#include <trace/events/ems_debug.h>
#include <trace/hooks/sched.h>
#include <trace/hooks/cpuidle.h>

/******************************************************************************
 * tracepoint of Android vendor hook                                          *
 ******************************************************************************/
#define TASK_AVD_TARGET_CPU(task)	(task->android_vendor_data1[1])

static void ems_hook_select_task_rq_fair(void *data,
			struct task_struct *p, int prev_cpu,
			int sd_flag, int wake_flags, int *new_cpu)
{
	int cpu;

	cpu = ems_select_task_rq_fair(p, prev_cpu, sd_flag, wake_flags);

	*new_cpu = cpu;
	TASK_AVD_TARGET_CPU(p) = cpu;
}

static void ems_hook_select_task_rq_rt(void *data,
			struct task_struct *p, int prev_cpu,
			int sd_flag, int wake_flags, int *new_cpu)
{
	int cpu;

	cpu = ems_select_task_rq_rt(p, prev_cpu, sd_flag, wake_flags);

	*new_cpu = cpu;
	TASK_AVD_TARGET_CPU(p) = cpu;
}

static void ems_hook_select_fallback_rq(void *data,
			int cpu, struct task_struct *p, int *new_cpu)
{
	int target_cpu = TASK_AVD_TARGET_CPU(p);

	*new_cpu = ems_select_fallback_rq(p, target_cpu);
	TASK_AVD_TARGET_CPU(p) = -1;
}

static void ems_hook_find_lowest_rq(void *data,
			struct task_struct *p, struct cpumask *local_cpu_mask,
			int ret, int *lowest_cpu)
{
	*lowest_cpu = frt_find_lowest_rq(p, local_cpu_mask);
}

static void ems_hook_scheduler_tick(void *data, struct rq *rq)
{
	ems_tick(rq);
}

static void ems_hook_exit_idle(void *data, int state,
		struct cpuidle_device *dev)
{
	ems_idle_exit(dev->cpu, state);
}

static void ems_hook_enter_idle(void *data, int *state,
		struct cpuidle_device *dev)
{
	ems_idle_enter(dev->cpu, state);
}

static void ems_hook_enqueue_task(void *data,
			struct rq *rq, struct task_struct *p, int flags)
{
	ems_enqueue_task(rq, p, flags);
}

static void ems_hook_dequeue_task(void *data,
			struct rq *rq, struct task_struct *p, int flags)
{
	ems_dequeue_task(rq, p, flags);
}

static void ems_hook_can_migrate_task(void *data,
			struct task_struct *p, int dst_cpu, int *can_migrate)
{
	*can_migrate = ems_can_migrate_task(p, dst_cpu);
}

static void ems_hook_cpu_cgroup_can_attach(void *data,
			struct cgroup_taskset *tset, int *retval)
{
	ems_cpu_cgroup_can_attach(tset, *retval);
}

static void ems_hook_rebalance_domains(void *data,
			struct rq *rq, int *continue_balancing)
{
	*continue_balancing = !ems_load_balance(rq);
}

static void ems_hook_nohz_balancer_kick(void *data,
			struct rq *rq, unsigned int *flags, int *done)
{
	*done = ems_load_balance(rq);
}

static void ems_hook_newidle_balance(void *data,
			struct rq *this_rq, struct rq_flags *rf,
			int *pulled_task, int *done)
{
	ems_newidle_balance(data, this_rq, rf, pulled_task, done);
}

static void ems_hook_post_init_entity_util_avg(void *data, struct sched_entity *se)
{
	ems_post_init_entity_util_avg(se);
}

static void ems_hook_find_new_ilb(void *data, struct cpumask *nohz_idle_cpus_mask, int *ilb)
{
	*ilb = ems_find_new_ilb(nohz_idle_cpus_mask);
}

static void ems_hook_sched_fork_init(void *data, struct task_struct *p)
{
	ems_qjump_queued(p) = 0;
	INIT_LIST_HEAD(ems_qjump_node(p));
}

static void ems_wake_up_new_task(void *unused, struct task_struct *p)
{
	gsc_init_new_task(p);
}

static void ems_hook_replace_next_task_fair(void *data, struct rq *rq,
					    struct task_struct **p_ptr,
					    struct sched_entity **se_ptr,
					    bool *repick,
					    bool simple,
					    struct task_struct *prev)
{
	ems_replace_next_task_fair(rq, p_ptr, se_ptr, repick, simple, prev);
}

static void ems_hook_schedule(void *data, struct task_struct *prev,
				struct task_struct *next, struct rq *rq)
{
	ems_schedule(prev, next, rq);
}

/******************************************************************************
 * built-in tracepoint                                                        *
 ******************************************************************************/
static void ems_hook_pelt_cfs_tp(void *data, struct cfs_rq *cfs_rq)
{
	trace_sched_load_cfs_rq(cfs_rq);
}

static void ems_hook_pelt_rt_tp(void *data, struct rq *rq)
{
	trace_sched_load_rt_rq(rq);
}

static void ems_hook_pelt_dl_tp(void *data, struct rq *rq)
{
#if 0
	trace_sched_load_dl_rq(rq);
#endif
}

static void ems_hook_pelt_irq_tp(void *data, struct rq *rq)
{
#ifdef CONFIG_HAVE_SCHED_AVG_IRQ
	trace_sched_load_irq(rq);
#endif
}

static void ems_hook_pelt_se_tp(void *data, struct sched_entity *se)
{
	trace_sched_load_se(se);
}

static void ems_hook_sched_overutilized_tp(void *data,
			struct root_domain *rd, bool overutilized)
{
	trace_sched_overutilized(overutilized, NULL, NULL);
}

static void ems_rvh_cpu_cgroup_attach(void *unused, struct cgroup_taskset *tset)
{
	gsc_task_cgroup_attach(tset);
}

void ems_rvh_flush_task(void *unused, struct task_struct *p)
{
	gsc_flush_task(p);
}

int hook_init(void)
{
	return 0;
}
