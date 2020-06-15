//  Copyright (c) 2019 Auriane Reverdell
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <hpx/config.hpp>
#include <hpx/hashing/config/defines.hpp>
#include <hpx/hashing/fibhash.hpp>

#if HPX_HASHING_HAVE_DEPRECATION_WARNINGS
#if defined(HPX_MSVC)
#pragma message("The header hpx/util/fibhash.hpp is deprecated, \
    please include hpx/hashing/fibhash.hpp instead")
#else
#warning "The header hpx/util/fibhash.hpp is deprecated, \
    please include hpx/hashing/fibhash.hpp instead"
#endif
#endif