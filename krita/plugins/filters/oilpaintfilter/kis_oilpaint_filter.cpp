/*
 * This file is part of Krita
 *
 * Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 * Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 * ported from digikam, Copyrighted by Gilles Caulier,
 * Original Oilpaint algorithm copyrighted 2004 by
 * Pieter Z. Voloshyn <pieter_voloshyn at ame.com.br>.
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

#include "kis_oilpaint_filter.h"

#include <stdlib.h>
#include <vector>

#include <QPoint>
#include <QSpinBox>

#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kis_debug.h>
#include <kpluginfactory.h>
#include <knuminput.h>

#include <KoProgressUpdater.h>
#include <KoUpdater.h>

#include <kis_doc2.h>
#include <kis_image.h>
#include "kis_iterator_ng.h"
#include <kis_layer.h>
#include <filter/kis_filter_registry.h>
#include <kis_global.h>
#include <kis_types.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>
#include <kis_paint_device.h>
#include <kis_iterator_ng.h>
#include "widgets/kis_multi_integer_filter_widget.h"


KisOilPaintFilter::KisOilPaintFilter() : KisFilter(id(), KisFilter::categoryArtistic(), i18n("&Oilpaint..."))
{
    setSupportsPainting(true);
    setSupportsThreading(false);
}

void KisOilPaintFilter::process(KisPaintDeviceSP device,
                         const QRect& applyRect,
                         const KisFilterConfiguration* config,
                         KoUpdater* progressUpdater
                               ) const
{
    QPoint srcTopLeft = applyRect.topLeft();
    Q_ASSERT(!device.isNull());
#if 1

    qint32 width = applyRect.width();
    qint32 height = applyRect.height();

    //read the filter configuration values from the KisFilterConfiguration object
    quint32 brushSize = config->getInt("brushSize", 1);
    quint32 smooth = config->getInt("smooth", 30);


    OilPaint(device, device, srcTopLeft, applyRect.topLeft(), width, height, brushSize, smooth, progressUpdater);
#endif
}

// This method have been ported from Pieter Z. Voloshyn algorithm code.

/* Function to apply the OilPaint effect.
 *
 * data             => The image data in RGBA mode.
 * w                => Width of image.
 * h                => Height of image.
 * BrushSize        => Brush size.
 * Smoothness       => Smooth value.
 *
 * Theory           => Using MostFrequentColor function we take the main color in
 *                     a matrix and simply write at the original position.
 */

void KisOilPaintFilter::OilPaint(const KisPaintDeviceSP src, KisPaintDeviceSP dst, const QPoint& srcTopLeft, const QPoint& dstTopLeft, int w, int h,
                                 int BrushSize, int Smoothness, KoUpdater* progressUpdater) const
{
    if (progressUpdater) {
        progressUpdater->setRange(0, w * h);
    }

    QRect bounds(srcTopLeft.x(), srcTopLeft.y(), w, h);

    KisHLineConstIteratorSP it = src->createHLineConstIteratorNG(srcTopLeft.x(), srcTopLeft.y(), w);
    KisHLineIteratorSP dstIt = dst->createHLineIteratorNG(dstTopLeft.x(), dstTopLeft.y(), w);

    int progress = 0;
    for (qint32 yOffset = 0; yOffset < h; yOffset++) {
        do {  //&& !cancelRequested()) {
                MostFrequentColor(src, dstIt->rawData(), bounds, it->x(), it->y(), BrushSize, Smoothness);
        }  while (it->nextPixel() && dstIt->nextPixel());
        it->nextRow();
        dstIt->nextRow();

        if (progressUpdater) progressUpdater->setValue(progress += w);
    }
}

// This method has been ported from Pieter Z. Voloshyn's algorithm code in Digikam.

/* Function to determine the most frequent color in a matrix
 *
 * Bits             => Bits array
 * Width            => Image width
 * Height           => Image height
 * X                => Position horizontal
 * Y                => Position vertical
 * Radius           => Is the radius of the matrix to be analyzed
 * Intensity        => Intensity to calculate
 *
 * Theory           => This function creates a matrix with the analyzed pixel in
 *                     the center of this matrix and find the most frequenty color
 */

void KisOilPaintFilter::MostFrequentColor(KisPaintDeviceSP src, quint8* dst, const QRect& bounds, int X, int Y, int Radius, int Intensity) const
{
    uint I;

    double Scale = Intensity / 255.0;

    // Alloc some arrays to be used
    uchar *IntensityCount = new uchar[(Intensity + 1) * sizeof(uchar)];

    const KoColorSpace* cs = src->colorSpace();

    QVector<float> channel(cs->channelCount());
    QVector<float>* AverageChannels = new QVector<float>[(Intensity + 1)];

    // Erase the array
    memset(IntensityCount, 0, (Intensity + 1) * sizeof(uchar));

    int startx = qMax(X - Radius, bounds.left());
    int starty = qMax(Y - Radius, bounds.top());
    int width = (2 * Radius) + 1;
    if ((startx + width - 1) > bounds.right()) width = bounds.right() - startx + 1;
    Q_ASSERT((startx + width - 1) <= bounds.right());
    int height = (2 * Radius) + 1;
    if ((starty + height) > bounds.bottom()) height = bounds.bottom() - starty + 1;
    Q_ASSERT((starty + height - 1) <= bounds.bottom());
    KisRectIteratorSP it = src->createRectIteratorNG(startx, starty, width, height);
    do {

        cs->normalisedChannelsValue(it->rawData(), channel);

        I = (uint)(cs->intensity8(it->rawData()) * Scale);
        IntensityCount[I]++;

        if (IntensityCount[I] == 1) {
            AverageChannels[I] = channel;
        } else {
            for (int i = 0; i < channel.size(); i++) {
                AverageChannels[I][i] += channel[i];
            }
        }
    } while (it->nextPixel());

    I = 0;
    int MaxInstance = 0;

    for (int i = 0 ; i <= Intensity ; ++i) {
        if (IntensityCount[i] > MaxInstance) {
            I = i;
            MaxInstance = IntensityCount[i];
        }
    }

    if (MaxInstance != 0) {
        channel = AverageChannels[I];
        for (int i = 0; i < channel.size(); i++) {
            channel[i] /= MaxInstance;
        }
        cs->fromNormalisedChannelsValue(dst, channel);
    } else {
        memset(dst, 0, cs->pixelSize());
        cs->setOpacity(dst, OPACITY_OPAQUE_U8, 1);
    }


    delete [] IntensityCount;        // free all the arrays
    delete [] AverageChannels;
}


KisConfigWidget * KisOilPaintFilter::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP, const KisImageWSP) const
{
    vKisIntegerWidgetParam param;
    param.push_back(KisIntegerWidgetParam(1, 5, 1, i18n("Brush size"), "brushSize"));
    param.push_back(KisIntegerWidgetParam(10, 255, 30, i18nc("smooth out the painting strokes the filter creates", "Smooth"), "smooth"));
    return new KisMultiIntegerFilterWidget(id().id(),  parent,  id().id(),  param);
}

KisFilterConfiguration* KisOilPaintFilter::factoryConfiguration(const KisPaintDeviceSP) const
{
    KisFilterConfiguration* config = new KisFilterConfiguration("oilpaint", 1);
    config->setProperty("brushSize", 1);
    config->setProperty("smooth", 30);
    return config;
}
