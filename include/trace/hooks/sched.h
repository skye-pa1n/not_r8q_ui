/* SPDX-License-Identifier: GPL-2.0 */
#undef TRACE_SYSTEM
#define TRACE_SYSTEM sched
#define TRACE_INCLUDE_PATH trace/hooks
#if !defined(_TRACE_HOOK_SCHED_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_HOOK_SCHED_H
#include <linux/tracepoint.h>
#include <trace/hooks/vendor_hooks.h>
/*
 * Following tracepoints are not exported in tracefs and provide a
 * mechanism for vendor modules to hook and extend functionality
 */
struct cgroup_taskset;
#ifdef __GENKSYMS__
struct cgroup_subsys_state;
struct cpufreq_policy;
struct em_perf_domain;
enum uclamp_id;
struct sched_entity;
struct task_struct;
struct uclamp_se;
#else
/* struct cgroup_subsys_state */
#include <linux/cgroup-defs.h>
/* struct cpufreq_policy */
#include <linux/cpufreq.h>
/* struct em_perf_domain */
#include <linux/energy_model.h>
/* enum uclamp_id, struct sched_entity, struct task_struct, struct uclamp_se */
#include <linux/sched.h>
#endif /* __GENKSYMS__ */

#endif /* _TRACE_HOOK_SCHED_H */
/* This part must be outside protection */
#include <trace/define_trace.h>
