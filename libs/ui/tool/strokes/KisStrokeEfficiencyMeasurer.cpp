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

#include "KisStrokeEfficiencyMeasurer.h"

#include <QPointF>
#include <QVector>
#include <QElapsedTimer>

#include <boost/optional.hpp>

#include "kis_global.h"

struct KisStrokeEfficiencyMeasurer::Private
{
    boost::optional<QPointF> lastSamplePos;
    qreal distance = 0;

    QElapsedTimer strokeTimeSource;
    bool isEnabled = true;

    int renderingStartTime = 0;
    int renderingTime = 0;

    int cursorMoveStartTime = 0;
    int cursorMoveTime = 0;

    int framesCount = 0;

};

KisStrokeEfficiencyMeasurer::KisStrokeEfficiencyMeasurer()
    : m_d(new Private())
{
    m_d->strokeTimeSource.start();
}

KisStrokeEfficiencyMeasurer::~KisStrokeEfficiencyMeasurer()
{
}

void KisStrokeEfficiencyMeasurer::setEnabled(bool value)
{
    m_d->isEnabled = value;
}

bool KisStrokeEfficiencyMeasurer::isEnabled() const
{
    return m_d->isEnabled;
}

void KisStrokeEfficiencyMeasurer::addSample(const QPointF &pt)
{
    if (!m_d->isEnabled) return;

    if (!m_d->lastSamplePos) {
        m_d->lastSamplePos = pt;
    } else {
        m_d->distance += kisDistance(pt, *m_d->lastSamplePos);
        *m_d->lastSamplePos = pt;
    }
}

void KisStrokeEfficiencyMeasurer::addSamples(const QVector<QPointF> &points)
{
    if (!m_d->isEnabled) return;

    Q_FOREACH (const QPointF &pt, points) {
        addSample(pt);
    }
}

void KisStrokeEfficiencyMeasurer::notifyRenderingStarted()
{
    m_d->renderingStartTime = m_d->strokeTimeSource.elapsed();
}

void KisStrokeEfficiencyMeasurer::notifyRenderingFinished()
{
    m_d->renderingTime = m_d->strokeTimeSource.elapsed() - m_d->renderingStartTime;
}

void KisStrokeEfficiencyMeasurer::notifyCursorMoveStarted()
{
    m_d->cursorMoveStartTime = m_d->strokeTimeSource.elapsed();
}

void KisStrokeEfficiencyMeasurer::notifyCursorMoveFinished()
{
    m_d->cursorMoveTime = m_d->strokeTimeSource.elapsed() - m_d->cursorMoveStartTime;
}

void KisStrokeEfficiencyMeasurer::notifyFrameRenderingStarted()
{
    m_d->framesCount++;
}

qreal KisStrokeEfficiencyMeasurer::averageCursorSpeed() const
{
    return m_d->cursorMoveTime ? m_d->distance / m_d->cursorMoveTime : 0.0;
}

qreal KisStrokeEfficiencyMeasurer::averageRenderingSpeed() const
{
    return m_d->renderingTime ? m_d->distance / m_d->renderingTime : 0.0;
}

qreal KisStrokeEfficiencyMeasurer::averageFps() const
{
    return m_d->renderingTime ? m_d->framesCount * 1000.0 / m_d->renderingTime : 0.0;
}


