/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISOPTIONTUPLE_H
#define KISOPTIONTUPLE_H

#include <KisMpl.h>
#include <boost/operators.hpp>
#include <boost/tti/has_static_member_data.hpp>

namespace detail {

BOOST_TTI_HAS_STATIC_MEMBER_DATA(supports_prefix)

/**
 * A simple metafunction that detects the presence of
 * 'supports_prefix' and checks its value.
 */
template <typename T, bool has_member = has_static_member_data_supports_prefix<T, const bool>::value>
struct supports_prefix;

template <typename T>
struct supports_prefix<T, false>
    : std::bool_constant<false>
{
};

template <typename T>
struct supports_prefix<T, true>
    : std::bool_constant<T::supports_prefix>
{
};

/**
 * Check if all the passed types define 'support_prefix = true'
 */
template <typename... T>
struct all_support_prefix
{
    static constexpr bool value = (supports_prefix<T>::value && ...);
};

/**
 * Check if none the passed types defines 'support_prefix = true'
 */
template <typename... T>
struct none_support_prefix
{
    static constexpr bool value = (!supports_prefix<T>::value && ...);
};

template <bool allSupportPrefix, bool noneSupportPrefix, typename... Data>
struct KisOptionTupleImpl
{
    static_assert (allSupportPrefix || noneSupportPrefix,
        "Either **all** or **none** arguments of KisOptionTuple should define 'supports_prefix'");
};

/**
 * A prefix-supporting implementation of KisOptionTuple
 */
template <typename FirstData, typename... RestData>
struct KisOptionTupleImpl<true, false, FirstData, RestData...>
    : public FirstData,
      public RestData...,
      public boost::equality_comparable<KisOptionTupleImpl<true, false, FirstData, RestData...>>
{
    template <typename... T>
    KisOptionTupleImpl(const QString &prefix, T... args)
        : FirstData(prefix, std::forward<decltype(args)>(args)...),
          RestData(prefix)...
    {
    }

    inline friend bool operator==(const KisOptionTupleImpl<true, false, FirstData, RestData...> &lhs,
                                  const KisOptionTupleImpl<true, false, FirstData, RestData...> &rhs)
    {

        return ((static_cast<const FirstData&>(lhs) == static_cast<const FirstData&>(rhs)) && ... &&
                (static_cast<const RestData&>(lhs) == static_cast<const RestData&>(rhs)));
    }

    bool read(const KisPropertiesConfiguration *setting)
    {
        return (static_cast<FirstData&>(*this).read(setting) && ... &&
                static_cast<RestData&>(*this).read(setting));
    }

    void write(KisPropertiesConfiguration *setting) const
    {
        (static_cast<const FirstData&>(*this).write(setting) , ... ,
                static_cast<const RestData&>(*this).write(setting));
    }

};

/**
 * An implementation of KisOptionTuple without prefix support
 */
template <typename... Data>
struct KisOptionTupleImpl<false, true, Data...>
    : public Data...,
      public boost::equality_comparable<KisOptionTupleImpl<false, true, Data...>>
{
    template <typename... T>
    KisOptionTupleImpl(T... args)
        : kismpl::first_type_t<Data...>(std::forward<decltype(args)>(args)...)
    {
    }

    inline friend bool operator==(const KisOptionTupleImpl<false, true, Data...> &lhs,
                                  const KisOptionTupleImpl<false, true, Data...> &rhs)
    {

        return ((static_cast<const Data&>(lhs) == static_cast<const Data&>(rhs)) && ... );
    }

    bool read(const KisPropertiesConfiguration *setting)
    {
        return (static_cast<Data&>(*this).read(setting) && ... );
    }

    void write(KisPropertiesConfiguration *setting) const
    {
        (static_cast<const Data&>(*this).write(setting) , ... );
    }
};

} // namespace detail

/**
 * KisOptionTuple is a class that merges multiple option
 * data structs into one by inheriting from all of them.
 *
 * KisOptionTuple automatically generates read, write and
 * operator== methods for the resulting class.
 *
 * You may later access the merged types via
 * kislager::lenses::to_base<BaseData> lens.
 *
 * Restrictions:
 *
 * 1) The merged structs must have **different** types
 * 2) The arguments passed to the constructor are passed
 *    to constructor of the first base class of in the list.
 *
 *
 * Prefixed data structures
 *
 * If **all** the passed data types support 'prefixed'
 * read and write (that is, define
 * 'static constexpr bool T::supports_prefix = true'),
 * then the tuple switches into a "prefixed" mode:
 *
 * 1) KisOptionTuple's first constructor argument starts
 *    accepting a prefix string
 *
 * 2) This prefix string is passed to all data objects'
 *    constructors
 *
 * 3) The rest of the constructor's arguments are passed
 *    to the first data object only
 *
 * Restrictions:
 *
 * 1) Either **all** or **none** data types must support
 *    prefixed mode (and define 'supports_prefix). Otherwise
 *    KisOptionTuple will fail to compile.
 */

template <typename... Data>
using KisOptionTuple =
    detail::KisOptionTupleImpl<detail::all_support_prefix<Data...>::value,
                               detail::none_support_prefix<Data...>::value,
                               Data...>;

#endif // KISOPTIONTUPLE_H
