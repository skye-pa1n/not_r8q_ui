// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023-2024 Sultan Alsawaf <sultan@kerneltoast.com>.
 */

/**
 * DOC: Capacity Aware Superset Scheduler (CASS) description
 *
 * The Capacity Aware Superset Scheduler (CASS) optimizes runqueue selection of
 * CFS tasks. By using CPU capacity as a basis for comparing the relative
 * utilization between different CPUs, CASS fairly balances load across CPUs of
 * varying capacities. This results in improved multi-core performance,
 * especially when CPUs are overutilized because CASS doesn't clip a CPU's
 * utilization when it eclipses the CPU's capacity.
 *
 * As a superset of capacity aware scheduling, CASS implements a hierarchy of
 * criteria to determine the better CPU to wake a task upon between CPUs that
 * have the same relative utilization. This way, single-core performance,
 * latency, and cache affinity are all optimized where possible.
 */

struct cass_cpu_cand {
	int cpu;
	unsigned int exit_lat;
	unsigned long cap;
	unsigned long util;
        unsigned long raw_util;
};

static __always_inline
unsigned long cass_cpu_util(int cpu, bool sync)
{
	struct cfs_rq *cfs_rq = &cpu_rq(cpu)->cfs;
	unsigned long util = READ_ONCE(cfs_rq->avg.util_avg);

	/* Deduct @current's util from this CPU if this is a sync wake */
	if (sync && cpu == smp_processor_id())
		lsub_positive(&util, task_util(current));

	if (sched_feat(UTIL_EST))
		util = max_t(unsigned long, util,
			     READ_ONCE(cfs_rq->avg.util_est.enqueued));

	return util;
}

/* Returns true if @a is a better CPU than @b */
static __always_inline
bool cass_cpu_better(const struct cass_cpu_cand *a,
		     const struct cass_cpu_cand *b, int prev_cpu, 
		     bool sync, unsigned long energy[NR_CPUS], struct task_struct *p)
{
#define cass_cmp_r(a, b, c) ({ res = ((a) - (b)) * (abs((a) - (b)) > (c)); })
#define cass_cmp(a, b) ({ res = (a) - (b); })
#define cass_eq(a, b) ({ res = (a) == (b); })
	long res;
	int boost = schedtune_task_boost(p);
	int latency_sensitive = schedtune_prefer_high_cap(p);

	/* Prefer the CPU with higher cap and lower utilization */
	if (boost && cass_cmp_r(a->cap - a->raw_util, b->cap - b->raw_util, 64))
		goto done;

	/* Prefer the current CPU for sync wakes */
	if (sync && (cass_eq(a->cpu, smp_processor_id()) ||
		     !cass_cmp(b->cpu, smp_processor_id())))
		goto done;

	/* Prefer the CPU with lower idle exit latency */
	if (cass_cmp_r(b->exit_lat, a->exit_lat, 1 + !latency_sensitive * 99))
		goto done;

	/* Prefer lower energy consumption CPU */
	if (cass_cmp_r(energy[b->cpu], energy[a->cpu], energy[b->cpu] >> 4))
		goto done;

	/* Prefer the previous CPU */
	if (cass_eq(a->cpu, prev_cpu) || !cass_cmp(b->cpu, prev_cpu))
		goto done;

	/* Prefer the CPU that shares a cache with the previous CPU */
	if (cass_cmp(cpus_share_cache(a->cpu, prev_cpu),
		     cpus_share_cache(b->cpu, prev_cpu)))
		goto done;

	/* Prefer the CPU with lower relative utilization */
	if (cass_cmp(b->util, a->util))
		goto done;

	/* Prefer the CPU with higher capacity */
	if (cass_cmp(a->cap, b->cap))
		goto done;

        /* Prefer the CPU with lower idle exit latency */
        if (cass_cmp(b->exit_lat, a->exit_lat))
                goto done;

	/* Prefer lower energy consumption CPU */
	if (cass_cmp(energy[b->cpu], energy[a->cpu]))
		goto done;

	/* @a isn't a better CPU than @b. @res must be <=0 to indicate such. */
done:
	/* @a is a better CPU than @b if @res is positive */
	return res > 0;
}

