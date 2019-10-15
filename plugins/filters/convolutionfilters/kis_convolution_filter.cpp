/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#include "kis_convolution_filter.h"
#include <klocalizedstring.h>
#include <kis_debug.h>

#include <KoCompositeOp.h>

#include "kis_painter.h"
#include "kis_convolution_painter.h"
#include "kis_convolution_kernel.h"
#include <filter/kis_filter_configuration.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>
#include "kis_lod_transform.h"

KisConvolutionFilter::KisConvolutionFilter(const KoID& id, const KoID & category, const QString & entry)
        : KisFilter(id, category, entry)
{
    setColorSpaceIndependence(FULLY_INDEPENDENT);
    setSupportsLevelOfDetail(true);
}


void KisConvolutionFilter::processImpl(KisPaintDeviceSP device,
                                       const QRect& applyRect,
                                       const KisFilterConfigurationSP config,
                                       KoUpdater* progressUpdater) const
{
    Q_UNUSED(config);

    QPoint srcTopLeft = applyRect.topLeft();
    Q_ASSERT(device != 0);

    KisConvolutionPainter painter(device);

    QBitArray channelFlags;
    if (config) {
        channelFlags = config->channelFlags();
    }
    if (channelFlags.isEmpty() || !config) {
        channelFlags = QBitArray(device->colorSpace()->channelCount(), true);
    }
 
    painter.setChannelFlags(channelFlags);
    painter.setProgress(progressUpdater);
    painter.applyMatrix(m_matrix, device, srcTopLeft, srcTopLeft, applyRect.size(), BORDER_REPEAT);

}

QRect KisConvolutionFilter::neededRect(const QRect &rect, const KisFilterConfigurationSP _config, int lod) const
{
    Q_UNUSED(_config);

    KisLodTransformScalar t(lod);

    const int windowsize = qMax(m_matrix->width(), m_matrix->height());
    const int margin  = qCeil(t.scale(0.5 * windowsize)) + 1;
    return kisGrowRect(rect, margin);
}

QRect KisConvolutionFilter::changedRect(const QRect &rect, const KisFilterConfigurationSP _config, int lod) const
{
    return neededRect(rect, _config, lod);
}

void KisConvolutionFilter::setIgnoreAlpha(bool v)
{
    m_ignoreAlpha = v;
}
