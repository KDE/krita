/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSTROKELAYERSTYLEFILTERPROJECTIONPLANE_H
#define KISSTROKELAYERSTYLEFILTERPROJECTIONPLANE_H

#include "kis_layer_style_filter_projection_plane.h"
#include "krita_utils.h"


class KisStrokeLayerStyleFilterProjectionPlane : public KisLayerStyleFilterProjectionPlane
{
public:
    KisStrokeLayerStyleFilterProjectionPlane(KisLayer *sourceLayer);
    KisStrokeLayerStyleFilterProjectionPlane(const KisStrokeLayerStyleFilterProjectionPlane &rhs, KisLayer *sourceLayer, KisPSDLayerStyleSP clonedStyle);
    ~KisStrokeLayerStyleFilterProjectionPlane() override;

    KritaUtils::ThresholdMode sourcePlaneOpacityThresholdRequirement() const;
};

using KisStrokeLayerStyleFilterProjectionPlaneSP = QSharedPointer<KisStrokeLayerStyleFilterProjectionPlane>;

#endif // KISSTROKELAYERSTYLEFILTERPROJECTIONPLANE_H
