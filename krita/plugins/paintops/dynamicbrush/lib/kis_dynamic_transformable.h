/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_DYNAMIC_TRANSFORMABLE_H_
#define _KIS_DYNAMIC_TRANSFORMABLE_H_

/**
 * This is an interface to define the function call which are common between
 * shapes and coloring.
 */
class KisDynamicTransformable {
    public:
        virtual ~KisDynamicTransformable() {}
        /**
         * Call this function to rotate this object
         */
        virtual void rotate(double r) = 0;
        /**
         * Call this function to resize the object.
         * @param xs horizontal scaling
         * @param ys vertical scaling
         */
        virtual void resize(double xs, double ys) = 0;
};

#endif
