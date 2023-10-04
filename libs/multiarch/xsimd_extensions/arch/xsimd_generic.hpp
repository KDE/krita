/*
 * SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef KIS_XSIMD_GENERIC_HPP
#define KIS_XSIMD_GENERIC_HPP

#include "xsimd_generic_details.hpp"

#include <array>
#include <type_traits>

namespace xsimd
{
/***********************
 * Truncate-initialize *
 ***********************/

template<typename V, typename T, typename A>
inline batch<T, A> truncate_to_type(xsimd::batch<T, A> const &self) noexcept
{
    return kernel::detail::apply_with_value(
        [](float i) -> float {
            if (std::numeric_limits<V>::min() > i) {
                return 0;
            } else if (std::numeric_limits<V>::max() < i) {
                return 0;
            } else {
                return static_cast<V>(i);
            }
        },
        self);
}

// Mask to 0 elements of a vector.
template<typename T, typename A>
inline auto set_zero(const batch<T, A> &src, const batch_bool<T, A> &mask) noexcept
{
    return xsimd::select(mask, xsimd::batch<T, A>(0), src);
}

// Mask to 1 elements of a vector.
template<typename T, typename A>
inline auto set_one(const batch<T, A> &src, const batch_bool<T, A> &mask) noexcept
{
    return xsimd::select(mask, xsimd::batch<T, A>(1), src);
}

/**********************************
 * Sign-extending unaligned loads *
 **********************************/

// Load `T::size` values from the array of `T2` elements.
template<typename T, typename T2>
inline T load_and_extend(const T2 *src) noexcept
{
    return kernel::detail::apply_with_index_and_value(
        [&](size_t i, typename T::value_type) {
            return static_cast<typename T::value_type>(src[i]);
        },
        T{});
}

/*************************************************
 * Type-inferred, auto-aligned memory allocation *
 *************************************************/

// Allocate size bytes of memory aligned to `batch<T, A>::alignment()`.
template<typename T, typename A>
inline T *aligned_malloc(size_t size) noexcept
{
    using T_v = batch<T, A>;

    return reinterpret_cast<T *>(xsimd::aligned_malloc(size, T_v::arch_type::alignment()));
}

// Return the maximum of a list of templated values at compile time.
template<size_t value, size_t... values>
constexpr typename std::enable_if<sizeof...(values) == 0, size_t>::type max()
{
    return value;
}

// Return the maximum of a list of templated values at compile time.
template<size_t value, size_t... values>
constexpr typename std::enable_if<sizeof...(values) != 0, size_t>::type max()
{
    return std::max(value, max<values...>());
}

// Allocate memory for `sz` T items, aligned to the selected architecture's
// alignment.
template<typename T, typename A>
inline T *vector_aligned_malloc(size_t sz) noexcept
{
    return static_cast<T *>(xsimd::aligned_malloc(sz * sizeof(T), A::alignment()));
}

// Free allocated memory, hiding the `const_cast` if necessary.
template<typename T>
inline void vector_aligned_free(const T *ptr) noexcept
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return xsimd::aligned_free(const_cast<T *>(ptr));
}

/****************
 * Interleaving *
 ****************/

// Return the tuple of interleaved batches `a` and `b`.
// First element is the low half, second is the upper half.
template<typename V>
inline std::pair<V, V> interleave(const V &a, const V &b) noexcept
{
    return {xsimd::zip_lo(a, b), xsimd::zip_hi(a, b)};
}

/**********************
 * Quadratic function *
 **********************/

template<typename T, typename A>
inline batch<T, A> pow2 (batch<T, A> const& self) noexcept
{
    return self * self;
}

#if XSIMD_VERSION_MAJOR <= 10

template <class B, class T, class A>
inline batch<B, A> bitwise_cast_compat(batch<T, A> const& x) noexcept
{
    return bitwise_cast<batch<B, A>>(x);
}

#else

template <class B, class T, class A>
inline batch<B, A> bitwise_cast_compat(batch<T, A> const& x) noexcept
{
    return bitwise_cast<B>(x);
}


#endif

}; // namespace xsimd

#endif
