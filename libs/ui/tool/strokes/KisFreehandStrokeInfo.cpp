/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
