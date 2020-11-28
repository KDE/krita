/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2008 Emanuele Tamponi <emanuele@valinor.it>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_bidirectional_mixing_option.h"
#include <klocalizedstring.h>

#include <QLabel>
#include <QVector>
#include <QRect>

#include <KoColorSpace.h>
#include <KoCompositeOpRegistry.h>
#include <KoColor.h>

#include <kis_painter.h>
#include <kis_fixed_paint_device.h>
#include <kis_paint_device.h>
#include <kis_properties_configuration.h>
#include "kis_bidirectional_mixing_option_widget.h"
#include <kis_iterator_ng.h>

KisBidirectionalMixingOption::KisBidirectionalMixingOption()
    : m_mixingEnabled(false)
{
}


KisBidirectionalMixingOption::~KisBidirectionalMixingOption()
{
}

void KisBidirectionalMixingOption::apply(KisPaintDeviceSP dab, KisPaintDeviceSP device, KisPainter* painter, qint32 sx, qint32 sy, qint32 sw, qint32 sh, quint8 pressure, const QRect& dstRect)
{
    if (!m_mixingEnabled) return;

    const KoColorSpace *cs = dab->colorSpace();
    KisPaintDeviceSP canvas = new KisPaintDevice(cs);
    KisPainter p(canvas);
    p.setCompositeOp(COMPOSITE_COPY);
    p.bitBlt(sx, sy, device, dstRect.x(), dstRect.y(), sw, sh);

    int count = cs->channelCount();
    QRect srcRect(sx, sy, sw, sh);
    KisSequentialConstIterator cit(canvas, srcRect);
    KisSequentialIterator dit(dab, srcRect);
    QVector<float> cc(count), dc(count);
    while (cit.nextPixel() && dit.nextPixel()) {
        if (cs->opacityU8(dit.rawData()) > 10 && cs->opacityU8(cit.rawDataConst()) > 10) {

            cs->normalisedChannelsValue(cit.rawDataConst(), cc);
            cs->normalisedChannelsValue(dit.rawData(), dc);

            for (int i = 0; i < count; i++) {
                dc[i] = (1.0 - 0.4 * pressure) * cc[i] + 0.4 * pressure * dc[i];
            }

            cs->fromNormalisedChannelsValue(dit.rawData(), dc);

            if (dit.x() == (int)(sw / 2) && dit.y() == (int)(sh / 2)) {
                painter->setPaintColor(KoColor(dit.rawData(), cs));
            }
        }
    }
}

void KisBidirectionalMixingOption::applyFixed(KisFixedPaintDeviceSP dab, KisPaintDeviceSP device, KisPainter* painter, qint32 sx, qint32 sy, qint32 sw, qint32 sh, quint8 pressure, const QRect& dstRect)
{
    Q_UNUSED(sx);
    Q_UNUSED(sy);

    if (!m_mixingEnabled) return;

    KisFixedPaintDevice canvas(device->colorSpace());
    canvas.setRect(QRect(dstRect.x(), dstRect.y(), sw, sh));
    canvas.lazyGrowBufferWithoutInitialization();
    device->readBytes(canvas.data(), canvas.bounds());

    const KoColorSpace* cs = dab->colorSpace();
    int channelCount = cs->channelCount();

    quint8* dabPointer = dab->data();
    quint8* canvasPointer = canvas.data();

    QVector<float> cc(channelCount);
    QVector<float> dc(channelCount);

    for (int y = 0; y < sh; y++) {
        for (int x = 0; x < sw; x++) {
            if (cs->opacityU8(dabPointer) > 10 && cs->opacityU8(canvasPointer) > 10) {

                cs->normalisedChannelsValue(canvasPointer, cc);
                cs->normalisedChannelsValue(dabPointer, dc);

                for (int i = 0; i < channelCount ; i++) {
                    dc[i] = (1.0 - 0.4 * pressure) * cc[i] + 0.4 * pressure * dc[i];
                }

                cs->fromNormalisedChannelsValue(dabPointer, dc);

                if (x == (int)(sw / 2) && y == (int)(sh / 2)) {
                    painter->setPaintColor(KoColor(dabPointer, cs));
                }
            }
        }
        dabPointer += dab->pixelSize();
        canvasPointer += canvas.pixelSize();
    }

}

void KisBidirectionalMixingOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    m_mixingEnabled = setting->getBool(BIDIRECTIONAL_MIXING_ENABLED, false);
}

