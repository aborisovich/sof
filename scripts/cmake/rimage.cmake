# SPDX-License-Identifier: BSD-3-Clause

WestDependency_Declare(rimage)
# find_package(rimage HINTS
#     "${sof_top_dir}rimage"
#     REQUIRED)

include(FetchContent)
# TODO: Move this dependency resolution to rimage project!
FetchContent_Declare(rimage SOURCE_DIR "${sof_top_dir}rimage")
FetchContent_MakeAvailable(rimage)
