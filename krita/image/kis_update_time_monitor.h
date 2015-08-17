/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_UPDATE_TIME_MONITOR_H
#define __KIS_UPDATE_TIME_MONITOR_H

#include "kritaimage_export.h"
#include "kis_types.h"


#include <QVector>
class QPointF;
class QRect;


class KRITAIMAGE_EXPORT KisUpdateTimeMonitor
{
public:
    static KisUpdateTimeMonitor* instance();
    ~KisUpdateTimeMonitor();

    void startStrokeMeasure();
    void endStrokeMeasure();
    void reportPaintOpPreset(KisPaintOpPresetSP preset);

    void reportMouseMove(const QPointF &pos);
    void printValues();

    void reportJobStarted(void *key);
    void reportJobFinished(void *key, const QVector<QRect> &rects);
    void reportUpdateFinished(const QRect &rect);

private:
    KisUpdateTimeMonitor();

private:
    struct Private;
    Private * const m_d;
};

#endif /* __KIS_UPDATE_TIME_MONITOR_H */
