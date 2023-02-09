/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright(c) 2023 Intel Corporation. All rights reserved.
 *
 * Author: Marcin Szkudlinski
 */

#ifndef __SOF_SCHEDULE_DP_SCHEDULE_H__
#define __SOF_SCHEDULE_DP_SCHEDULE_H__

#include <sof/schedule/task.h>
#include <sof/trace/trace.h>
#include <user/trace.h>
#include <stdint.h>

/**
 * \verbose
 *
 * DP scheduler is a scheduler that creates a separate preemptible Zephyr thread for each SOF task
 * There's only one instance of DP in the system, however, the threads may run on any core
 * Threads are pinned to specific core, there's no SMP processing.
 *
 * Once task is scheduled (schedule_dp_task is called) the task will be active in the system
 *
 * Task run() may return 3 values:
 *  SOF_TASK_STATE_RESCHEDULE - the task will be rescheduled as specified in scheduler period
 *  SOF_TASK_STATE_PENDING - the task will not be rescheduled till reschedule_task is called
 *  other states - the task will be terminated
 *
 * NOTE: task - means a SOF task
 *	 thread - means a Zephyr preemptible thread
 *
 * TODO  - EDF:
 * Threads run on the same priority, lower than thread running LL tasks. Zephyr EDF mechanism
 * is used for decision which thread/task is to be scheduled next. The DP scheduler calculates
 * the task deadline and set it in Zephyr thread properties, the final scheduling decision is made
 * by Zephyr.
 *
 * Each time tick the scheduler iterates through the list of all active tasks and calculates
 * a deadline based on
 *  - knowledge how the modules are binded
 *  - declared time required by a task to complete processing
 *  - the deadline of the last module
 *
 * TODO - task with budgets
 *  - task start at high priority, after given execution time the priority is dropped
 */

/**
 * \brief Init the Data Processing scheduler
 */
int scheduler_dp_init(void);

/**
 * \brief Set the Data Processing scheduler to be accessible at secondary cores
 */
int scheduler_dp_init_secondary_core(void);

/**
 * dp_scheduler_run should be run in LL scheduler loop, after all LL task have been executed
 */
void dp_scheduler_run(void);

/**
 * \brief initialize a DP task and add it to scheduling
 *
 * \param[io] task pointer to allocated memory space for task context
 * \param[in] uid pointer to UUID of the task
 * \param[in] run pointer to task function
 * \param[in] data pointer to the thread private data
 * \param[in] core CPU the thread should run on
 * \param[in] stack_size size of stack for a zephyr task
 * \param[in] task_priority priority of the zephyr task
 */
int scheduler_dp_task_init(struct task *task,
			   const struct sof_uuid_entry *uid,
			   enum task_state (*run)(void *data),
			   void *data,
			   uint16_t core,
			   size_t stack_size,
			   uint32_t task_priority);

#endif /* __SOF_SCHEDULE_EDF_SCHEDULE_H__ */