static int cass_best_cpu(struct task_struct *p, int prev_cpu, bool sync)
{
	/* Initialize @best such that @best always has a valid CPU at the end */
	struct cass_cpu_cand cands[2], *best = cands, *curr;
	struct rq *rq = cpu_rq(smp_processor_id());
	unsigned long energy[NR_CPUS] = {0};
	struct cpuidle_state *idle_state;
	cpumask_t candidates = {};
	struct perf_domain *pd;
	bool has_idle = false;
	unsigned long p_util;
	int cidx = 0, cpu;

	/* Get candidate CPUs */
	cpumask_and(&candidates, &p->cpus_allowed, cpu_active_mask);

	/* Calculate energy of candidate cpu */
	pd = rcu_dereference(rq->rd->pd);
	if (pd)
		compute_energy_change(p, pd, prev_cpu, &candidates, energy);

	/* Get the utilization for this task */
	p_util = task_util_est(p);

	/*
	 * Find the best CPU to wake @p on. Although idle_get_state() requires
	 * an RCU read lock, an RCU read lock isn't needed because we're not
	 * preemptible and RCU-sched is unified with normal RCU. Therefore,
	 * non-preemptible contexts are implicitly RCU-safe.
	 */
	for_each_cpu(cpu, &candidates) {
		/* Use the free candidate slot */
		curr = &cands[cidx];
		curr->cpu = cpu;

		/*
		 * Check if this CPU is idle or only has SCHED_IDLE tasks. For
		 * sync wakes, treat the current CPU as idle if @current is the
		 * only running task.
		 */
		if ((sync && cpu == smp_processor_id() && rq->nr_running == 1) ||
		    available_idle_cpu(cpu) || sched_idle_cpu(cpu)) {
			/* Discard any previous non-idle candidate */
			if (!has_idle) {
				best = curr;
				cidx ^= 1;
			}
			has_idle = true;

			/* Nonzero exit latency indicates this CPU is idle */
			curr->exit_lat = 1;

			/* Add on the actual idle exit latency, if any */
			idle_state = idle_get_state(cpu_rq(cpu));
			if (idle_state)
				curr->exit_lat += idle_state->exit_latency;
		} else {
			/* Skip non-idle CPUs if there's an idle candidate */
			if (has_idle)
				continue;

			/* Zero exit latency indicates this CPU isn't idle */
			curr->exit_lat = 0;
		}

		/* Get this CPU's utilization, possibly without @current */
		curr->raw_util = cass_cpu_util(cpu, sync);

		/*
		 * Add @p's utilization to this CPU if it's not @p's CPU, to
		 * find what this CPU's relative utilization would look like
		 * if @p were on it.
		 */
		if (cpu != task_cpu(p))
			curr->raw_util += p_util;

		/*
		 * Get the current capacity of this CPU adjusted for thermal
		 * pressure as well as IRQ and RT-task time.
		 */
		curr->cap = capacity_of(cpu);

		/* Calculate the relative utilization for this CPU candidate */
		curr->util = curr->raw_util * SCHED_CAPACITY_SCALE / curr->cap;

		/* If @best == @curr then there's no need to compare them */
		if (best == curr)
			continue;

		/* Check if this CPU is better than the best CPU found */
		if (cass_cpu_better(curr, best, prev_cpu, sync, energy, p)) {
			best = curr;
			cidx ^= 1;
		}
	}

	return best->cpu;
}

static int cass_select_task_rq_fair(struct task_struct *p, int prev_cpu,
				    int sd_flag, int wake_flags,
				    int sibling_count_hint)
{
	bool sync;

	/* Don't balance on exec since we don't know what @p will look like */
	if (sd_flag & SD_BALANCE_EXEC)
		return prev_cpu;

	/*
	 * If there aren't any valid CPUs which are active, then just return the
	 * first valid CPU since it's possible for certain types of tasks to run
	 * on inactive CPUs.
	 */
	if (unlikely(!cpumask_intersects(&p->cpus_allowed, cpu_active_mask)))
		return cpumask_first(&p->cpus_allowed);

	/* cass_best_cpu() needs the task's utilization, so sync it up */
	if (!(sd_flag & SD_BALANCE_FORK))
		sync_entity_load_avg(&p->se);

	sync = (wake_flags & WF_SYNC) && !(current->flags & PF_EXITING);
	return cass_best_cpu(p, prev_cpu, sync);
}
