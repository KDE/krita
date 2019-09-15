/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KisFreehandStrokeInfo.h"

#include <kis_painter.h>
#include <kis_distance_information.h>


KisFreehandStrokeInfo::KisFreehandStrokeInfo()
    : painter(new KisPainter())
    , dragDistance(new KisDistanceInformation())
{
}

KisFreehandStrokeInfo::KisFreehandStrokeInfo(const KisDistanceInformation &startDist)
    : painter(new KisPainter())
    , dragDistance(new KisDistanceInformation(startDist))
{
}

KisFreehandStrokeInfo::KisFreehandStrokeInfo(KisFreehandStrokeInfo *rhs, int levelOfDetail)
    : painter(new KisPainter())
    , dragDistance(new KisDistanceInformation(*rhs->dragDistance, levelOfDetail))
    , m_parentStrokeInfo(rhs)
{
    rhs->m_childStrokeInfo = this;
}

KisFreehandStrokeInfo::~KisFreehandStrokeInfo()
{
    if (m_parentStrokeInfo) {
        m_parentStrokeInfo->m_childStrokeInfo = 0;
    }

    delete(painter);
    delete(dragDistance);
}

KisDistanceInformation* KisFreehandStrokeInfo::buddyDragDistance()
{
    return m_childStrokeInfo ? m_childStrokeInfo->dragDistance : 0;
}
