/*
 * Copyright (c) 2023 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Author: Andrey Borisovich <andrey.borisovich@intel.com>
 */

#ifndef __SOF_DEBUG_MCPS_PROFILER_H__
#define __SOF_DEBUG_MCPS_PROFILER_H__

#include <autoconf.h>

#ifdef CONFIG_MCPS_PROFILER

#include <stdint.h>
#include <utilities/array.h>

struct mcps_profiler_data
{
	uint64_t sum;
	uint64_t first_overrun_timestamp;
	uint64_t cpc_to_cps_factor;
	uint32_t limit;
	uint32_t minimum;
	uint32_t maximum;
	uint32_t count;
	uint32_t nb_input_channel_pin0;
};

void mcps_profiler_init(size_t profiled_data_limit);

void mcps_profiler_init2(size_t profiled_data_limit, uint64_t cpc_to_cps_factor, size_t nb_input_channel);

struct mcps_profiler_data *const get_profiler_results();

#endif /* CONFIG_MCPS_PROFILER */
#endif /* __SOF_DEBUG_MCPS_PROFILER_H__ */
