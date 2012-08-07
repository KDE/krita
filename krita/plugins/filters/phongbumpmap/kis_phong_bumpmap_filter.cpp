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
#include "KoUpdater.h"
#include "kis_math_toolbox.h"
#include "KoColorSpaceRegistry.h"
#include <filter/kis_filter_configuration.h>
#include "kis_iterator_ng.h"
#include "kundo2command.h"
#include "kis_painter.h"

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
                                    const KisFilterConfiguration *config,
                                    KoUpdater *progressUpdater
                                    ) const
{
    if (!config) return;

    if (progressUpdater) progressUpdater->setProgress(0);
    
    // Benchmark
    QTime timer, timerE;
    
    QString userChosenHeightChannel = config->getString(PHONG_HEIGHT_CHANNEL, "FAIL");

    if (userChosenHeightChannel == "FAIL") {
        qDebug("FIX YOUR FILTER");
        return;
    }
    
    timer.start();

    KoChannelInfo *m_heightChannel = 0;

    foreach (KoChannelInfo* channel, device->colorSpace()->channels()) {
        if (userChosenHeightChannel == channel->name()) {
            m_heightChannel = channel;
        }
    }

    QRect inputArea = applyRect;
    QRect outputArea = applyRect;
    
    inputArea.adjust(-1, -1, 1, 1);

    quint32 posup;
    quint32 posdown;
    quint32 posleft;
    quint32 posright;
    QColor I; //Reflected light

    if (progressUpdater) progressUpdater->setProgress(1);

    //======Preparation paraphlenalia=======

    //Hardcoded facts about Phong Bumpmap: it _will_ generate an RGBA16 bumpmap
    const quint8    BYTE_DEPTH_OF_BUMPMAP    = 2;      // 16 bits per channel
    const quint8    CHANNEL_COUNT_OF_BUMPMAP = 4;      // RGBA
    const quint32   pixelsOfInputArea        = abs(inputArea.width() * inputArea.height());
    const quint32   pixelsOfOutputArea       = abs(outputArea.width() * outputArea.height());
    const quint8    pixelSize                = BYTE_DEPTH_OF_BUMPMAP * CHANNEL_COUNT_OF_BUMPMAP;
    const quint32   bytesToFillBumpmapArea   = pixelsOfOutputArea * pixelSize;
    QVector<quint8> bumpmap(bytesToFillBumpmapArea);
    quint8         *bumpmapDataPointer       = bumpmap.data();
    quint32         ki                       = KoChannelInfo::displayPositionToChannelIndex(m_heightChannel->displayPosition(),
                                                                                            device->colorSpace()->channels());
    PhongPixelProcessor tileRenderer(pixelsOfInputArea, config);

    if (progressUpdater) progressUpdater->setProgress(2);

    //===============RENDER=================

    QVector<PtrToDouble> toDoubleFuncPtr(device->colorSpace()->channels().count());
    KisMathToolbox *mathToolbox = KisMathToolboxRegistry::instance()->value(device->colorSpace()->mathToolboxId().id());
    if (!mathToolbox->getToDoubleChannelPtr(device->colorSpace()->channels(), toDoubleFuncPtr)) {
        return;
    }

    KisHLineConstIteratorSP iterator;
    quint32 curPixel = 0;
    iterator = device->createHLineConstIteratorNG(inputArea.x(),
                                             inputArea.y(),
                                             inputArea.width()
                                             );

    for (qint32 srcRow = 0; srcRow < inputArea.height(); ++srcRow) {
        do {
            const quint8 *data = iterator->oldRawData();
            tileRenderer.realheightmap[curPixel] = toDoubleFuncPtr[ki](data, device->colorSpace()->channels()[ki]->pos());
            curPixel++;
        }
        while (iterator->nextPixel());
        iterator->nextRow();
    }

    if (progressUpdater) progressUpdater->setProgress(50);

    const int tileHeightMinus1 = inputArea.height() - 1;
    const int tileWidthMinus1 = inputArea.width() - 1;

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

    if (progressUpdater) progressUpdater->setProgress(90);

    KisPaintDeviceSP bumpmapPaintDevice = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb16());
    bumpmapPaintDevice->writeBytes(bumpmap.data(), outputArea.x(), outputArea.y(), outputArea.width(), outputArea.height());
    KUndo2Command *leaker = bumpmapPaintDevice->convertTo(device->colorSpace(), KoColorConversionTransformation::IntentPerceptual, KoColorConversionTransformation::BlackpointCompensation);
    KisPainter copier(device);
    copier.bitBlt(outputArea.x(), outputArea.y(), bumpmapPaintDevice,
                  outputArea.x(), outputArea.y(), outputArea.width(), outputArea.height());
    //device->prepareClone(bumpmapPaintDevice);
    //device->makeCloneFrom(bumpmapPaintDevice, bumpmapPaintDevice->extent());  // THIS COULD BE BUG GY
    
    delete leaker;
    if (progressUpdater) progressUpdater->setProgress(100);
}

