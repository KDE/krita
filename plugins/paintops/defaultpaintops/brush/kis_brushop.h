/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2004 Clarence Dang <dang@kde.org>
 *  SPDX-FileCopyrightText: 2004 Adrian Page <adrian@pagenet.plus.com>
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_BRUSHOP_H_
#define KIS_BRUSHOP_H_

#include "kis_brush_based_paintop.h"
#include <kis_brush_based_paintop_settings.h>
#include <KisStandardOptions.h>
#include <KisAirbrushOptionData.h>
#include <KisSpacingOption.h>
#include <KisScatterOption.h>
#include <KisSharpnessOption.h>
#include <KisRotationOption.h>
#include <KisFlowOpacityOption.h>

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

    std::pair<int, bool> doAsynchronousUpdate(QVector<KisRunnableStrokeJobData *> &jobs) override;

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
    KisAirbrushOptionData m_airbrushData;

    KisSizeOption m_sizeOption;
    KisRatioOption m_ratioOption;
    KisRateOption m_rateOption;
    KisSoftnessOption m_softnessOption;
    KisLightnessStrengthOption m_lightnessStrengthOption;
    KisSpacingOption m_spacingOption;
    KisScatterOption m_scatterOption;
    KisSharpnessOption m_sharpnessOption;
    KisRotationOption m_rotationOption;
    KisFlowOpacityOption2 m_opacityOption;


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
