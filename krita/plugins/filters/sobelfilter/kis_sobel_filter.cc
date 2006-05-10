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

#include <stdlib.h>
#include <vector>

#include <QPoint>
#include <QSpinBox>

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <knuminput.h>

#include <kis_doc.h>
#include <kis_image.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <kis_filter_registry.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_progress_display_interface.h>

#include "kis_multi_bool_filter_widget.h"
#include "kis_sobel_filter.h"

#define MIN(a,b) (((a)<(b))?(a):(b))

void KisSobelFilterConfiguration::fromXML(const QString & s)
{
    KisFilterConfiguration::fromXML(s);
    m_doHorizontally = getBool( "doHorizontally" );
    m_doVertically = getBool( "doVertically" );
    m_keepSign = getBool( "makeOpaque" );
}

QString KisSobelFilterConfiguration::toString()
{
    m_properties.clear();
    setProperty("doHorizontally", m_doHorizontally);
    setProperty("doVertically", m_doVertically);
    setProperty("keepSign", m_keepSign);
    setProperty("makeOpaque", m_makeOpaque);

    return KisFilterConfiguration::toString();
}

KisSobelFilter::KisSobelFilter() : KisFilter(id(), "edge", i18n("&Sobel..."))
{
}

void KisSobelFilter::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* configuration, const QRect& rect)
{
    //read the filter configuration values from the KisFilterConfiguration object
    bool doHorizontally = ((KisSobelFilterConfiguration*)configuration)->doHorizontally();
    bool doVertically = ((KisSobelFilterConfiguration*)configuration)->doVertically();
    bool keepSign = ((KisSobelFilterConfiguration*)configuration)->keepSign();
    bool makeOpaque = ((KisSobelFilterConfiguration*)configuration)->makeOpaque();

    //pixelize(src, dst, x, y, width, height, pixelWidth, pixelHeight);
    sobel(rect, src, dst, doHorizontally, doVertically, keepSign, makeOpaque);
}

void KisSobelFilter::prepareRow (KisPaintDeviceSP src, quint8* data, quint32 x, quint32 y, quint32 w, quint32 h)
{
    if (y > h -1) y = h -1;
    quint32 pixelSize = src->pixelSize();

    src->readBytes( data, x, y, w, 1 );

    for (quint32 b = 0; b < pixelSize; b++) {
        int offset = pixelSize - b;
        data[-offset] = data[b];
        data[w * pixelSize + b] = data[(w - 1) * pixelSize + b];
    }
}

#define RMS(a, b) (sqrt ((a) * (a) + (b) * (b)))
#define ROUND(x) ((int) ((x) + 0.5))

void KisSobelFilter::sobel(const QRect & rc, KisPaintDeviceSP src, KisPaintDeviceSP dst, bool doHorizontal, bool doVertical, bool keepSign, bool makeOpaque)
{
    QRect rect = rc; //src->exactBounds();
    quint32 x = rect.x();
    quint32 y = rect.y();
    quint32 width = rect.width();
    quint32 height = rect.height();
    quint32 pixelSize = src->pixelSize();

    setProgressTotalSteps( height );
    setProgressStage(i18n("Applying sobel filter..."),0);

    /*  allocate row buffers  */
    quint8* prevRow = new quint8[ (width + 2) * pixelSize];
    Q_CHECK_PTR(prevRow);
    quint8* curRow = new quint8[ (width + 2) * pixelSize];
    Q_CHECK_PTR(curRow);
    quint8* nextRow = new quint8[ (width + 2) * pixelSize];
    Q_CHECK_PTR(nextRow);
    quint8* dest = new quint8[ width  * pixelSize];
    Q_CHECK_PTR(dest);

    quint8* pr = prevRow + pixelSize;
    quint8* cr = curRow + pixelSize;
    quint8* nr = nextRow + pixelSize;

    prepareRow (src, pr, x, y - 1, width, height);
    prepareRow (src, cr, x, y, width, height);

    quint32 counter =0;
    quint8* d;
    quint8* tmp;
    qint32 gradient, horGradient, verGradient;
    // loop through the rows, applying the sobel convolution
    for (quint32 row = 0; row < height; row++)
        {

            // prepare the next row
            prepareRow (src, nr, x, row + 1, width, height);
            d = dest;

            for (quint32 col = 0; col < width * pixelSize; col++)
                {
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
                        (ROUND (RMS (horGradient, verGradient)) / 5.66) // always >0
                        : (keepSign ? (127 + (ROUND ((horGradient + verGradient) / 8.0)))
                        : (ROUND (QABS (horGradient + verGradient) / 4.0))));

                    *d++ = gradient;
                    if (gradient > 10) counter ++;
                }

            //  shuffle the row pointers
            tmp = pr;
            pr = cr;
            cr = nr;
            nr = tmp;

            //store the dest
            dst->writeBytes(dest, x, row, width, 1);

            if ( makeOpaque )
                {
                    KisHLineIteratorPixel dstIt = dst->createHLineIterator(x, row, width, true);
                    while( ! dstIt.isDone() )
                        {
                            dstIt.rawData()[pixelSize-1]=255; //XXXX: is the alpha channel always 8 bit? Otherwise this is wrong!
                            ++dstIt;
                        }
                }
            setProgress(row);
        }
    setProgressDone();

    delete[] prevRow;
    delete[] curRow;
    delete[] nextRow;
    delete[] dest;
}


KisFilterConfigWidget * KisSobelFilter::createConfigurationWidget(QWidget* parent, KisPaintDeviceSP)
{
    vKisBoolWidgetParam param;
    param.push_back( KisBoolWidgetParam( true, i18n("Sobel horizontally"), "doHorizontally" ) );
    param.push_back( KisBoolWidgetParam( true, i18n("Sobel vertically"), "doVertically" ) );
    param.push_back( KisBoolWidgetParam( true, i18n("Keep sign of result"), "keepSign" ) );
    param.push_back( KisBoolWidgetParam( true, i18n("Make image opaque"), "makeOpaque" ) );
    return new KisMultiBoolFilterWidget(parent, id().id().toAscii(), id().id(), param );
}

KisFilterConfiguration* KisSobelFilter::configuration(QWidget* nwidget)
{
    KisMultiBoolFilterWidget* widget = (KisMultiBoolFilterWidget*) nwidget;
    if( widget == 0 )
        {
            return new KisSobelFilterConfiguration( true, true, true, true);
        } else {
        return new KisSobelFilterConfiguration( widget->valueAt( 0 ), widget->valueAt( 1 ), widget->valueAt( 2 ), widget->valueAt( 3 ) );
    }
}
