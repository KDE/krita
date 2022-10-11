/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISOPTIONTUPLE_H
#define KISOPTIONTUPLE_H

#include <KisMpl.h>
#include <boost/operators.hpp>

/**
 * KisOptionTuple is a class that merges multiple option
 * data structs into one by inheriting from all of them.
 *
 * KisOptionTuple automatically generates read, write and
 * operator== methods for the resulting class.
 *
 * You may later access the merged types via
 * kiszug::lenses::to_base<BaseData> lens.
 *
 * Restrictions:
 *
 * 1) The merged structs must have **different** types
 * 2) The arguments passed to the constructor are passed
 *    to constructor of the first base class of in the list.
 */

template <typename... Data>
struct KisOptionTuple : public Data..., boost::equality_comparable<KisOptionTuple<Data...>>
{
    template<typename... T>
    KisOptionTuple(T... args)
        : kismpl::first_type_t<Data...>(std::forward<decltype(args)>(args)...)
    {
    }

    inline friend bool operator==(const KisOptionTuple<Data...> &lhs, const KisOptionTuple<Data...> &rhs) {
        return ((static_cast<const Data&>(lhs) == static_cast<const Data&>(rhs)) && ...);
    }

    bool read(const KisPropertiesConfiguration *setting) {
        return (static_cast<Data&>(*this).read(setting) && ...);
    }

    void write(KisPropertiesConfiguration *setting) const {
        (static_cast<Data&>(*this).write(setting) , ...);
    }
};


#endif // KISOPTIONTUPLE_H
