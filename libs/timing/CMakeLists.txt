# Copyright (c) 2019 The STE||AR-Group
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.3.2 FATAL_ERROR)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Default location is $HPX_ROOT/libs/timing/include
set(timing_headers
  hpx/timing/high_resolution_clock.hpp
  hpx/timing/high_resolution_timer.hpp
  hpx/timing/scoped_timer.hpp
  hpx/timing/steady_clock.hpp
)

# Default location is $HPX_ROOT/libs/timing/include_compatibility
set(timing_compat_headers
  hpx/util/high_resolution_clock.hpp
  hpx/util/high_resolution_timer.hpp
  hpx/util/scoped_timer.hpp
  hpx/util/steady_clock.hpp
)

# cmake will not create a separate VS project without any source files, thus
# this adds a dummy (empty) source file to the target
# Default location is $HPX_ROOT/libs/timing/src
set(timing_sources
)

include(HPX_AddModule)
add_hpx_module(timing
    COMPATIBILITY_HEADERS ON    # Added in 1.4.0
    DEPRECATION_WARNINGS
    FORCE_LINKING_GEN
    GLOBAL_HEADER_GEN ON
    SOURCES ${timing_sources}
    HEADERS ${timing_headers}
    COMPAT_HEADERS ${timing_compat_headers}
    DEPENDENCIES hpx_config
    CMAKE_SUBDIRS examples tests
)