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
#include "kis_image.h"

#include "krita_utils.h"

#include <boost/random/mersenne_twister.hpp>
#include "kis_random_accessor_ng.h"
#include "kis_iterator_ng.h"


struct Q_DECL_HIDDEN KisLayerStyleFilterEnvironment::Private
{
    KisLayer *sourceLayer;
    KisPixelSelectionSP cachedRandomSelection;

    static KisPixelSelectionSP generateRandomSelection(const QRect &rc);
};


KisPixelSelectionSP
KisLayerStyleFilterEnvironment::Private::
generateRandomSelection(const QRect &rc)
{
    KisPixelSelectionSP selection = new KisPixelSelection();
    KisSequentialIterator dstIt(selection, rc);

    boost::mt11213b uniformSource;

    if (uniformSource.max() >= 0x00FFFFFF) {
        while (dstIt.nextPixel()) {
            int randValue = uniformSource();
            *dstIt.rawData() = (quint8) randValue;

            if (!dstIt.nextPixel()) break;
            randValue >>= 8;
            *dstIt.rawData() = (quint8) randValue;

            if (!dstIt.nextPixel()) break;
            randValue >>= 8;
            *dstIt.rawData() = (quint8) randValue;
        }

    } else {
        while (dstIt.nextPixel()) {
            *dstIt.rawData() = (quint8) uniformSource();
        }
    }

    return selection;
}

KisLayerStyleFilterEnvironment::KisLayerStyleFilterEnvironment(KisLayer *sourceLayer)
    : m_d(new Private)
{
    Q_ASSERT(sourceLayer);
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

int KisLayerStyleFilterEnvironment::currentLevelOfDetail() const
{
    return m_d->sourceLayer ?
        m_d->sourceLayer->original()->defaultBounds()->currentLevelOfDetail() : 0;
}

void KisLayerStyleFilterEnvironment::setupFinalPainter(KisPainter *gc,
                                                       quint8 opacity,
                                                       const QBitArray &channelFlags) const
{
    Q_ASSERT(m_d->sourceLayer);
    gc->setOpacity(KritaUtils::mergeOpacity(opacity, m_d->sourceLayer->opacity()));
    gc->setChannelFlags(KritaUtils::mergeChannelFlags(channelFlags, m_d->sourceLayer->channelFlags()));

}

KisPixelSelectionSP KisLayerStyleFilterEnvironment::cachedRandomSelection(const QRect &requestedRect) const
{
    KisPixelSelectionSP selection = m_d->cachedRandomSelection;

    QRect existingRect;

    if (selection) {
        existingRect = selection->selectedExactRect();
    }

    if (!existingRect.contains(requestedRect)) {
        m_d->cachedRandomSelection =
            Private::generateRandomSelection(requestedRect | existingRect);
    }

    return m_d->cachedRandomSelection;
}
