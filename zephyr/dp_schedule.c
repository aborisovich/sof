// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright(c) 2023 Intel Corporation. All rights reserved.
 *
 * Author: Marcin Szkudlinski
 */

#include <sof/audio/component.h>
#include <sof/schedule/task.h>
#include <stdint.h>
#include <sof/schedule/dp_schedule.h>
#include <sof/schedule/ll_schedule_domain.h>
#include <sof/trace/trace.h>
#include <rtos/wait.h>
#include <rtos/interrupt.h>
#include <zephyr/kernel.h>
#include <zephyr/sys_clock.h>

#include <zephyr/kernel/thread.h>

LOG_MODULE_REGISTER(dp_schedule, CONFIG_SOF_LOG_LEVEL);
/* 87858bc2-baa9-40b6-8e4c-2c95ba8b1545 */
DECLARE_SOF_UUID("dp-schedule", dp_sched_uuid, 0x87858bc2, 0xbaa9, 0x40b6,
		 0x8e, 0x4c, 0x2c, 0x95, 0xba, 0x8b, 0x15, 0x45);

DECLARE_TR_CTX(dp_tr, SOF_UUID(dp_sched_uuid), LOG_LEVEL_INFO);

struct dp_schedule_data {
	struct list_item tasks;		/* list of active dp tasks */
	uint32_t num_tasks;		/* number of active dp tasks */
};

struct dp_task_pdata {
	k_tid_t thread_id;		/* zephyr thread ID */
	k_thread_stack_t *p_stack;	/* pointer to thread stack */
	uint32_t ticks_period;		/* period the task should be scheduled in LL ticks */
	uint32_t ticks_to_trigger;	/* number of ticks the task should be triggered after */
	struct k_sem sem;		/* semaphore for task scheduling */
};

/*
 * there's only one instance of DP scheduler for all cores
 * Keep pointer to it here
 */
static struct dp_schedule_data *dp_sch;

static inline void zephyr_dp_lock(uint32_t *flags)
{
	irq_local_disable(*flags);
}

static inline void zephyr_dp_unlock(uint32_t *flags)
{
	irq_local_enable(*flags);
}

/*
 * function called event LL tick, always on core 0
 *
 * TODO:
 * the scheduler should here calculate deadlines of all task and tell Zephyr about them
 * Currently there's an assumption that the task is always ready to run
 *
 */
void dp_scheduler_ll_tick(void)
{
	struct list_item *tlist;
	struct task *curr_task;
	struct dp_task_pdata *pdata;
	uint32_t lock_flags;

	if (!dp_sch)
		return;

	zephyr_dp_lock(&lock_flags);
	list_for_item(tlist, &dp_sch->tasks) {
		curr_task = container_of(tlist, struct task, list);
		pdata = curr_task->priv_data;

		if (curr_task->state == SOF_TASK_STATE_QUEUED) {
			if (pdata->ticks_to_trigger == 0) {
				pdata->ticks_to_trigger = pdata->ticks_period;
				curr_task->state = SOF_TASK_STATE_RUNNING;
				/* trigger the thread */
				k_sem_give(&pdata->sem);
			} else
				pdata->ticks_to_trigger--;
		}
	}
	zephyr_dp_unlock(&lock_flags);
}

static int scheduler_dp_task_cancel(void *data, struct task *task)
{
	(void)(data);
	uint32_t lock_flags;
	struct dp_task_pdata *pdata = task->priv_data;

	/* this is asyn cancel - mark the task to terminate */
	zephyr_dp_lock(&lock_flags);
	task->state = SOF_TASK_STATE_CANCEL;
	zephyr_dp_unlock(&lock_flags);

	/* release the thread to let it terminate itself */
	k_sem_give(&pdata->sem);

	return 0;
}

