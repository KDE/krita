/*
 *  Copyright (c) 2016 Alvin Wong <alvinhochun-at-gmail-com>
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

#ifndef KIS_STABILIZER_DELAYED_PAINT_HELPER_H
#define KIS_STABILIZER_DELAYED_PAINT_HELPER_H

#include <QElapsedTimer>
#include <QQueue>
#include <QTimer>
#include <QVector>

#include <functional>

#include "kis_paint_information.h"
#include "kritaui_export.h"

class KRITAUI_EXPORT KisStabilizerDelayedPaintHelper : public QObject
{
    Q_OBJECT

    struct TimedPaintInfo
    {
        int elapsedTime;
        KisPaintInformation paintInfo;
        TimedPaintInfo(int elapsedTime, KisPaintInformation paintInfo);
    };

    QTimer m_paintTimer;
    QQueue<TimedPaintInfo> m_paintQueue;
    int m_lastPendingTime;
    int m_lastPaintTime;
    QElapsedTimer m_elapsedTimer;

    // Callbacks
    std::function<void(const KisPaintInformation &, const KisPaintInformation &)> m_paintLine;
    std::function<void()> m_requestUpdateOutline;

public:
    KisStabilizerDelayedPaintHelper();
    ~KisStabilizerDelayedPaintHelper() override {}

    bool running() const {
        return m_paintTimer.isActive();
    }

    bool hasLastPaintInformation() const {
        return !m_paintQueue.isEmpty();
    }

    KisPaintInformation lastPaintInformation() const {
        // Please call hasLastPaintInformation before this
        return m_paintQueue.head().paintInfo;
    }

    void setPaintLineCallback(std::function<void(const KisPaintInformation &, const KisPaintInformation &)> paintLine) {
        m_paintLine = paintLine;
    }

    void setUpdateOutlineCallback(std::function<void()> requestUpdateOutline) {
        m_requestUpdateOutline = requestUpdateOutline;
    }

    void start(const KisPaintInformation &firstPaintInfo);
    void update(const QVector<KisPaintInformation> &newPaintInfos);
    void paintSome();
    void end();
    void cancel();

private Q_SLOTS:
    void stabilizerDelayedPaintTimer();
};

#endif // KIS_STABILIZER_DELAYED_PAINT_HELPER_H
