/*! \file */
/* ************************************************************************
 * Copyright (c) 2019-2021 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * ************************************************************************ */

#pragma once
#ifndef ROCSPARSE_RANDOM_HPP
#define ROCSPARSE_RANDOM_HPP

#include "rocsparse_math.hpp"

#include <random>
#include <type_traits>

/* ==================================================================================== */
// Random number generator

using rocsparse_rng_t = std::mt19937;

void rocsparse_rng_set(rocsparse_rng_t a);

void rocsparse_seed_set(rocsparse_rng_t a);

void rocsparse_rng_nan_set(rocsparse_rng_t a);

rocsparse_rng_t& rocsparse_rng_get();

rocsparse_rng_t& rocsparse_seed_get();

rocsparse_rng_t& rocsparse_rng_nan_get();

// extern  rocsparse_rng_t rocsparse_rng, rocsparse_seed, rocsparse_rng_nan;

// Reset the seed (mainly to ensure repeatability of failures in a given suite)
inline void rocsparse_seedrand()
{
    rocsparse_rng_set(rocsparse_seed_get());
    rocsparse_rng_nan_set(rocsparse_seed_get());
}

/* ==================================================================================== */
/*! \brief  Random number generator which generates NaN values */
class rocsparse_nan_rng
{
    // Generate random NaN values
    template <typename T, typename UINT_T, int SIG, int EXP>
    static T random_nan_data()
    {
        static_assert(sizeof(UINT_T) == sizeof(T), "Type sizes do not match");
        union u_t
        {
            u_t() {}
            UINT_T u;
            T      fp;
        } x;
        do
            x.u = std::uniform_int_distribution<UINT_T>{}(rocsparse_rng_nan_get());
        while(!(x.u & (((UINT_T)1 << SIG) - 1))); // Reject Inf (mantissa == 0)
        x.u |= (((UINT_T)1 << EXP) - 1) << SIG; // Exponent = all 1's
        return x.fp; // NaN with random bits
    }

public:
    // Random integer
    template <typename T, typename std::enable_if<std::is_integral<T>{}, int>::type = 0>
    explicit operator T()
    {
        return std::uniform_int_distribution<T>{}(rocsparse_rng_nan_get());
    }

    // Random NaN double
    explicit operator double()
    {
        return random_nan_data<double, uint64_t, 52, 11>();
    }

    // Random NaN float
    explicit operator float()
    {
        return random_nan_data<float, uint32_t, 23, 8>();
    }

    explicit operator rocsparse_float_complex()
    {
        return {float(*this), float(*this)};
    }

    explicit operator rocsparse_double_complex()
    {
        return {double(*this), double(*this)};
    }
};

/* ==================================================================================== */
/* generate random number :*/

/*! \brief  generate a random number in range [a,b] */

template <typename T>
inline T random_generator_exact(int a = 1, int b = 10)
{
    return std::uniform_int_distribution<int>(a, b)(rocsparse_rng_get());
}

template <>
inline rocsparse_float_complex random_generator_exact<rocsparse_float_complex>(int a, int b)
{
    return rocsparse_float_complex(random_generator_exact<float>(a, b),
                                   random_generator_exact<float>(a, b));
}

template <>
inline rocsparse_double_complex random_generator_exact<rocsparse_double_complex>(int a, int b)
{
    return rocsparse_double_complex(random_generator_exact<double>(a, b),
                                    random_generator_exact<double>(a, b));
}

template <typename T, typename std::enable_if_t<std::is_integral<T>::value, bool> = true>
inline T random_generator(T a = static_cast<T>(1), T b = static_cast<T>(10))
{
    return random_generator_exact<T>(a, b);
}

template <typename T, typename std::enable_if_t<!std::is_integral<T>::value, bool> = true>
inline T random_generator(T a = static_cast<T>(0), T b = static_cast<T>(1))
{
    return std::uniform_real_distribution<T>(a, b)(rocsparse_rng_get());
}

template <>
inline rocsparse_float_complex random_generator<rocsparse_float_complex>(rocsparse_float_complex a,
                                                                         rocsparse_float_complex b)
{
    return rocsparse_float_complex(random_generator<float>(std::real(a), std::real(b)),
                                   random_generator<float>(std::imag(a), std::imag(b)));
}

template <>
inline rocsparse_double_complex
    random_generator<rocsparse_double_complex>(rocsparse_double_complex a,
                                               rocsparse_double_complex b)
{
    return rocsparse_double_complex(random_generator<double>(std::real(a), std::real(b)),
                                    random_generator<double>(std::imag(a), std::imag(b)));
}

/*! \brief generate a random normally distributed number around 0 with stddev 1 */
template <typename T>
inline T random_generator_normal()
{
    return std::normal_distribution<T>(0.0, 1.0)(rocsparse_rng_get());
}

#endif // ROCSPARSE_RANDOM_HPP
