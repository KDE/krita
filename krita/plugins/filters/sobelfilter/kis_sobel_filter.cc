/*
 * This file is part of Krita
 *
 * Copyright (c) 2005 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 * ported from Gimp, Copyright (C) 1997 Eiichi Takamori <taka@ma1.seikyou.ne.jp>
 * original pixelize.c for GIMP 0.54 by Tracy Scott
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

#include "kis_sobel_filter.h"

#include <stdlib.h>
#include <vector>

#include <QPoint>
#include <QSpinBox>

#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kis_debug.h>
#include <kgenericfactory.h>
#include <knuminput.h>

#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <filter/kis_filter_registry.h>
#include <kis_global.h>
#include <kis_selection.h>
#include <kis_types.h>
#include <kis_paint_device.h>
#include <KoProgressUpdater.h>
#include <KoUpdater.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>

#include "widgets/kis_multi_bool_filter_widget.h"


#define MIN(a,b) (((a)<(b))?(a):(b))

// void KisSobelFilterConfiguration::fromXML(const QString & s)
// {
//     KisFilterConfiguration::fromXML(s);
//     m_doHorizontally = getBool( "doHorizontally" );
//     m_doVertically = getBool( "doVertically" );
//     m_keepSign = getBool( "makeOpaque" );
// }
//
// QString KisSobelFilterConfiguration::toString()
// {
//     m_properties.clear();
//     setProperty("doHorizontally", m_doHorizontally);
//     setProperty("doVertically", m_doVertically);
//     setProperty("keepSign", m_keepSign);
//     setProperty("makeOpaque", m_makeOpaque);
//
//     return KisFilterConfiguration::toString();
// }

KisSobelFilter::KisSobelFilter() : KisFilter(id(), categoryEdgeDetection(), i18n("&Sobel..."))
{
    setSupportsPainting(false);
    setSupportsPreview(true);
    setSupportsThreading(false); // TODO Sobel doesn't support threading on image with height > 512
}


void KisSobelFilter::prepareRow(KisPaintDeviceSP src, quint8* data, quint32 x, quint32 y, quint32 w, quint32 h) const
{
    if (y > h - 1) y = h - 1;
    quint32 pixelSize = src->pixelSize();

    src->readBytes(data, x, y, w, 1);

    for (quint32 b = 0; b < pixelSize; b++) {
        int offset = pixelSize - b;
        data[-offset] = data[b];
        data[w * pixelSize + b] = data[(w - 1) * pixelSize + b];
    }
}

#define RMS(a, b) (sqrt ((a) * (a) + (b) * (b)))
#define ROUND(x) ((int) ((x) + 0.5))

void KisSobelFilter::process(KisConstProcessingInformation srcInfo,
                             KisProcessingInformation dstInfo,
                             const QSize& size,
                             const KisFilterConfiguration* configuration,
                             KoUpdater* progressUpdater
                            ) const
{
    const KisPaintDeviceSP src = srcInfo.paintDevice();
    KisPaintDeviceSP dst = dstInfo.paintDevice();
    QPoint dstTopLeft = dstInfo.topLeft();
    QPoint srcTopLeft = srcInfo.topLeft();
    Q_ASSERT(!src.isNull());
    Q_ASSERT(!dst.isNull());


    //read the filter configuration values from the KisFilterConfiguration object
    bool doHorizontal = configuration->getBool("doHorizontally", true);
    bool doVertical = configuration->getBool("doVertically", true);
    bool keepSign = configuration->getBool("keepSign", true);
    bool makeOpaque = configuration->getBool("makeOpaque", true);

    quint32 width = size.width();
    quint32 height = size.height();
    quint32 pixelSize = src->pixelSize();

    int cost = size.height();

    /*  allocate row buffers  */
    quint8* prevRow = new quint8[(width + 2) * pixelSize];
    Q_CHECK_PTR(prevRow);
    quint8* curRow = new quint8[(width + 2) * pixelSize];
    Q_CHECK_PTR(curRow);
    quint8* nextRow = new quint8[(width + 2) * pixelSize];
    Q_CHECK_PTR(nextRow);
    quint8* dest = new quint8[ width  * pixelSize];
    Q_CHECK_PTR(dest);

    quint8* pr = prevRow + pixelSize;
    quint8* cr = curRow + pixelSize;
    quint8* nr = nextRow + pixelSize;

    prepareRow(src, pr, srcTopLeft.x(), srcTopLeft.y() - 1, width, height);
    prepareRow(src, cr, srcTopLeft.x(), srcTopLeft.y(), width, height);

    quint32 counter = 0;
    quint8* d;
    quint8* tmp;
    qint32 gradient, horGradient, verGradient;
    // loop through the rows, applying the sobel convolution

    KisHLineIteratorPixel dstIt = dst->createHLineIterator(dstTopLeft.x(), dstTopLeft.y(), width, dstInfo.selection());

    for (quint32 row = 0; row < height; row++) {

        // prepare the next row
        prepareRow(src, nr, srcTopLeft.x(), srcTopLeft.y() + row + 1, width, height);
        d = dest;

        for (quint32 col = 0; col < width * pixelSize; col++) {
            int positive = col + pixelSize;
            int negative = col - pixelSize;
            horGradient = (doHorizontal ?
                           ((pr[negative] +  2 * pr[col] + pr[positive]) -
                            (nr[negative] + 2 * nr[col] + nr[positive]))
                           : 0);

            verGradient = (doVertical ?
                           ((pr[negative] + 2 * cr[negative] + nr[negative]) -
                            (pr[positive] + 2 * cr[positive] + nr[positive]))
                           : 0);
            gradient = (qint32)((doVertical && doHorizontal) ?
                                (ROUND(RMS(horGradient, verGradient)) / 5.66)   // always >0
                                : (keepSign ? (127 + (ROUND((horGradient + verGradient) / 8.0)))
                                   : (ROUND(qAbs(horGradient + verGradient) / 4.0))));

            *d++ = gradient;
            if (gradient > 10) counter ++;
        }

        //  shuffle the row pointers
        tmp = pr;
        pr = cr;
        cr = nr;
        nr = tmp;

        //store the dest
        dst->writeBytes(dest, dstTopLeft.x(), row, width, 1);

        if (makeOpaque) {
            while (! dstIt.isDone()) {
                dst->colorSpace()->setAlpha(dstIt.rawData(), 255, 1);
                ++dstIt;
            }
            dstIt.nextRow();
        }
        if (progressUpdater) progressUpdater->setProgress(row / cost);
    }

    delete[] prevRow;
    delete[] curRow;
    delete[] nextRow;
    delete[] dest;
}


KisConfigWidget * KisSobelFilter::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP, const KisImageWSP) const
{
    vKisBoolWidgetParam param;
    param.push_back(KisBoolWidgetParam(true, i18n("Sobel horizontally"), "doHorizontally"));
    param.push_back(KisBoolWidgetParam(true, i18n("Sobel vertically"), "doVertically"));
    param.push_back(KisBoolWidgetParam(true, i18n("Keep sign of result"), "keepSign"));
    param.push_back(KisBoolWidgetParam(true, i18n("Make image opaque"), "makeOpaque"));
    return new KisMultiBoolFilterWidget(id().id(), parent, id().id(), param);
}