KisFilterConfiguration *KisFilterPhongBumpmap::factoryConfiguration(const KisPaintDeviceSP) const
{
    KisFilterConfiguration *config = new KisFilterConfiguration(id(), 2);
    config->setProperty(PHONG_AMBIENT_REFLECTIVITY, 0.2);
    config->setProperty(PHONG_DIFFUSE_REFLECTIVITY, 0.5);
    config->setProperty(PHONG_SPECULAR_REFLECTIVITY, 0.3);
    config->setProperty(PHONG_SHINYNESS_EXPONENT, 2);
    config->setProperty(PHONG_DIFFUSE_REFLECTIVITY_IS_ENABLED, true);
    config->setProperty(PHONG_SPECULAR_REFLECTIVITY_IS_ENABLED, true);
    // Indexes are off by 1 simply because arrays start at 0 and the GUI naming scheme started at 1
    config->setProperty(PHONG_ILLUMINANT_IS_ENABLED[0], true);
    config->setProperty(PHONG_ILLUMINANT_IS_ENABLED[1], true);
    config->setProperty(PHONG_ILLUMINANT_IS_ENABLED[2], false);
    config->setProperty(PHONG_ILLUMINANT_IS_ENABLED[3], false);
    config->setProperty(PHONG_ILLUMINANT_COLOR[0], QColor(255, 255, 0));
    config->setProperty(PHONG_ILLUMINANT_COLOR[1], QColor(255, 0, 0));
    config->setProperty(PHONG_ILLUMINANT_COLOR[2], QColor(0, 0, 255));
    config->setProperty(PHONG_ILLUMINANT_COLOR[3], QColor(0, 255, 0));
    config->setProperty(PHONG_ILLUMINANT_AZIMUTH[0], 50);
    config->setProperty(PHONG_ILLUMINANT_AZIMUTH[1], 100);
    config->setProperty(PHONG_ILLUMINANT_AZIMUTH[2], 150);
    config->setProperty(PHONG_ILLUMINANT_AZIMUTH[3], 200);
    config->setProperty(PHONG_ILLUMINANT_INCLINATION[0], 25);
    config->setProperty(PHONG_ILLUMINANT_INCLINATION[1], 20);
    config->setProperty(PHONG_ILLUMINANT_INCLINATION[2], 30);
    config->setProperty(PHONG_ILLUMINANT_INCLINATION[3], 40);
    return config;
}

QRect KisFilterPhongBumpmap::neededRect(const QRect &rect, const KisFilterConfiguration* /*config*/) const
{
    return rect.adjusted(-1, -1, 1, 1);
}

QRect KisFilterPhongBumpmap::changedRect(const QRect &rect, const KisFilterConfiguration* /*config*/) const
{
    return rect;
}

KisConfigWidget *KisFilterPhongBumpmap::createConfigurationWidget(QWidget *parent, const KisPaintDeviceSP dev, const KisImageWSP image) const
{
    KisPhongBumpmapConfigWidget *w = new KisPhongBumpmapConfigWidget(dev, image, parent);
    return w;
}