static int scheduler_dp_task_free(void *data, struct task *task)
{
	uint32_t lock_flags;
	struct dp_schedule_data *sch = data;
	struct dp_task_pdata *pdata = task->priv_data;

	/* abort the execution of the thread */
	k_thread_abort(pdata->thread_id);

	zephyr_dp_lock(&lock_flags);
	list_item_del(&task->list);
	sch->num_tasks--;
	task->priv_data = NULL;
	task->state = SOF_TASK_STATE_FREE;
	zephyr_dp_unlock(&lock_flags);

	/* free all allocated resources */
	rfree(pdata->p_stack);
	rfree(pdata->thread_id);
	rfree(pdata);

	return 0;
}

/* Thread function called in component context, on target core */
static void dp_thread_fn(void *p1, void *p2, void *p3)
{
	struct task *task = p1;
	struct dp_task_pdata *task_pdata = task->priv_data;
	uint32_t lock_flags;
	enum task_state state;

	do {
		/*
		 * the thread is started immediately after creation, it will stop on semaphore
		 * while in INIT state
		 */
		k_sem_take(&task_pdata->sem, K_FOREVER);

		if (task->state == SOF_TASK_STATE_RUNNING) {
			state = task_run(task);

			/*
			 * handle task state - the task may be canceled by external call
			 * or request termination itself
			 */
			zephyr_dp_lock(&lock_flags);

			if (task->state == SOF_TASK_STATE_RUNNING)
				task->state = state;

			switch (task->state) {
			case SOF_TASK_STATE_RESCHEDULE:
				task->state = SOF_TASK_STATE_QUEUED;
				break;


			case SOF_TASK_STATE_CANCEL:
				scheduler_dp_task_cancel(&dp_sch, task);

			case SOF_TASK_STATE_COMPLETED:
				task_complete(task);
				break;

			default:
				/* illegal state */
				assert(false);
			}

			zephyr_dp_unlock(&lock_flags);
		}
	} while (task_is_active(task) || (task->state == SOF_TASK_STATE_COMPLETED));

	/* zephyr will terminate a thread that exits this functon */
}

static int scheduler_dp_task_shedule(void *data, struct task *task, uint64_t start,
				     uint64_t period)
{
	struct dp_schedule_data *sch = data;
	struct dp_task_pdata *pdata = task->priv_data;
	struct list_item *tlist;
	struct task *curr_task;
	uint32_t lock_flags;

	zephyr_dp_lock(&lock_flags);
	/* check if task is already scheduled */
	list_for_item(tlist, &sch->tasks) {
		curr_task = container_of(tlist, struct task, list);

		/* keep original task */
		if (curr_task == task) {
			zephyr_dp_unlock(&lock_flags);
			return 0;
		}
	}

	if ((task->state != SOF_TASK_STATE_INIT) &&
	    (task->state != SOF_TASK_STATE_COMPLETED))
		return -EINVAL;

	/* calculate period and start time in LL ticks */
	pdata->ticks_period = ROUND_DOWN(period, LL_TIMER_PERIOD_US) / LL_TIMER_PERIOD_US;
	pdata->ticks_to_trigger = ROUND_DOWN(start, LL_TIMER_PERIOD_US) / LL_TIMER_PERIOD_US;

	if (start == 0) {
		/* trigger the task immediately, don't wait for LL tick */
		task->state = SOF_TASK_STATE_RUNNING;
		k_sem_give(&pdata->sem);
	} else {
		/* wait for tick */
		task->state = SOF_TASK_STATE_QUEUED;
	}

	/* add a task to DP scheduler list */
	list_item_prepend(&task->list, &sch->tasks);
	sch->num_tasks++;

	zephyr_dp_unlock(&lock_flags);

	return 0;
}

static struct scheduler_ops schedule_dp_ops = {
	.schedule_task		= scheduler_dp_task_shedule,
	.schedule_task_cancel	= scheduler_dp_task_cancel,
	.schedule_task_free	= scheduler_dp_task_free
};


