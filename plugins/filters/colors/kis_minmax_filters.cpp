/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_minmax_filters.h"
#include <KoUpdater.h>
#include <KoChannelInfo.h>
#include <filter/kis_filter_category_ids.h>
#include <filter/kis_filter_configuration.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>
#include <KisSequentialIteratorProgress.h>


typedef void (*funcMaxMin)(const quint8* , quint8* , uint);

template<typename _TYPE>
void maximize(const quint8* s, quint8* d, uint nbpixels)
{
    const _TYPE* sT = reinterpret_cast<const _TYPE*>(s);
    _TYPE* dT = reinterpret_cast<_TYPE*>(d);
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
    const _TYPE* sT = reinterpret_cast<const _TYPE*>(s);
    _TYPE* dT = reinterpret_cast<_TYPE*>(d);
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

KisFilterMax::KisFilterMax() : KisFilter(id(), FiltersCategoryColorId, i18n("M&aximize Channel"))
{
    setSupportsPainting(true);
    setSupportsLevelOfDetail(true);
    setColorSpaceIndependence(FULLY_INDEPENDENT);
    setShowConfigurationWidget(false);
}

void KisFilterMax::processImpl(KisPaintDeviceSP device,
                               const QRect& rect,
                               const KisFilterConfigurationSP config,
                               KoUpdater* progressUpdater
                               ) const
{
    Q_UNUSED(config);
    Q_ASSERT(device != 0);

    const KoColorSpace * cs = device->colorSpace();
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

    KisSequentialIteratorProgress it(device, rect, progressUpdater);
    while (it.nextPixel()) {
        F(it.oldRawData(), it.rawData(), nC);
    }
}

KisFilterMin::KisFilterMin() : KisFilter(id(), FiltersCategoryColorId, i18n("M&inimize Channel"))
{
    setSupportsPainting(true);
    setColorSpaceIndependence(FULLY_INDEPENDENT);
    setShowConfigurationWidget(false);
}

void KisFilterMin::processImpl(KisPaintDeviceSP device,
                               const QRect& rect,
                               const KisFilterConfigurationSP config,
                               KoUpdater* progressUpdater
                               ) const
{
    Q_UNUSED(config);
    Q_ASSERT(device != 0);

    const KoColorSpace * cs = device->colorSpace();
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

    KisSequentialIteratorProgress it(device, rect, progressUpdater);
    while (it.nextPixel()) {
        F(it.oldRawData(), it.rawData(), nC);
    }
}

