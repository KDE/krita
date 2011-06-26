/*
 *  Copyright (c) 2010-2011 José Luis Vergara <pentalis@gmail.com>
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

#include "kis_phong_bumpmap_filter.h"
#include "kis_phong_bumpmap_config_widget.h"
#include "phong_pixel_processor.h"

#include "kis_paint_device.h"
#include "kis_config_widget.h"
#include "kis_math_toolbox.h"
#include "KoColorSpaceRegistry.h"
#include <filter/kis_filter_configuration.h>
#include "kis_iterator_ng.h"

KisFilterPhongBumpmap::KisFilterPhongBumpmap()
                      : KisFilter(KoID("phongbumpmap"     , i18n("PhongBumpmap")),
                                  KisFilter::categoryMap(), i18n("&PhongBumpmap..."))
{
    setColorSpaceIndependence(TO_LAB16);
    setSupportsPainting(true);
    setSupportsIncrementalPainting(true);
}

void KisFilterPhongBumpmap::process(KisPaintDeviceSP device,
                                    const QRect& applyRect,
                                    const KisFilterConfiguration* config,
                                    KoUpdater* /*progressUpdater*/
                                    ) const
{
#ifdef __GNUC__
    #warning TODO: implement progress updater for phong bumpmap
#endif

    // Benchmark
    QTime timer, timerE;

    QString userChosenHeightChannel = config->getString(PHONG_HEIGHT_CHANNEL, "FAIL");

    if (userChosenHeightChannel == "FAIL") {
        qDebug("FIX YOUR FILTER");
        return;
    }

    // TODO complete this part
    userChosenHeightChannel = "Red";
    
    timer.start();

    KoChannelInfo* m_heightChannel = 0;

    foreach (KoChannelInfo* channel, device->colorSpace()->channels()) {
        if (userChosenHeightChannel == channel->name()) {
            m_heightChannel = channel;
        }
    }

    QRect inputArea = applyRect;
    inputArea.adjust(-1, -1, 1, 1);
    QRect outputArea = inputArea.adjusted(1, 1, -1, -1);

    quint32 posup;
    quint32 posdown;
    quint32 posleft;
    quint32 posright;
    QRect tileLimits;
    QColor I; //Reflected light

    //======Preparation paraphlenalia=======

    //Hardcoded facts about Phong Bumpmap: it _will_ generate an RGBA16 bumpmap
    const quint8    BYTE_DEPTH_OF_BUMPMAP    = 2;      // 16 bits per channel
    const quint8    CHANNEL_COUNT_OF_BUMPMAP = 4;      // RGBA
    const quint32   pixelsOfOutputArea       = abs(outputArea.width() * outputArea.height());
    const quint8    pixelSize                = BYTE_DEPTH_OF_BUMPMAP * CHANNEL_COUNT_OF_BUMPMAP;
    const quint32   bytesToFillBumpmapArea   = pixelsOfOutputArea * pixelSize;

    QVector<quint8> bumpmap(bytesToFillBumpmapArea);
    quint8         *bumpmapDataPointer       = bumpmap.data();
    quint32         ki                       = m_heightChannel->index();
    PhongPixelProcessor tileRenderer(config);

    //===============RENDER=================
    
    QVector<PtrToDouble> toDoubleFuncPtr(device->colorSpace()->channels().count());
    KisMathToolbox *mathToolbox = KisMathToolboxRegistry::instance()->value(device->colorSpace()->mathToolboxId().id());
    if (!mathToolbox->getToDoubleChannelPtr(device->colorSpace()->channels(), toDoubleFuncPtr)) {
        return;
    }
    
    KisHLineIteratorSP iterator;
    quint32 curPixel = 0;
    iterator = device->createHLineIteratorNG(inputArea.x(),
                                             inputArea.y(),
                                             inputArea.width()
                                             );
    curPixel = 0;
    
    for (qint32 srcRow = 0; srcRow < inputArea.height(); ++srcRow) {
        do {
            const quint8 *data = iterator->rawData();
            tileRenderer.realheightmap[curPixel] = toDoubleFuncPtr[ki](data, device->colorSpace()->channels()[ki]->pos());
            curPixel++;
        }
        while (iterator->nextPixel());
        iterator->nextRow();
    }

    const int tileHeightMinus1 = inputArea.width() - 1;
    const int tileWidthMinus1 = inputArea.height() - 1;

    // Foreach INNER pixel in tile
    for (int y = 1; y < tileHeightMinus1; ++y) {
        for (int x = 1; x < tileWidthMinus1; ++x) {
            posup   = (y + 1) * inputArea.width() + x;
            posdown = (y - 1) * inputArea.width() + x;
            posleft  = y * inputArea.width() + x - 1;
            posright = y * inputArea.width() + x + 1;

            memcpy(bumpmapDataPointer,
                   tileRenderer.testingHeightmapIlluminatePixel(posup, posdown, posleft, posright).data(),
                   pixelSize);
            bumpmapDataPointer += pixelSize;
        }
    }

    KisPaintDeviceSP bumpmapPaintDevice = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb16());
    bumpmapPaintDevice->writeBytes(bumpmap.data(), outputArea.x(), outputArea.y(), outputArea.width(), outputArea.height());
    bumpmapPaintDevice->convertTo(device->colorSpace());
    device->makeCloneFrom(bumpmapPaintDevice, bumpmapPaintDevice->extent());  // THIS COULD BE BUG GY
}

KisFilterConfiguration *KisFilterPhongBumpmap::factoryConfiguration(const KisPaintDeviceSP) const
{
    KisFilterConfiguration *config = new KisFilterConfiguration(id(), 0);
    return config;
}

QRect KisFilterPhongBumpmap::neededRect(const QRect &rect, const KisFilterConfiguration* /*config*/) const
{
    return rect.adjusted(-2, -2, 2, 2);
}

QRect KisFilterPhongBumpmap::changedRect(const QRect &rect, const KisFilterConfiguration* /*config*/) const
{
    return rect.adjusted(-2, -2, 2, 2);
}

KisConfigWidget *KisFilterPhongBumpmap::createConfigurationWidget(QWidget *parent, const KisPaintDeviceSP dev, const KisImageWSP image) const
{
    KisPhongBumpmapConfigWidget *w = new KisPhongBumpmapConfigWidget(dev, image, parent);
    return w;
}


