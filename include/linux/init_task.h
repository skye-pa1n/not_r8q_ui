/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX__INIT_TASK_H
#define _LINUX__INIT_TASK_H

#include <linux/rcupdate.h>
#include <linux/irqflags.h>
#include <linux/utsname.h>
#include <linux/lockdep.h>
#include <linux/ftrace.h>
#include <linux/ipc.h>
#include <linux/pid_namespace.h>
#include <linux/user_namespace.h>
#include <linux/securebits.h>
#include <linux/seqlock.h>
#include <linux/rbtree.h>
#include <linux/sched/autogroup.h>
#include <net/net_namespace.h>
#include <linux/sched/rt.h>
#include <linux/task_integrity.h>
#include <linux/livepatch.h>
#include <linux/mm_types.h>

#include <asm/thread_info.h>

extern struct files_struct init_files;
extern struct fs_struct init_fs;
extern struct nsproxy init_nsproxy;
extern struct group_info init_groups;
extern struct cred init_cred;

#ifndef CONFIG_VIRT_CPU_ACCOUNTING_NATIVE
#define INIT_PREV_CPUTIME(x)	.prev_cputime = {			\
	.lock = __RAW_SPIN_LOCK_UNLOCKED(x.prev_cputime.lock),		\
},
#else
#define INIT_PREV_CPUTIME(x)
#endif

#ifdef CONFIG_POSIX_TIMERS
#define INIT_CPU_TIMERS(s)						\
	.cpu_timers = {							\
		LIST_HEAD_INIT(s.cpu_timers[0]),			\
		LIST_HEAD_INIT(s.cpu_timers[1]),			\
		LIST_HEAD_INIT(s.cpu_timers[2]),			\
	},
#else
#define INIT_CPU_TIMERS(s)
#endif

#ifdef CONFIG_FIVE
# define INIT_TASK_INTEGRITY(integrity) {				\
	.user_value = INTEGRITY_NONE,					\
	.value = INTEGRITY_NONE,					\
	.usage_count = ATOMIC_INIT(1),					\
	.value_lock = __SPIN_LOCK_UNLOCKED(integrity.value_lock),	\
	.list_lock = __SPIN_LOCK_UNLOCKED(integrity.list_lock),		\
	.events = { .list = LIST_HEAD_INIT(integrity.events.list),},	\
}

# define INIT_INTEGRITY(tsk)						\
	.integrity = &init_integrity,
#else
# define INIT_INTEGRITY(tsk)
# define INIT_TASK_INTEGRITY(integrity)
#endif

#ifdef CONFIG_SCHED_MUQSS
#define INIT_TASK_COMM "MuQSS"
#else
#define INIT_TASK_COMM "swapper"
#endif

/* Attach to the init_task data structure for proper alignment */
#ifdef CONFIG_ARCH_TASK_STRUCT_ON_STACK
#define __init_task_data __attribute__((__section__(".data..init_task")))
#else
#define __init_task_data /**/
#endif

/* Attach to the thread_info data structure for proper alignment */
#define __init_thread_info __attribute__((__section__(".data..init_thread_info")))

#endif
