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

#ifndef KISSTROKESPEEDMONITOR_H
#define KISSTROKESPEEDMONITOR_H

#include <QObject>

#include "kis_types.h"
#include "kritaui_export.h"

class KRITAUI_EXPORT KisStrokeSpeedMonitor : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString lastPresetName READ lastPresetName NOTIFY sigStatsUpdated)
    Q_PROPERTY(qreal lastPresetSize READ lastPresetSize NOTIFY sigStatsUpdated)

    Q_PROPERTY(qreal lastCursorSpeed READ lastCursorSpeed NOTIFY sigStatsUpdated)
    Q_PROPERTY(qreal lastRenderingSpeed READ lastRenderingSpeed NOTIFY sigStatsUpdated)
    Q_PROPERTY(qreal lastFps READ lastFps NOTIFY sigStatsUpdated)

    Q_PROPERTY(bool lastStrokeSaturated READ lastCursorSpeed NOTIFY sigStatsUpdated)

    Q_PROPERTY(qreal avgCursorSpeed READ avgCursorSpeed NOTIFY sigStatsUpdated)
    Q_PROPERTY(qreal avgRenderingSpeed READ avgRenderingSpeed NOTIFY sigStatsUpdated)
    Q_PROPERTY(qreal avgFps READ avgFps NOTIFY sigStatsUpdated)

public:
    KisStrokeSpeedMonitor();
    ~KisStrokeSpeedMonitor();

    static KisStrokeSpeedMonitor* instance();

    bool haveStrokeSpeedMeasurement() const;

    void notifyStrokeFinished(qreal cursorSpeed, qreal renderingSpeed, qreal fps, KisPaintOpPresetSP preset);


    QString lastPresetName() const;
    qreal lastPresetSize() const;

    qreal lastCursorSpeed() const;
    qreal lastRenderingSpeed() const;
    qreal lastFps() const;
    bool lastStrokeSaturated() const;

    qreal avgCursorSpeed() const;
    qreal avgRenderingSpeed() const;
    qreal avgFps() const;


Q_SIGNALS:
    void sigStatsUpdated();

public Q_SLOTS:
    void setHaveStrokeSpeedMeasurement(bool value);

private Q_SLOTS:
    void resetAccumulatedValues();
    void slotConfigChanged();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISSTROKESPEEDMONITOR_H
