/*
 *  Copyright (c) 2018 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KoCanvasUpdatesCollector.h"

#include <KoCanvasBase.h>

KoCanvasUpdatesCollector::KoCanvasUpdatesCollector(KoCanvasBase *canvas)
    : m_canvas(canvas)
{
}

KoCanvasUpdatesCollector::~KoCanvasUpdatesCollector()
{
    forceUpdate();
}

KoCanvasUpdatesCollector::KoCanvasUpdatesCollector(KoCanvasUpdatesCollector &&rhs)
    : m_canvas(rhs.m_canvas)
{
    rhs.m_canvas = 0;
}

KoCanvasUpdatesCollector& KoCanvasUpdatesCollector::operator=(KoCanvasUpdatesCollector &&rhs)
{
    forceUpdate();
    m_canvas = rhs.m_canvas;
    rhs.m_canvas = 0;

    return *this;
}

void KoCanvasUpdatesCollector::addUpdate(const QRectF &rect)
{
    if (!rect.isEmpty()) {
        m_rects << rect;
    }
}

void KoCanvasUpdatesCollector::addUpdate(const QVector<QRectF> &rects)
{
    Q_FOREACH (const QRectF &rc, rects) {
        addUpdate(rc);
    }
}

void KoCanvasUpdatesCollector::forceUpdate()
{
    if (m_canvas && !m_rects.isEmpty()) {
        // do some optimizations :_)

        m_canvas->updateCanvas(m_rects);
        m_rects.clear();
    }
}

void KoCanvasUpdatesCollector::reset()
{
    m_rects.clear();
}

