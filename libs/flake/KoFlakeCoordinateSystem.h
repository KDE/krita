/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KOFLAKECOORDINATE_SYSTEM_H
#define KOFLAKECOORDINATE_SYSTEM_H

#include <QString>

namespace KoFlake {

enum CoordinateSystem {
    UserSpaceOnUse,
    ObjectBoundingBox
};

inline CoordinateSystem coordinatesFromString(const QString &value, CoordinateSystem defaultValue)
{
    CoordinateSystem result = defaultValue;

    if (value == "userSpaceOnUse") {
        result = UserSpaceOnUse;
    } else if (value == "objectBoundingBox") {
        result = ObjectBoundingBox;
    }

    return result;
}

inline QString coordinateToString(CoordinateSystem value)
{
    return
        value == ObjectBoundingBox?
        "objectBoundingBox" :
        "userSpaceOnUse";
}
}

#endif // KOFLAKECOORDINATE_SYSTEM_H

