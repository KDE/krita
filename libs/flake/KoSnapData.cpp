/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoSnapData.h"

#include <QPointF>

KoSnapData::KoSnapData()
{
}

KoSnapData::~KoSnapData()
{
}

QList<QPointF> KoSnapData::snapPoints() const
{
    return m_points;
}

void KoSnapData::setSnapPoints(const QList<QPointF> &snapPoints)
{
    m_points = snapPoints;
}

QList<KoPathSegment> KoSnapData::snapSegments() const
{
    return m_segments;
}

void KoSnapData::setSnapSegments(const QList<KoPathSegment> &snapSegments)
{
    m_segments = snapSegments;
}
