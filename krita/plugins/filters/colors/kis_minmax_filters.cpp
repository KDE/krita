/*
 * This file is part of Krita
 *
 * Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#include "kis_minmax_filters.h"
#include <KoProgressUpdater.h>
#include <KoUpdater.h>

#include <kis_iterators_pixel.h>
#include <filter/kis_filter_configuration.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>

typedef void (*funcMaxMin)(const quint8* , quint8* , uint);

template<typename _TYPE>
void maximize(const quint8* s, quint8* d, uint nbpixels)
{
    const _TYPE* sT = (_TYPE*)(s);
    _TYPE* dT = (_TYPE*)(d);
    _TYPE vmax = *sT;
    for (uint i = 1; i < nbpixels; i ++) {
        if (sT[i] > vmax) {
            vmax = sT[i];
        }
    }
    for (uint i = 0; i < nbpixels; i ++) {
        if (dT[i] != vmax) {
            dT[i] = 0;
        }
    }
}

template<typename _TYPE>
void minimize(const quint8* s, quint8* d, uint nbpixels)
{
    const _TYPE* sT = (_TYPE*)(s);
    _TYPE* dT = (_TYPE*)(d);
    _TYPE vmin = *sT;
    for (uint i = 1; i < nbpixels; i ++) {
        if (sT[i] < vmin) {
            vmin = sT[i];
        }
    }
    for (uint i = 0; i < nbpixels; i ++) {
        if (dT[i] != vmin) {
            dT[i] = 0;
        }
    }
}

KisFilterMax::KisFilterMax() : KisFilter(id(), categoryColors(), i18n("M&aximize Channel"))
{
    setSupportsPainting(true);
    setSupportsPreview(true);
    setSupportsIncrementalPainting(false);
    setColorSpaceIndependence(FULLY_INDEPENDENT);
}

void KisFilterMax::process(KisConstProcessingInformation srcInfo,
                           KisProcessingInformation dstInfo,
                           const QSize& size,
                           const KisFilterConfiguration* config,
                           KoUpdater* progressUpdater
                          ) const
{
    Q_UNUSED(config);
    const KisPaintDeviceSP src = srcInfo.paintDevice();
    KisPaintDeviceSP dst = dstInfo.paintDevice();
    QPoint dstTopLeft = dstInfo.topLeft();
    QPoint srcTopLeft = srcInfo.topLeft();
    Q_ASSERT(src != 0);
    Q_ASSERT(dst != 0);

    KisRectIteratorPixel dstIt = dst->createRectIterator(dstTopLeft.x(), dstTopLeft.y(), size.width(), size.height(), dstInfo.selection());
    KisRectConstIteratorPixel srcIt = src->createRectConstIterator(srcTopLeft.x(), srcTopLeft.y(), size.width(), size.height(), srcInfo.selection());

    int pixelsProcessed = 0;
    int totalCost = size.width() * size.height() / 100;

    const KoColorSpace * cs = src->colorSpace();
    qint32 nC = cs->colorChannelCount();

    funcMaxMin F;
    KoChannelInfo::enumChannelValueType cT = cs->channels()[0]->channelValueType();
    if (cT == KoChannelInfo::UINT8 || cT == KoChannelInfo::INT8) {
        F = & maximize<quint8>;
    } else if (cT == KoChannelInfo::UINT16 || cT == KoChannelInfo::INT16) {
        F = & maximize<quint16>;
    } else if (cT == KoChannelInfo::FLOAT32) {
        F = & maximize<float>;
    } else {
        return;
    }

    while (! srcIt.isDone()) {
        if (srcIt.isSelected()) {
            F(srcIt.oldRawData(), dstIt.rawData(), nC);
        }
        if (progressUpdater) progressUpdater->setProgress((++pixelsProcessed) / totalCost);
        ++srcIt;
        ++dstIt;
    }
}

KisFilterMin::KisFilterMin() : KisFilter(id(), categoryColors(), i18n("M&inimize Channel"))
{
    setSupportsPainting(true);
    setSupportsPreview(true);
    setSupportsIncrementalPainting(false);
    setColorSpaceIndependence(FULLY_INDEPENDENT);
}

void KisFilterMin::process(KisConstProcessingInformation srcInfo,
                           KisProcessingInformation dstInfo,
                           const QSize& size,
                           const KisFilterConfiguration* config,
                           KoUpdater* progressUpdater
                          ) const
{
    Q_UNUSED(config);
    const KisPaintDeviceSP src = srcInfo.paintDevice();
    KisPaintDeviceSP dst = dstInfo.paintDevice();
    QPoint dstTopLeft = dstInfo.topLeft();
    QPoint srcTopLeft = srcInfo.topLeft();
    Q_ASSERT(src != 0);
    Q_ASSERT(dst != 0);

    KisRectIteratorPixel dstIt = dst->createRectIterator(dstTopLeft.x(), dstTopLeft.y(), size.width(), size.height(), dstInfo.selection());
    KisRectConstIteratorPixel srcIt = src->createRectConstIterator(srcTopLeft.x(), srcTopLeft.y(), size.width(), size.height(), srcInfo.selection());

    int pixelsProcessed = 0;
    int totalCost = size.width() * size.height() / 100;
    if (totalCost == 0) totalCost = 1;

    const KoColorSpace * cs = src->colorSpace();
    qint32 nC = cs->colorChannelCount();

    funcMaxMin F;
    KoChannelInfo::enumChannelValueType cT = cs->channels()[0]->channelValueType();
    if (cT == KoChannelInfo::UINT8 || cT == KoChannelInfo::INT8) {
        F = & minimize<quint8>;
    } else if (cT == KoChannelInfo::UINT16 || cT == KoChannelInfo::INT16) {
        F = & minimize<quint16>;
    } else if (cT == KoChannelInfo::FLOAT32) {
        F = & minimize<float>;
    } else {
        return;
    }

    while (! srcIt.isDone()) {
        if (srcIt.isSelected()) {
            F(srcIt.oldRawData(), dstIt.rawData(), nC);
        }
        if (progressUpdater) progressUpdater->setProgress((++pixelsProcessed) / totalCost);
        ++srcIt;
        ++dstIt;
    }
}

