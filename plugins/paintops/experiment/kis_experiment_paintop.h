/*
 *  Copyright (c) 2010-2011 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef KIS_EXPERIMENT_PAINTOP_H_
#define KIS_EXPERIMENT_PAINTOP_H_

#include <klocalizedstring.h>
#include <brushengine/kis_paintop.h>
#include <kis_types.h>

#include "kis_experiment_paintop_settings.h"
#include "kis_experimentop_option.h"

#include <kis_painter.h>

class QPointF;
class KisPainter;

class KisExperimentPaintOp : public KisPaintOp
{

public:

    KisExperimentPaintOp(const KisPaintOpSettingsSP settings, KisPainter *painter, KisNodeSP node, KisImageSP image);
    ~KisExperimentPaintOp() override;

    void paintLine(const KisPaintInformation& pi1, const KisPaintInformation& pi2, KisDistanceInformation *currentDistance) override;

protected:
    KisSpacingInformation paintAt(const KisPaintInformation& info) override;

    KisSpacingInformation updateSpacingImpl(const KisPaintInformation &info) const override;

private:
    void paintRegion(const QRegion &changedRegion);
    QPointF speedCorrectedPosition(const KisPaintInformation& pi1,
                                   const KisPaintInformation& pi2);


    static qreal simplifyThreshold(const QRectF &bounds);
    static QPointF getAngle(const QPointF& p1, const QPointF& p2, qreal distance);
    static QPainterPath applyDisplace(const QPainterPath& path, int speed);


    bool m_displaceEnabled {false};
    int m_displaceCoeff {0};
    QPainterPath m_lastPaintedPath;

    bool m_windingFill {false};
    bool m_hardEdge {false};

    bool m_speedEnabled {false};
    int m_speedMultiplier {1};
    qreal m_savedSpeedCoeff {1.0};
    QPointF m_savedSpeedPoint;

    bool m_smoothingEnabled {false};
    int m_smoothingThreshold {1};
    QPointF m_savedSmoothingPoint;
    int m_savedSmoothingDistance {1};

    int m_savedUpdateDistance {1};
    QVector<QPointF> m_savedPoints;
    int m_lastPaintTime {0};

    bool m_firstRun {true};
    QPointF m_center;

    QPainterPath m_path;
    ExperimentOption m_experimentOption;

    bool m_useMirroring {false};
    KisPainter *m_originalPainter {0};
    KisPaintDeviceSP m_originalDevice;

    KisPainter::FillStyle m_fillStyle {KisPainter::FillStyleNone};
};

#endif // KIS_EXPERIMENT_PAINTOP_H_
