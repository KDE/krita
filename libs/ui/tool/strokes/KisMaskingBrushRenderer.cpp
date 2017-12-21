/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KisMaskingBrushRenderer.h"

#include "kis_painter.h"
#include "kis_paint_device.h"
#include "kis_sequential_iterator.h"

#include <KoGrayColorSpaceTraits.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorModelStandardIds.h>


KisMaskingBrushRenderer::KisMaskingBrushRenderer(KisPaintDeviceSP dstDevice)
    : m_dstDevice(dstDevice)
{
    m_strokeDevice = new KisPaintDevice(dstDevice->colorSpace());
    m_maskDevice = new KisPaintDevice(
        KoColorSpaceRegistry::instance()->colorSpace(GrayAColorModelID.id(),
                                                     Integer8BitsColorDepthID.id()));

    m_strokeDevice->setDefaultBounds(dstDevice->defaultBounds());
    m_maskDevice->setDefaultBounds(dstDevice->defaultBounds());
}

KisPaintDeviceSP KisMaskingBrushRenderer::strokeDevice() const
{
    return m_strokeDevice;
}

KisPaintDeviceSP KisMaskingBrushRenderer::maskDevice() const
{
    return m_maskDevice;
}

void KisMaskingBrushRenderer::updateProjection(const QRect &rc)
{
    if (rc.isEmpty()) return;

    KisPainter::copyAreaOptimized(rc.topLeft(), m_strokeDevice, m_dstDevice, rc);

    KisSequentialIterator dstIt(m_dstDevice, rc);
    KisSequentialConstIterator maskIt(m_maskDevice, rc);

    using MaskPixel = KoGrayU8Traits::Pixel;
    const KoColorSpace *dstCs = m_dstDevice->colorSpace();

    do {
        const MaskPixel *maskPtr = reinterpret_cast<const MaskPixel*>(maskIt.rawDataConst());
        quint8 *dstPtr = reinterpret_cast<quint8*>(dstIt.rawData());

        const quint8 maskValue = KoColorSpaceMaths<quint8>::multiply(maskPtr->gray, maskPtr->alpha);

        // TODO: bunch-processing!
        dstCs->applyAlphaU8Mask(dstPtr, &maskValue, 1);


    } while (dstIt.nextPixel() && maskIt.nextPixel());


}

