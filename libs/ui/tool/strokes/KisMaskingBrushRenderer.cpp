/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisMaskingBrushRenderer.h"

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorModelStandardIds.h>
#include <KoChannelInfo.h>
#include <KoCompositeOpRegistry.h>

#include "kis_painter.h"
#include "kis_paint_device.h"
#include "kis_random_accessor_ng.h"

#include "KisMaskingBrushCompositeOpBase.h"
#include "KisMaskingBrushCompositeOpFactory.h"


KisMaskingBrushRenderer::KisMaskingBrushRenderer(KisPaintDeviceSP dstDevice, const QString &compositeOpId)
    : m_dstDevice(dstDevice)
{
    m_strokeDevice = new KisPaintDevice(dstDevice->colorSpace());
    m_maskDevice = new KisPaintDevice(
                KoColorSpaceRegistry::instance()->colorSpace(GrayAColorModelID.id(),
                                                             Integer8BitsColorDepthID.id()));

    m_strokeDevice->setDefaultBounds(dstDevice->defaultBounds());
    m_maskDevice->setDefaultBounds(dstDevice->defaultBounds());

    const KoColorSpace *dstCs = m_dstDevice->colorSpace();
    const int pixelSize = dstCs->pixelSize();

    KoChannelInfo::enumChannelValueType alphaChannelType = KoChannelInfo::UINT8;
    int alphaChannelOffset = -1;

    QList<KoChannelInfo *> channels = dstCs->channels();
    for (int i = 0; i < pixelSize; i++) {
        if (channels[i]->channelType() == KoChannelInfo::ALPHA) {
            // TODO: check correctness for 16bits!
            alphaChannelOffset = channels[i]->pos()/* * channels[i]->size()*/;
            alphaChannelType = channels[i]->channelValueType();
            break;
        }
    }

    KIS_SAFE_ASSERT_RECOVER (alphaChannelOffset >= 0) {
        alphaChannelOffset = 0;
    }

    m_compositeOp.reset(
        KisMaskingBrushCompositeOpFactory::create(
            compositeOpId, alphaChannelType, pixelSize, alphaChannelOffset));
}

KisMaskingBrushRenderer::~KisMaskingBrushRenderer()
{
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

    KisRandomAccessorSP dstIt = m_dstDevice->createRandomAccessorNG();
    KisRandomConstAccessorSP maskIt = m_maskDevice->createRandomConstAccessorNG();

    qint32 dstY = rc.y();
    qint32 rowsRemaining = rc.height();

    while (rowsRemaining > 0) {
        qint32 dstX = rc.x();

        const qint32 numContiguousDstRows = dstIt->numContiguousRows(dstY);
        const qint32 numContiguousMaskRows = maskIt->numContiguousRows(dstY);

        const qint32 rows = std::min({rowsRemaining, numContiguousDstRows, numContiguousMaskRows});

        qint32 columnsRemaining = rc.width();

        while (columnsRemaining > 0) {

            const qint32 numContiguousDstColumns = dstIt->numContiguousColumns(dstX);
            const qint32 numContiguousMaskColumns = maskIt->numContiguousColumns(dstX);
            const qint32 columns = std::min({columnsRemaining, numContiguousDstColumns, numContiguousMaskColumns});

            const qint32 dstRowStride = dstIt->rowStride(dstX, dstY);
            const qint32 maskRowStride = maskIt->rowStride(dstX, dstY);

            dstIt->moveTo(dstX, dstY);
            maskIt->moveTo(dstX, dstY);

            m_compositeOp->composite(maskIt->rawDataConst(), maskRowStride,
                                     dstIt->rawData(), dstRowStride,
                                     columns, rows);

            dstX += columns;
            columnsRemaining -= columns;
        }

        dstY += rows;
        rowsRemaining -= rows;
    }
}

