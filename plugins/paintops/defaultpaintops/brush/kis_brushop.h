/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#ifndef KIS_BRUSHOP_H_
#define KIS_BRUSHOP_H_

#include "kis_brush_based_paintop.h"
#include <kis_airbrush_option_widget.h>
#include <kis_pressure_flow_opacity_option.h>
#include <kis_pressure_size_option.h>
#include <kis_pressure_ratio_option.h>
#include <kis_pressure_flow_option.h>
#include <kis_pressure_rotation_option.h>
#include <kis_pressure_scatter_option.h>
#include <kis_pressure_softness_option.h>
#include <kis_pressure_sharpness_option.h>
#include <kis_pressure_spacing_option.h>
#include <kis_pressure_rate_option.h>
#include <kis_brush_based_paintop_settings.h>

#include <KisRollingMeanAccumulatorWrapper.h>

#include <QElapsedTimer>

class KisPainter;
class KisColorSource;
class KisDabRenderingExecutor;
struct KisRenderedDab;
class KisRunnableStrokeJobData;

class KisBrushOp : public KisBrushBasedPaintOp
{

public:

    KisBrushOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisNodeSP node, KisImageSP image);
    ~KisBrushOp() override;

    void paintLine(const KisPaintInformation &pi1, const KisPaintInformation &pi2, KisDistanceInformation *currentDistance) override;

    std::pair<int, bool> doAsyncronousUpdate(QVector<KisRunnableStrokeJobData *> &jobs) override;

protected:
    KisSpacingInformation paintAt(const KisPaintInformation& info) override;

    KisSpacingInformation updateSpacingImpl(const KisPaintInformation &info) const override;

    KisTimingInformation updateTimingImpl(const KisPaintInformation &info) const override;

    struct UpdateSharedState;
    typedef QSharedPointer<UpdateSharedState> UpdateSharedStateSP;

    void addMirroringJobs(Qt::Orientation direction,
                          QVector<QRect> &rects,
                          UpdateSharedStateSP state,
                          QVector<KisRunnableStrokeJobData*> &jobs);

    UpdateSharedStateSP m_updateSharedState;


private:
    KisAirbrushOptionProperties m_airbrushOption;
    KisPressureSizeOption m_sizeOption;
    KisPressureRatioOption m_ratioOption;
    KisPressureSpacingOption m_spacingOption;
    KisPressureRateOption m_rateOption;
    KisPressureFlowOption m_flowOption;
    KisFlowOpacityOption m_opacityOption;
    KisPressureSoftnessOption m_softnessOption;
    KisPressureSharpnessOption m_sharpnessOption;
    KisPressureRotationOption m_rotationOption;
    KisPressureScatterOption m_scatterOption;

    KisPaintDeviceSP m_lineCacheDevice;

    QScopedPointer<KisDabRenderingExecutor> m_dabExecutor;
    qreal m_currentUpdatePeriod = 20.0;
    KisRollingMeanAccumulatorWrapper m_avgSpacing;
    KisRollingMeanAccumulatorWrapper m_avgNumDabs;
    KisRollingMeanAccumulatorWrapper m_avgUpdateTimePerDab;

    const int m_idealNumRects;

    const int m_minUpdatePeriod;
    const int m_maxUpdatePeriod;
};

#endif // KIS_BRUSHOP_H_
