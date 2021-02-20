/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