int scheduler_dp_init_secondary_core(void)
{
	if (!dp_sch)
		return -ENOMEM;

	/* register the scheduler instance for secondary core */
	scheduler_init(SOF_SCHEDULE_DP, &schedule_dp_ops, dp_sch);

	return 0;
}

int scheduler_dp_init(void)
{
	dp_sch = rzalloc(SOF_MEM_ZONE_SYS_SHARED, 0, SOF_MEM_CAPS_RAM, sizeof(*dp_sch));
	if (!dp_sch)
		return -ENOMEM;

	list_init(&dp_sch->tasks);

	scheduler_init(SOF_SCHEDULE_DP, &schedule_dp_ops, dp_sch);

	return 0;
}

int scheduler_dp_task_init(struct task *task,
			   const struct sof_uuid_entry *uid,
			   enum task_state (*run)(void *data),
			   void *data,
			   uint16_t core,
			   size_t stack_size,
			   uint32_t task_priority)
{
	struct dp_task_pdata *pdata;
	struct k_thread *p_thread;
	k_tid_t thread_id;
	void *p_stack;
	int ret;

	if (task->priv_data)
		return -EEXIST;

	ret = schedule_task_init(
			task,
			uid,
			SOF_SCHEDULE_DP,
			0,
			run,
			data,
			core,
			0);
	if (ret < 0) {
		tr_err(&dp_tr, "zephyr_dp_task_init(): schedule_task_init failed");
		goto err;
	}

	/* allocate priv data space */
	pdata = rzalloc(SOF_MEM_ZONE_SYS_SHARED, 0, SOF_MEM_CAPS_RAM,
			sizeof(*pdata));
	if (!pdata) {
		tr_err(&dp_tr, "zephyr_dp_task_init(): pdata alloc failed");
		ret = -ENOMEM;
		goto err;
	}
	k_sem_init(&pdata->sem, 0, 1);

	/* allocate stack for a thread */
	stack_size = Z_KERNEL_STACK_SIZE_ADJUST(stack_size);
	p_stack = rballoc_align(0, SOF_MEM_CAPS_RAM, stack_size, Z_KERNEL_STACK_OBJ_ALIGN);
	if (!p_stack) {
		tr_err(&dp_tr, "zephyr_dp_task_init(): stack alloc failed");
		ret = -ENOMEM;
		goto err;
	}

	/* allocate zephyr thread  */
	p_thread = rzalloc(SOF_MEM_ZONE_SYS_SHARED, 0, SOF_MEM_CAPS_RAM,
			   sizeof(*p_thread));
	if (!p_thread) {
		tr_err(&dp_tr, "zephyr_dp_task_init(): thread alloc failed");
		ret = -ENOMEM;
		goto err;
	}

	/* create a zephyr thread for the task */
	thread_id = k_thread_create(p_thread,
				 p_stack,
				 stack_size,
				 dp_thread_fn,
				 task,
				 NULL,
				 NULL,
				 task_priority,
				 K_USER,
				 K_FOREVER);
	if (!thread_id)	{
		ret = -EFAULT;
		tr_err(&dp_tr, "zephyr_dp_task_init(): zephyr thread create failed");
		goto err;
	}
	/* pin the thread to specific core */
	ret = k_thread_cpu_pin(thread_id, task->core);
	if (ret < 0) {
		ret = -EFAULT;
		tr_err(&dp_tr, "zephyr_dp_task_init(): zephyr task pin to core failed");
		goto err;
	}

	/* success, fill the structures */
	task->priv_data = pdata;
	task->state = SOF_TASK_STATE_INIT;
	pdata->thread_id = thread_id;

	/* start the thread - it will immediately stop at a semaphore */
	k_thread_start(thread_id);

	return 0;
err:
	/* cleanup - free all allocated resources */
	if (p_stack)
		rfree(p_stack);
	if (thread_id)
		k_thread_abort(thread_id);
	if (p_thread)
		rfree(p_thread);
	if (pdata)
		rfree(pdata);
	return ret;
}
