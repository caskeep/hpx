//  Copyright (c) 2019 National Technology & Engineering Solutions of Sandia,
//                     LLC (NTESS).
//  Copyright (c) 2018-2020 Hartmut Kaiser
//  Copyright (c) 2018-2019 Adrian Serio
//  Copyright (c) 2019-2020 Nikunj Gupta
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <hpx/resiliency/resiliency_cpos.hpp>
#include <hpx/resiliency/util.hpp>

#include <hpx/async_distributed/async.hpp>
#include <hpx/futures/future.hpp>
#include <hpx/modules/async_local.hpp>

#include <cstddef>
#include <exception>
#include <stdexcept>
#include <utility>
#include <vector>

namespace hpx { namespace resiliency { namespace experimental {

    ///////////////////////////////////////////////////////////////////////////
    namespace detail {

        ///////////////////////////////////////////////////////////////////////
        template <typename Vote, typename Pred, typename F, typename... Ts>
        hpx::future<
            typename hpx::util::detail::invoke_deferred_result<F, Ts...>::type>
        async_replicate_vote_validate(
            std::size_t n, Vote&& vote, Pred&& pred, F&& f, Ts&&... ts)
        {
            using result_type =
                typename hpx::util::detail::invoke_deferred_result<F,
                    Ts...>::type;

            // launch given function n times
            std::vector<hpx::future<result_type>> results;
            results.reserve(n);

            for (std::size_t i = 0; i != n; ++i)
            {
                results.emplace_back(hpx::async(f, ts...));
            }

            // wait for all threads to finish executing and return the first
            // result that passes the predicate, properly handle exceptions
            return hpx::dataflow(
                // do not schedule new thread for the lambda
                hpx::launch::sync,
                [pred = std::forward<Pred>(pred),
                    vote = std::forward<Vote>(vote),
                    n](std::vector<hpx::future<result_type>>&& results) mutable
                -> result_type {
                    // Store all valid results
                    std::vector<result_type> valid_results;
                    valid_results.reserve(n);

                    std::exception_ptr ex;

                    for (auto&& f : std::move(results))
                    {
                        if (f.has_exception())
                        {
                            // rethrow abort_replicate_exception, if caught
                            ex = detail::rethrow_on_abort_replicate(f);
                        }
                        else
                        {
                            auto&& result = f.get();
                            if (hpx::util::invoke(pred, result))
                            {
                                valid_results.emplace_back(std::move(result));
                            }
                        }
                    }

                    if (!valid_results.empty())
                    {
                        return hpx::util::invoke(
                            std::forward<Vote>(vote), std::move(valid_results));
                    }

                    if (bool(ex))
                        std::rethrow_exception(ex);

                    // throw aborting exception no correct results ere produced
                    throw abort_replicate_exception{};
                },
                std::move(results));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // Asynchronously launch given function \a f exactly \a n times. Verify
    // the result of those invocations using the given predicate \a pred.
    // Run all the valid results against a user provided voting function.
    // Return the valid output.
    template <typename Vote, typename Pred, typename F, typename... Ts>
    hpx::future<
        typename hpx::util::detail::invoke_deferred_result<F, Ts...>::type>
    tag_invoke(async_replicate_vote_validate_t, std::size_t n, Vote&& vote,
        Pred&& pred, F&& f, Ts&&... ts)
    {
        return detail::async_replicate_vote_validate(n,
            std::forward<Vote>(vote), std::forward<Pred>(pred),
            std::forward<F>(f), std::forward<Ts>(ts)...);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Asynchronously launch given function \a f exactly \a n times. Verify
    // the result of those invocations using the given predicate \a pred. Run
    // all the valid results against a user provided voting function.
    // Return the valid output.
    template <typename Vote, typename F, typename... Ts>
    hpx::future<
        typename hpx::util::detail::invoke_deferred_result<F, Ts...>::type>
    tag_invoke(
        async_replicate_vote_t, std::size_t n, Vote&& vote, F&& f, Ts&&... ts)
    {
        return detail::async_replicate_vote_validate(n,
            std::forward<Vote>(vote), detail::replicate_validator{},
            std::forward<F>(f), std::forward<Ts>(ts)...);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Asynchronously launch given function \a f exactly \a n times. Verify
    // the result of those invocations using the given predicate \a pred.
    // Return the first valid result.
    template <typename Pred, typename F, typename... Ts>
    hpx::future<
        typename hpx::util::detail::invoke_deferred_result<F, Ts...>::type>
    tag_invoke(async_replicate_validate_t, std::size_t n, Pred&& pred, F&& f,
        Ts&&... ts)
    {
        return detail::async_replicate_vote_validate(n,
            detail::replicate_voter{}, std::forward<Pred>(pred),
            std::forward<F>(f), std::forward<Ts>(ts)...);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Asynchronously launch given function \a f exactly \a n times. Verify
    // the result of those invocations by checking for exception.
    // Return the first valid result.
    template <typename F, typename... Ts>
    hpx::future<
        typename hpx::util::detail::invoke_deferred_result<F, Ts...>::type>
    tag_invoke(async_replicate_t, std::size_t n, F&& f, Ts&&... ts)
    {
        return detail::async_replicate_vote_validate(n,
            detail::replicate_voter{}, detail::replicate_validator{},
            std::forward<F>(f), std::forward<Ts>(ts)...);
    }

}}}    // namespace hpx::resiliency::experimental
