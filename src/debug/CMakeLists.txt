# SPDX-License-Identifier: BSD-3-Clause

if(CONFIG_GDB_DEBUG)
	add_subdirectory(gdb)
endif()

if(CONFIG_MCPS_PROFILER)
	add_subdirectory(perf)
endif()

add_local_sources(sof panic.c)
