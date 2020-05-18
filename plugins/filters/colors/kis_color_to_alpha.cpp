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

#include "kis_color_to_alpha.h"
#include <QCheckBox>
#include <QSpinBox>

#include <KoColorSpaceMaths.h>
#include <KoConfig.h>
#include <KoUpdater.h>

#include "kis_progress_update_helper.h"
#include <kis_paint_device.h>
#include <kis_selection.h>
#include <filter/kis_filter_category_ids.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>

#include "ui_wdgcolortoalphabase.h"
#include "kis_wdg_color_to_alpha.h"
#include <kis_iterator_ng.h>
#include <KisSequentialIteratorProgress.h>

KisFilterColorToAlpha::KisFilterColorToAlpha()
    : KisFilter(id(), FiltersCategoryColorId, i18n("&Color to Alpha..."))
{
    setSupportsPainting(true);
    setSupportsAdjustmentLayers(true);
    setSupportsLevelOfDetail(true);
    setColorSpaceIndependence(FULLY_INDEPENDENT);
}

KisConfigWidget * KisFilterColorToAlpha::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP, bool) const
{
    return new KisWdgColorToAlpha(parent);
}

KisFilterConfigurationSP KisFilterColorToAlpha::defaultConfiguration() const
{
    KisFilterConfigurationSP config = factoryConfiguration();
    config->setProperty("targetcolor", QColor(255, 255, 255));
    config->setProperty("threshold", 100);
    return config;
}

template<typename channel_type, typename composite_type>
inline void inverseOver(const int numChannels, const int *channelIndex,
                        channel_type *dst, const channel_type *baseColor,
                        qreal dstOpacity)
{
    for (int i = 0; i < numChannels; i++) {
        const int idx = channelIndex[i];
        dst[idx] =
                KoColorSpaceMaths<channel_type>::clamp(
                    (static_cast<composite_type>(dst[idx]) - baseColor[idx]) / dstOpacity + baseColor[idx]);
    }
}

template<typename channel_type, typename composite_type>
void applyToIterator(const int numChannels, const int *channelIndex,
                     KisSequentialIteratorProgress &it, KoColor baseColor,
                     int threshold, const KoColorSpace *cs)
{
    qreal thresholdF = threshold;
    quint8 *baseColorData_uint8 = baseColor.data();
    channel_type *baseColorData = reinterpret_cast<channel_type*>(baseColorData_uint8);

    while (it.nextPixel()) {
        channel_type *dst = reinterpret_cast<channel_type*>(it.rawData());
        quint8 *dst_uint8 = it.rawData();

        quint8 diff = cs->difference(baseColorData_uint8, dst_uint8);

        qreal newOpacity = diff >= threshold ? 1.0 : diff / thresholdF;

        if(newOpacity < cs->opacityF(dst_uint8)) {
            cs->setOpacity(dst_uint8, newOpacity, 1);
        }

        inverseOver<channel_type, composite_type>(numChannels, channelIndex,
                                                  dst, baseColorData,
                                                  newOpacity);
    }
}

void KisFilterColorToAlpha::processImpl(KisPaintDeviceSP device,
                                        const QRect& rect,
                                        const KisFilterConfigurationSP _config,
                                        KoUpdater* progressUpdater
                                        ) const
{
    Q_ASSERT(device != 0);

    KisFilterConfigurationSP config = _config ? _config : new KisFilterConfiguration("colortoalpha", 1);

    QVariant value;
    QColor cTA = (config->getProperty("targetcolor", value)) ? value.value<QColor>() : QColor(255, 255, 255);
    int threshold = (config->getProperty("threshold", value)) ? value.toInt() : 1;

    const KoColorSpace * cs = device->colorSpace();

    KisSequentialIteratorProgress it(device, rect, progressUpdater);
    KoColor baseColor(cTA, cs);

    QVector<int> channelIndex;
    KoChannelInfo::enumChannelValueType valueType = KoChannelInfo::OTHER;

    QList<KoChannelInfo*> channels = cs->channels();

    for (int i = 0; i < channels.size(); i++) {
        const KoChannelInfo *info = channels[i];

        if (info->channelType() != KoChannelInfo::COLOR) continue;

        KoChannelInfo::enumChannelValueType currentValueType =
                info->channelValueType();

        if (valueType != KoChannelInfo::OTHER &&
                valueType != currentValueType) {

            warnKrita << "Cannot apply a Color-to-Alpha filter to a heterogeneous colorspace";
            return;
        } else {
            valueType = currentValueType;
        }

        channelIndex.append(i);
    }

    switch (valueType) {
    case KoChannelInfo::UINT8:
        applyToIterator<quint8, qint16>(channelIndex.size(), channelIndex.data(),
                                        it, baseColor,
                                        threshold, cs);
        break;
    case KoChannelInfo::UINT16:
        applyToIterator<quint16, qint32>(channelIndex.size(), channelIndex.data(),
                                         it, baseColor,
                                         threshold, cs);
        break;
    case KoChannelInfo::UINT32:
        applyToIterator<quint32, qint64>(channelIndex.size(), channelIndex.data(),
                                         it, baseColor,
                                         threshold, cs);
        break;

    case KoChannelInfo::FLOAT32:
        applyToIterator<float, float>(channelIndex.size(), channelIndex.data(),
                                      it, baseColor,
                                      threshold, cs);
        break;
    case KoChannelInfo::FLOAT64:
        applyToIterator<double, double>(channelIndex.size(), channelIndex.data(),
                                        it, baseColor,
                                        threshold, cs);
        break;
    case KoChannelInfo::FLOAT16:
#ifdef HAVE_OPENEXR
#include <half.h>
        applyToIterator<half, half>(channelIndex.size(), channelIndex.data(),
                                    it, baseColor,
                                    threshold, cs);
        break;

#endif
    case KoChannelInfo::INT8: /* !UNSUPPORTED! */
    case KoChannelInfo::INT16: /* !UNSUPPORTED! */

    case KoChannelInfo::OTHER:
        warnKrita << "Color To Alpha: Unsupported channel type:" << valueType;
    }
}
