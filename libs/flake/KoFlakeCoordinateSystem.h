/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

