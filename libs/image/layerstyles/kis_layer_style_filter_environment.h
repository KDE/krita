/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_LAYER_STYLE_FILTER_ENVIRONMENT_H
#define __KIS_LAYER_STYLE_FILTER_ENVIRONMENT_H

#include <QScopedPointer>
#include <QRect>

#include <kritaimage_export.h>
#include "kis_types.h"

class KisPainter;
class KisLayer;
class QPainterPath;
class QBitArray;
class KisCachedPaintDevice;
class KisCachedSelection;


class KRITAIMAGE_EXPORT KisLayerStyleFilterEnvironment
{
public:
    KisLayerStyleFilterEnvironment(KisLayer *sourceLayer);
    ~KisLayerStyleFilterEnvironment();

    QRect layerBounds() const;
    QRect defaultBounds() const;
    int currentLevelOfDetail() const;

    void setupFinalPainter(KisPainter *gc,
                           quint8 opacity,
                           const QBitArray &channelFlags) const;

    KisPixelSelectionSP cachedRandomSelection(const QRect &requestedRect) const;

    KisCachedSelection* cachedSelection();
    KisCachedPaintDevice* cachedPaintDevice();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_LAYER_STYLE_FILTER_ENVIRONMENT_H */
