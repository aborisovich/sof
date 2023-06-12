/*
 * Copyright (c) 2023 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Author: Andrey Borisovich <andrey.borisovich@intel.com>
 */

#include <sof/debug/perf/mcps_profiler.h>
#include <xtensa/core-macros.h>
#include <rtos/string.h>

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

static struct mcps_profiler_data profiler_data;
static size_t ccount_init;
static bool running;
static uint64_t start_time;

void mcps_profiler_init(size_t profiled_data_limit)
{
    memset(&profiler_data, 0, sizeof(profiler_data));
    profiler_data.minimum = UINT32_MAX;
    profiler_data.maximum = 0;
    profiler_data.limit = profiled_data_limit;
    profiler_data.nb_input_channel_pin0 = 1;
    profiler_data.first_overrun_timestamp = 0;
    start_time = 0;
}

void mcps_profiler_init2(size_t profiled_data_limit, uint64_t cpc_to_cps_factor, size_t nb_input_channel)
{
    mcps_profiler_init(profiled_data_limit);
    if (nb_input_channel != 0)
        profiler_data.nb_input_channel_pin0 = nb_input_channel;
    profiler_data.cpc_to_cps_factor = cpc_to_cps_factor;
}

void mcps_profiler_start(bool clear_cache)
{
    if(clear_cache)
    {
        xthal_icache_all_invalidate();
        xthal_dcache_all_writeback();
        xthal_dcache_all_invalidate();
    }
    ccount_init = xthal_get_ccount();
    start_time = k_uptime_get();
}

static void profiler_update_data(size_t profiled_data_value)
{
    if (profiled_data_value >= profiler_data.limit && profiler_data.first_overrun_timestamp == 0)
        profiler_data.first_overrun_timestamp = k_uptime_delta(&start_time);

    if (profiled_data_value < profiler_data.minimum) {
        profiler_data.minimum = profiled_data_value;
    }

    if (profiled_data_value > profiler_data.maximum) {
        profiler_data.maximum = profiled_data_value;
    }

    // Stop to compute average if the max count has been reached
    if (profiler_data.count != UINT32_MAX) {
        profiler_data.count++;
        profiler_data.sum += profiled_data_value;
    }
}

struct mcps_profiler_data *const get_profiler_results()
{
    profiler_update_data(xthal_get_ccount() - ccount_init);
    return &profiler_data;
}
