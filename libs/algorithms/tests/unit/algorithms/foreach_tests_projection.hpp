//  Copyright (c) 2014-2015 Hartmut Kaiser
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <hpx/include/parallel_algorithm.hpp>
#include <hpx/modules/testing.hpp>

#include <atomic>
#include <cstddef>
#include <iterator>
#include <utility>
#include <vector>

#include "test_utils.hpp"

////////////////////////////////////////////////////////////////////////////////
template <typename ExPolicy, typename IteratorTag, typename Proj>
void test_for_each(ExPolicy&& policy, IteratorTag, Proj&& proj)
{
    static_assert(
        hpx::parallel::execution::is_execution_policy<ExPolicy>::value,
        "hpx::parallel::execution::is_execution_policy<ExPolicy>::value");

    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<std::size_t> c(10007);
    std::fill(std::begin(c), std::end(c), std::size_t(42));

    std::atomic<std::size_t> count(0);

    iterator result = hpx::parallel::for_each(
        std::forward<ExPolicy>(policy), iterator(std::begin(c)),
        iterator(std::end(c)),
        [&count, &proj](std::size_t v) {
            HPX_TEST_EQ(v, proj(std::size_t(42)));
            ++count;
        },
        proj);

    HPX_TEST(result == iterator(std::end(c)));
    HPX_TEST_EQ(count, c.size());
}

template <typename ExPolicy, typename IteratorTag, typename Proj>
void test_for_each_async(ExPolicy&& p, IteratorTag, Proj&& proj)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<std::size_t> c(10007);
    std::fill(std::begin(c), std::end(c), std::size_t(42));

    std::atomic<std::size_t> count(0);

    hpx::future<iterator> f = hpx::parallel::for_each(
        std::forward<ExPolicy>(p), iterator(std::begin(c)),
        iterator(std::end(c)),
        [&count, &proj](std::size_t v) {
            HPX_TEST_EQ(v, proj(std::size_t(42)));
            ++count;
        },
        proj);
    f.wait();

    HPX_TEST(f.get() == iterator(std::end(c)));
    HPX_TEST_EQ(count, c.size());
}

////////////////////////////////////////////////////////////////////////////////
template <typename ExPolicy, typename IteratorTag, typename Proj>
void test_for_each_exception(ExPolicy policy, IteratorTag, Proj&& proj)
{
    static_assert(
        hpx::parallel::execution::is_execution_policy<ExPolicy>::value,
        "hpx::parallel::execution::is_execution_policy<ExPolicy>::value");

    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<std::size_t> c(10007);
    std::fill(std::begin(c), std::end(c), std::size_t(42));

    bool caught_exception = false;
    try
    {
        hpx::parallel::for_each(
            policy, iterator(std::begin(c)), iterator(std::end(c)),
            [](std::size_t v) { throw std::runtime_error("test"); }, proj);

        HPX_TEST(false);
    }
    catch (hpx::exception_list const& e)
    {
        caught_exception = true;
        test::test_num_exceptions<ExPolicy, IteratorTag>::call(policy, e);
    }
    catch (...)
    {
        HPX_TEST(false);
    }

    HPX_TEST(caught_exception);
}

template <typename ExPolicy, typename IteratorTag, typename Proj>
void test_for_each_exception_async(ExPolicy p, IteratorTag, Proj&& proj)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<std::size_t> c(10007);
    std::fill(std::begin(c), std::end(c), std::size_t(42));

    bool caught_exception = false;
    bool returned_from_algorithm = false;
    try
    {
        hpx::future<iterator> f = hpx::parallel::for_each(
            p, iterator(std::begin(c)), iterator(std::end(c)),
            [](std::size_t v) { throw std::runtime_error("test"); }, proj);
        returned_from_algorithm = true;
        f.get();

        HPX_TEST(false);
    }
    catch (hpx::exception_list const& e)
    {
        caught_exception = true;
        test::test_num_exceptions<ExPolicy, IteratorTag>::call(p, e);
    }
    catch (...)
    {
        HPX_TEST(false);
    }

    HPX_TEST(caught_exception);
    HPX_TEST(returned_from_algorithm);
}

////////////////////////////////////////////////////////////////////////////////
template <typename ExPolicy, typename IteratorTag, typename Proj>
void test_for_each_bad_alloc(ExPolicy policy, IteratorTag, Proj&& proj)
{
    static_assert(
        hpx::parallel::execution::is_execution_policy<ExPolicy>::value,
        "hpx::parallel::execution::is_execution_policy<ExPolicy>::value");

    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<std::size_t> c(10007);
    std::fill(std::begin(c), std::end(c), std::size_t(42));

    bool caught_exception = false;
    try
    {
        hpx::parallel::for_each(
            policy, iterator(std::begin(c)), iterator(std::end(c)),
            [](std::size_t v) { throw std::bad_alloc(); }, proj);

        HPX_TEST(false);
    }
    catch (std::bad_alloc const&)
    {
        caught_exception = true;
    }
    catch (...)
    {
        HPX_TEST(false);
    }

    HPX_TEST(caught_exception);
}

template <typename ExPolicy, typename IteratorTag, typename Proj>
void test_for_each_bad_alloc_async(ExPolicy p, IteratorTag, Proj&& proj)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<std::size_t> c(10007);
    std::fill(std::begin(c), std::end(c), std::size_t(42));

    bool caught_exception = false;
    bool returned_from_algorithm = false;
    try
    {
        hpx::future<iterator> f = hpx::parallel::for_each(
            p, iterator(std::begin(c)), iterator(std::end(c)),
            [](std::size_t v) { throw std::bad_alloc(); }, proj);
        returned_from_algorithm = true;
        f.get();

        HPX_TEST(false);
    }
    catch (std::bad_alloc const&)
    {
        caught_exception = true;
    }
    catch (...)
    {
        HPX_TEST(false);
    }

    HPX_TEST(caught_exception);
    HPX_TEST(returned_from_algorithm);
}
