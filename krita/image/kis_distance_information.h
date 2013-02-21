/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_DISTANCE_INFORMATION_H_
#define _KIS_DISTANCE_INFORMATION_H_

/**
 * This function is used as return of paintLine to contains information that need
 * to be passed for the next call.
 */
struct KisDistanceInformation {

    KisDistanceInformation()
        : distance(0)
        , spacing(0)
    {}

    KisDistanceInformation(double _distance, double _spacing)
        : distance(_distance)
        , spacing(_spacing)
    {}

    void clear()
    {
        distance = 0;
        spacing = 0;
    }

    double distance;
    double spacing;
};

#endif
