/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_LAYER_STYLE_FILTER_ENVIRONMENT_H */
