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

#ifndef KOCANVASUPDATESCOLLECTOR_H
#define KOCANVASUPDATESCOLLECTOR_H

#include "kritaflake_export.h"
#include <QVector>
#include <QRectF>

class KoCanvasBase;

/**
 * @brief The KoCanvasUpdatesCollector class is supposed to gather multiple
 * updates issues for a single canvas and emit them all at once, either on
 * destruction or on when called explicitly
 */
class KRITAFLAKE_EXPORT KoCanvasUpdatesCollector
{
public:
    /**
     * Create an updates collector that collects the updates for canvas \p canvas
     */
    KoCanvasUpdatesCollector(KoCanvasBase *canvas);

    /**
     * Issue all the gathered updates and destroy the object
     */
    ~KoCanvasUpdatesCollector();

    // not-copyable
    KoCanvasUpdatesCollector(const KoCanvasUpdatesCollector &rhs) = delete;
    KoCanvasUpdatesCollector& operator=(const KoCanvasUpdatesCollector &rhs) = delete;

    // movable
    KoCanvasUpdatesCollector(KoCanvasUpdatesCollector &&rhs);
    KoCanvasUpdatesCollector& operator=(KoCanvasUpdatesCollector &&rhs);

    /**
     * Add \p rect to the list of pending updates
     */
    void addUpdate(const QRectF &rect);

    /**
     * Add \p rects to the list of pending updates
     */
    void addUpdate(const QVector<QRectF> &rects);

    /**
     * Issue all the gathered updates right now (if there are any). Please note that
     * the gathered updates are guarenteed to be issued only once! If you mix update() and
     * forceUpdate(), then the updates will be release in portions.
     */
    void forceUpdate();

    /**
     * Drop all the gathered updates
     */
    void reset();

private:
    KoCanvasBase *m_canvas = 0;
    QVector<QRectF> m_rects;
};

#endif // KOCANVASUPDATESCOLLECTOR_H
