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

#include "kis_layer_style_filter_environment.h"

#include <QBitArray>

#include "kis_layer.h"
#include "kis_ls_utils.h"

#include "kis_selection.h"
#include "kis_pixel_selection.h"
#include "kis_painter.h"

#include "krita_utils.h"


struct KisLayerStyleFilterEnvironment::Private
{
    KisLayer *sourceLayer;
};


KisLayerStyleFilterEnvironment::KisLayerStyleFilterEnvironment(KisLayer *sourceLayer)
    : m_d(new Private)
{
    m_d->sourceLayer = sourceLayer;
}

KisLayerStyleFilterEnvironment::~KisLayerStyleFilterEnvironment()
{
}

QRect KisLayerStyleFilterEnvironment::layerBounds() const
{
    return m_d->sourceLayer ? m_d->sourceLayer->projection()->exactBounds() : QRect();
}

QRect KisLayerStyleFilterEnvironment::defaultBounds() const
{
    return m_d->sourceLayer ?
        m_d->sourceLayer->original()->defaultBounds()->bounds() : QRect();
}

QPainterPath KisLayerStyleFilterEnvironment::layerOutlineCache() const
{
    // TODO: make it really cachable!

    KisPaintDeviceSP srcDevice = m_d->sourceLayer->projection();

    QRect srcRect = srcDevice->exactBounds();
    if (srcRect.isEmpty()) return QPainterPath();

    KisSelectionSP baseSelection =
        KisLsUtils::selectionFromAlphaChannel(srcDevice, srcRect);
    KisPixelSelectionSP selection = baseSelection->pixelSelection();

    // needs no 'invalidate' call
    selection->recalculateOutlineCache();

    return selection->outlineCache();
}

void KisLayerStyleFilterEnvironment::setupFinalPainter(KisPainter *gc,
                                                       quint8 opacity,
                                                       const QBitArray &channelFlags) const
{
    gc->setOpacity(KritaUtils::mergeOpacity(opacity, m_d->sourceLayer->opacity()));
    gc->setChannelFlags(KritaUtils::mergeChannelFlags(channelFlags, m_d->sourceLayer->channelFlags()));
}
