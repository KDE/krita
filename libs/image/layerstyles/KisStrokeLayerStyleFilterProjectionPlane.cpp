/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisStrokeLayerStyleFilterProjectionPlane.h"

#include "kis_ls_stroke_filter.h"

KisStrokeLayerStyleFilterProjectionPlane::KisStrokeLayerStyleFilterProjectionPlane(KisLayer *sourceLayer)
    : KisLayerStyleFilterProjectionPlane(sourceLayer)
{
}

KisStrokeLayerStyleFilterProjectionPlane::KisStrokeLayerStyleFilterProjectionPlane(const KisStrokeLayerStyleFilterProjectionPlane &rhs, KisLayer *sourceLayer, KisPSDLayerStyleSP clonedStyle)
    : KisLayerStyleFilterProjectionPlane(rhs, sourceLayer, clonedStyle)
{
}

KisStrokeLayerStyleFilterProjectionPlane::~KisStrokeLayerStyleFilterProjectionPlane()
{
}

KritaUtils::ThresholdMode KisStrokeLayerStyleFilterProjectionPlane::sourcePlaneOpacityThresholdRequirement() const
{
    if (!filter()) return KritaUtils::ThresholdNone;

    const KisLsStrokeFilter *filter = dynamic_cast<const KisLsStrokeFilter*>(this->filter());
    return filter ? filter->sourcePlaneOpacityThresholdRequirement(style()) : KritaUtils::ThresholdNone;
}
