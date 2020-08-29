//  Copyright (c) 2020 Hartmut Kaiser
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/config.hpp>

#if !defined(HPX_COMPUTE_DEVICE_CODE)

#include <hpx/collectives/detail/communication_set_node.hpp>
#include <hpx/exception.hpp>
#include <hpx/modules/execution.hpp>
#include <hpx/modules/futures.hpp>
#include <hpx/modules/iterator_support.hpp>
#include <hpx/parallel/container_algorithms/count.hpp>
#include <hpx/runtime/basename_registration.hpp>
#include <hpx/runtime/components/component_factory.hpp>
#include <hpx/runtime/components/new.hpp>
#include <hpx/runtime/components/server/component.hpp>
#include <hpx/runtime/naming/id_type.hpp>
#include <hpx/runtime_local/get_num_localities.hpp>

#include <cstddef>
#include <string>
#include <utility>

///////////////////////////////////////////////////////////////////////////////
using communication_set_node_component =
    hpx::components::component<hpx::lcos::detail::communication_set_node>;

HPX_REGISTER_COMPONENT(communication_set_node_component);

namespace hpx { namespace lcos { namespace detail {

    std::size_t calculate_connected_node(std::size_t site, std::size_t arity)
    {
        if (site < arity)
        {
            return 0;
        }
        return ((site - arity) / arity) * arity;
    }

    std::size_t calculate_num_connected(
        std::size_t num_sites, std::size_t site, std::size_t arity)
    {
        std::size_t num_children =
            hpx::ranges::count(hpx::parallel::execution::par,
                hpx::util::make_counting_iterator(site + 1),
                hpx::util::make_counting_iterator(num_sites), site,
                [&](std::size_t node) {
                    return calculate_connected_node(node, arity);
                });
        return num_children + 1;
    }

    ///////////////////////////////////////////////////////////////////////////
    communication_set_node::communication_set_node()    //-V730
      : arity_(0)
      , num_connected_(0)
      , num_sites_(0)
      , site_(0)
      , connect_to_(0)
      , needs_initialization_(false)
    {
        HPX_ASSERT(false);    // shouldn't ever be called
    }

    communication_set_node::communication_set_node(std::size_t num_sites,
        std::string name, std::size_t site, std::size_t arity)
      : name_(name)
      , arity_(arity)
      , num_connected_(calculate_num_connected(num_sites, site, arity))
      , num_sites_(num_sites)
      , site_(site)
      , connect_to_(calculate_connected_node(site, arity))
      , needs_initialization_(true)
      , gate_(num_connected_)
    {
        HPX_ASSERT(num_connected_ != 0);
        HPX_ASSERT(num_sites_ != 0);

        // node zero does not connect to any other node
        if (connect_to_ != 0)
        {
            connected_node_ =
                hpx::find_from_basename(std::move(name), connect_to_);
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<hpx::id_type> register_communication_set_name(
        hpx::future<hpx::id_type>&& f, std::string basename, std::size_t site)
    {
        hpx::id_type target = f.get();

        // Register unmanaged id to avoid cyclic dependencies, unregister
        // is done after all data has been collected in the component above.
        hpx::future<bool> result =
            hpx::register_with_basename(basename, target, site);

        return result.then(hpx::launch::sync,
            [target = std::move(target), basename = std::move(basename)](
                hpx::future<bool>&& f) -> hpx::id_type {
                bool result = f.get();
                if (!result)
                {
                    HPX_THROW_EXCEPTION(bad_parameter,
                        "hpx::lcos::detail::register_communication_set_name",
                        "the given base name for the communication_set_node "
                        "operation was already registered: " +
                            basename);
                }
                return target;
            });
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<hpx::id_type> create_communication_set_node(
        char const* basename, std::size_t num_sites, std::size_t this_site,
        std::size_t arity)
    {
        // we create communication nodes for base participants only
        HPX_ASSERT((this_site % arity) == 0);

        std::string name(basename);

        // create a new communicator_server
        hpx::future<hpx::id_type> id =
            hpx::local_new<detail::communication_set_node>(
                num_sites, name, this_site, arity);

        // register the communicator's id using the given basename
        return id.then(hpx::launch::sync,
            util::bind_back(&detail::register_communication_set_name,
                std::move(name), this_site));
    }
}}}    // namespace hpx::lcos::detail

#endif
