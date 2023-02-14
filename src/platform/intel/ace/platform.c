// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2022 Intel Corporation. All rights reserved.
//
// Author: Liam Girdwood <liam.r.girdwood@linux.intel.com>
//         Keyon Jie <yang.jie@linux.intel.com>
//         Rander Wang <rander.wang@intel.com>
//         Janusz Jankowski <janusz.jankowski@linux.intel.com>

#include <sof/debug/debug.h>
#include <sof/ipc/common.h>
#include <sof/ipc/driver.h>
#include <sof/ipc/msg.h>
#include <sof/lib/agent.h>
#include <sof/lib/mm_heap.h>
#include <sof/lib/watchdog.h>
#include <sof/schedule/edf_schedule.h>
#include <sof/schedule/dp_schedule.h>
#include <sof/schedule/ll_schedule.h>
#include <sof/schedule/ll_schedule_domain.h>
#include <sof/trace/trace.h>
#include <ipc/header.h>
#include <ipc/info.h>
#include <kernel/abi.h>
#include <rtos/clk.h>
#include <adsp_memory.h>
#include <zephyr/drivers/mm/mm_drv_intel_adsp_mtl_tlb.h>
#include <zephyr/pm/pm.h>
#include <intel_adsp_ipc_devtree.h>
#include <sof/debug/panic.h>
#include <zephyr/pm/device.h>
#include <zephyr/pm/policy.h>

#include <sof_versions.h>
#include <stdint.h>

static const struct sof_ipc_fw_ready ready
	__section(".fw_ready") = {
	.hdr = {
		.cmd = SOF_IPC_FW_READY,
		.size = sizeof(struct sof_ipc_fw_ready),
	},
	.version = {
		.hdr.size = sizeof(struct sof_ipc_fw_version),
		.micro = SOF_MICRO,
		.minor = SOF_MINOR,
		.major = SOF_MAJOR,
/* opt-in; reproducible build by default */
#if BLD_COUNTERS
		.build = SOF_BUILD, /* See version-build-counter.cmake */
		.date = __DATE__,
		.time = __TIME__,
#else /* BLD_COUNTERS */
		.build = -1,
		.date = "dtermin.\0",
		.time = "fwready.\0",
#endif /* BLD_COUNTERS */
		.tag = SOF_TAG,
		.abi_version = SOF_ABI_VERSION,
		.src_hash = SOF_SRC_HASH,
	},
	.flags = DEBUG_SET_FW_READY_FLAGS,
};

int platform_boot_complete(uint32_t boot_message)
{
	struct ipc_cmd_hdr header;

	/* get any IPC specific boot message and optional data */
	ipc_boot_complete_msg(&header, 0);

	struct ipc_msg msg = {
		.header = header.pri,
		.extension = header.ext,
		.tx_size = sizeof(ready),
		.tx_data = (void *)&ready,
	};

	/* send fimrware ready message. */
	return ipc_platform_send_msg(&msg);
}

/* address where zephyr PM will save memory during D3 transition */
extern void *global_imr_ram_storage;

/* notifier called before every power state transition */
static void notify_pm_state_entry(enum pm_state state)
{
	size_t storage_buffer_size;

	if (state == PM_STATE_SOFT_OFF) {
		/* allocate IMR global_imr_ram_storage */
		const struct device *tlb_dev = DEVICE_DT_GET(DT_NODELABEL(tlb));

		__ASSERT_NO_MSG(tlb_dev);
		const struct intel_adsp_tlb_api *tlb_api =
				(struct intel_adsp_tlb_api *)tlb_dev->api;

		/* get HPSRAM storage buffer size */
		storage_buffer_size = tlb_api->get_storage_size();

		/* add space for LPSRAM */
		storage_buffer_size += LP_SRAM_SIZE;

		/* allocate IMR buffer and store it in the global pointer */
		global_imr_ram_storage = rmalloc(SOF_MEM_ZONE_SYS,
						 0,
						 SOF_MEM_CAPS_L3,
						 storage_buffer_size);

		/* IPC driver may not allow to change power state */
		__ASSERT(!pm_device_action_run(INTEL_ADSP_IPC_HOST_DEV, PM_DEVICE_ACTION_SUSPEND),
			"TODO: Prevent entering D3");
	}
}

/* notifier called after every power state transition */
static void notify_pm_state_exit(enum pm_state state)
{
	if (state == PM_STATE_SOFT_OFF)	{
		/* free global_imr_ram_storage */
		rfree(global_imr_ram_storage);
		global_imr_ram_storage = NULL;

		pm_device_action_run(INTEL_ADSP_IPC_HOST_DEV, PM_DEVICE_ACTION_RESUME);
		enum pm_device_state ipc_state = PM_DEVICE_STATE_SUSPENDING;
		pm_device_state_get(INTEL_ADSP_IPC_HOST_DEV, &ipc_state);
		__ASSERT_NO_MSG(ipc_state == PM_DEVICE_STATE_ACTIVE);

		// sends fw-ready message signalling successfull exit from D3 state
		platform_boot_complete(0);
	}
}

static struct pm_notifier pm_state_notifier = {
	.state_entry = notify_pm_state_entry,
	.state_exit = notify_pm_state_exit,
};

/* Runs on the primary core only */
int platform_init(struct sof *sof)
{
	int ret;

	trace_point(TRACE_BOOT_PLATFORM_CLOCK);
	platform_clock_init(sof);
	clock_set_freq(CLK_CPU(cpu_get_id()), CLK_MAX_CPU_HZ);

	trace_point(TRACE_BOOT_PLATFORM_SCHED);

	/* init low latency timer domain and scheduler. Any failure is fatal */
	sof->platform_timer_domain = zephyr_domain_init(PLATFORM_DEFAULT_CLOCK);
	ret = scheduler_init_ll(sof->platform_timer_domain);
	if (ret < 0)
		return ret;

	ret = scheduler_dp_init();
	if (ret < 0)
		return ret;

	/* init the system agent */
	trace_point(TRACE_BOOT_PLATFORM_AGENT);
	sa_init(sof, CONFIG_SYSTICK_PERIOD);

	/* init DMACs */
	trace_point(TRACE_BOOT_PLATFORM_DMA);
	ret = dmac_init(sof);
	if (ret < 0)
		return ret;

	/* register power states entry / exit notifiers */
	pm_notifier_register(&pm_state_notifier);

	/* initialize the host IPC mechanisms */
	trace_point(TRACE_BOOT_PLATFORM_IPC);
	ipc_init(sof);
	idc_init();

	watchdog_init();

	/* show heap status */
	heap_trace_all(1);

	return 0;
}

int platform_context_save(struct sof *sof)
{
	return 0;
}
