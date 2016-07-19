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

#ifndef __KIS_PAINTING_TWEAKS_H
#define __KIS_PAINTING_TWEAKS_H

#include "kritaglobal_export.h"

class QPainter;
class QRegion;
class QRect;

namespace KisPaintingTweaks {

    /**
     * This is a workaround for QPainter::clipRegion() bug. When zoom
     * is about 2000% and rotation is in a range[-5;5] degrees, the
     * generated region will have about 20k+ regtangles inside. Their
     * processing will be really slow. These functions fworkarounds
     * the issue.
     */
    KRITAGLOBAL_EXPORT QRegion safeClipRegion(const QPainter &painter);

    /**
     * \see safeClipRegion()
     */
    KRITAGLOBAL_EXPORT QRect safeClipBoundingRect(const QPainter &painter);
}

#endif /* __KIS_PAINTING_TWEAKS_H */
