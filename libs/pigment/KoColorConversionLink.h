/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef _KO_COLOR_CONVERSION_LINK_H_
#define _KO_COLOR_CONVERSION_LINK_H_

class QString;

class KoColorConversionLink {
    public:
        enum Direction {
            OutputLink = 1,
            InputLink = 2,
            BothDirection = 3
        };
    public:
        KoColorConversionLink(QString _srcModelId, QString _srcDepthId, QString _dstModelId, QString _dstDepthId, Direction _direction);
    private:
        struct Private;
        Private* const d;
};

#endif
