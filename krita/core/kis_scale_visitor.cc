/*
 *  Copyright (c) 2004, 2005 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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
#include <qdatetime.h>

#include <kdebug.h>
#include <klocale.h>

#include "kis_paint_device_impl.h"
#include "kis_scale_visitor.h"
#include "kis_progress_display_interface.h"
#include "kis_filter_strategy.h"


void KisScaleVisitor::scale(double xscale, double yscale, KisProgressDisplayInterface * progress, KisFilterStrategy *filterStrategy)
{
    double fwidth = filterStrategy->support();

    QRect rect = m_dev -> exactBounds();
    Q_INT32 width = rect.width();
    Q_INT32 height =  rect.height();
    m_pixelSize=m_dev -> pixelSize();

    // compute size of target image
    if ( xscale == 1.0F && yscale == 1.0F ) {
        return;
    }
    Q_INT32 targetW = QABS( qRound( xscale * width ) );
    Q_INT32 targetH = QABS( qRound( yscale * height ) );

    Q_UINT8* newData = new Q_UINT8[targetW * targetH * m_pixelSize ];
    Q_CHECK_PTR(newData);

    double* weight = new double[ m_pixelSize ];    /* filter calculation variables */

    Q_UINT8* pel = new Q_UINT8[ m_pixelSize ];
    Q_CHECK_PTR(pel);

    Q_UINT8 *pel2 = new Q_UINT8[ m_pixelSize ];
    Q_CHECK_PTR(pel2);

    bool* bPelDelta = new bool[ m_pixelSize ];
    ContribList    *contribX;
    ContribList    contribY;
    const Q_INT32 BLACK_PIXEL=0;
    const Q_INT32 WHITE_PIXEL=255;


    // create intermediate row to hold vertical dst row zoom
    Q_UINT8 * tmp = new Q_UINT8[ width * m_pixelSize ];
    Q_CHECK_PTR(tmp);

    //create array of pointers to intermediate rows
    Q_UINT8 **tmpRows = new Q_UINT8*[ height ];

    //create array of pointers to intermediate rows that are actually used simultaneously and allocate memory for the rows
    Q_UINT8 **tmpRowsMem;
    if(yscale < 1.0)
    {
        tmpRowsMem = new Q_UINT8*[ (int)(fwidth / yscale * 2 + 1) ];
        for(int i = 0; i < (int)(fwidth / yscale * 2 + 1); i++)
        {
             tmpRowsMem[i] = new Q_UINT8[ width * m_pixelSize ];
             Q_CHECK_PTR(tmpRowsMem[i]);
        }
    }
    else
    {
        tmpRowsMem = new Q_UINT8*[ (int)(fwidth * 2 + 1) ];
        for(int i = 0; i < (int)(fwidth * 2 + 1); i++)
        {
            tmpRowsMem[i] = new Q_UINT8[ width * m_pixelSize ];
            Q_CHECK_PTR(tmpRowsMem[i]);
        }
    }
    //progress info
    m_cancelRequested = false;
    if ( progress )
        progress -> setSubject(this, true, true);
    emit notifyProgressStage(i18n("Scaling layer..."),0);

    // build x weights
    contribX = new ContribList[ targetW ];
    for(int x = 0; x < targetW; x++)
    {
        calcContrib(&contribX[x], xscale, fwidth, width, filterStrategy, x);
    }

    QTime starttime = QTime::currentTime ();

    for(int y = 0; y < targetH; y++)
    {
        //progress info
        emit notifyProgress((y * 100) / targetH);
        if (m_cancelRequested) {
                break;
        }

        // build y weights
        calcContrib(&contribY, yscale, fwidth, height, filterStrategy, y);

        //copy pixel data to temporary arrays
        for(int srcpos = 0; srcpos < contribY.n; srcpos++)
        {
            if (!(contribY.p[srcpos].m_pixel < 0 || contribY.p[srcpos].m_pixel >= height))
            {
                //tmpRows[contribY.p[srcpos].m_pixel] = new Q_UINT8[ width * m_pixelSize ];
                tmpRows[ contribY.p[srcpos].m_pixel ] = tmpRowsMem[ srcpos ];
                m_dev ->readBytes(tmpRows[contribY.p[srcpos].m_pixel], 0, contribY.p[srcpos].m_pixel, width, 1);
            }
        }

        /* Apply vert filter to make dst row in tmp. */
        for(int x = 0; x < width; x++)
        {
            for(int channel = 0; channel < m_pixelSize; channel++){
                weight[channel] = 0.0;
                bPelDelta[channel] = FALSE;
                pel[channel]=tmpRows[contribY.p[0].m_pixel][ x * m_pixelSize + channel ];
            }
            for(int srcpos = 0; srcpos < contribY.n; srcpos++)
            {
                if (!(contribY.p[srcpos].m_pixel < 0 || contribY.p[srcpos].m_pixel >= height)){
                    for(int channel = 0; channel < m_pixelSize; channel++)
                    {
                        pel2[channel]=tmpRows[contribY.p[srcpos].m_pixel][ x * m_pixelSize + channel ];
                        if(pel2[channel] != pel[channel]) bPelDelta[channel] = TRUE;
                            weight[channel] += pel2[channel] * contribY.p[srcpos].m_weight;
                    }
                }
            }

            for(int channel = 0; channel < m_pixelSize; channel++){
                weight[channel] = bPelDelta[channel] ? static_cast<int>(qRound(weight[channel])) : pel[channel];
                        tmp[ x * m_pixelSize + channel ] = static_cast<Q_UINT8>(CLAMP(weight[channel], BLACK_PIXEL, WHITE_PIXEL));
            }
        } /* next row in temp column */
        delete[] contribY.p;

        for(int x = 0; x < targetW; x++)
        {
            for(int channel = 0; channel < m_pixelSize; channel++){
                weight[channel] = 0.0;
                bPelDelta[channel] = FALSE;
                pel[channel] = tmp[ contribX[x].p[0].m_pixel * m_pixelSize + channel ];
            }
            for(int srcpos = 0; srcpos < contribX[x].n; srcpos++)
            {
                for(int channel = 0; channel < m_pixelSize; channel++){
                    pel2[channel] = tmp[ contribX[x].p[srcpos].m_pixel * m_pixelSize + channel ];
                    if(pel2[channel] != pel[channel])
                        bPelDelta[channel] = TRUE;
                    weight[channel] += pel2[channel] * contribX[x].p[srcpos].m_weight;
                }
            }
            for(int channel = 0; channel < m_pixelSize; channel++){
                weight[channel] = bPelDelta[channel] ? static_cast<int>(qRound(weight[channel])) : pel[channel];
                int currentPos = (y*targetW+x) * m_pixelSize; // try to be at least a little efficient
                if (weight[channel]<0)
                    newData[currentPos + channel] = 0;
                else if (weight[channel]>255)
                    newData[currentPos + channel] = 255;
                else
                    newData[currentPos + channel] = (uchar)weight[channel];
             }
        } /* next dst row */
    } /* next dst column */
    if(!m_cancelRequested){
        m_dev -> writeBytes( newData, 0, 0, targetW, targetH);
        m_dev -> crop(0, 0, targetW, targetH);
    }

    /* free the memory allocated for horizontal filter weights */
    for(int x = 0; x < targetW; x++)
        delete[] contribX[x].p;
    delete[] contribX;

    delete[] newData;
    delete[] pel;
    delete[] pel2;
    delete[] tmp;
    delete[] weight;
    delete[] bPelDelta;

    if(yscale < 1.0)
    {
        for(int i = 0; i < (int)(fwidth / yscale * 2 + 1); i++)
        {
            delete[] tmpRowsMem[i];
        }
    }
    else
    {
        for(int i = 0; i < (int)(fwidth * 2 + 1); i++)
        {
            delete[] tmpRowsMem[i];
        }
    }

    //progress info
    emit notifyProgressDone();

    QTime stoptime = QTime::currentTime ();
    kdDebug() << "time needed for scaling: " << starttime.msecsTo ( stoptime )  << "ms" << endl;

    return;
}

int KisScaleVisitor::calcContrib(ContribList *contrib, double scale, double fwidth, int srcwidth, KisFilterStrategy* filterStrategy, Q_INT32 i)
{
        //ContribList* contribX: receiver of contrib info
        //double xscale: horizontal zooming scale
        //double fwidth: Filter sampling width
        //int dstwidth: Target bitmap width
        //int srcwidth: Source bitmap width
        //double (*filterf)(double): Filter proc
        //int i: Pixel column in source bitmap being processed

        double width;
        double fscale;
        double center, begin, end;
        double weight;
        Q_INT32 k, n;

        if(scale < 1.0)
        {
                //Shrinking image
                width = fwidth / scale;
                fscale = 1.0 / scale;

                contrib->n = 0;
                contrib->p = new Contrib[ (int)(width * 2 + 1) ];

                center = (double) i / scale;
                begin = ceil(center - width);
                end = floor(center + width);
                for(int srcpos = (int)begin; srcpos <= end; ++srcpos)
                {
                        weight = center - (double) srcpos;
                        weight = filterStrategy->valueAt(weight / fscale) / fscale;
                        if(srcpos < 0)
                                n = -srcpos;
                        else if(srcpos >= srcwidth)
                                n = (srcwidth - srcpos) + srcwidth - 1;
                        else
                                n = srcpos;

                        k = contrib->n++;
                        contrib->p[k].m_pixel = n;
                        contrib->p[k].m_weight = weight;
                }
        }
        else
        {
                // Expanding image
                contrib->n = 0;
                contrib->p = new Contrib[ (int)(fwidth * 2 + 1) ];

                center = (double) i / scale;
                begin = ceil(center - fwidth);
                end = floor(center + fwidth);

                for(int srcpos = (int)begin; srcpos <= end; ++srcpos)
                {
                        weight = center - (double) srcpos;
                        weight = filterStrategy->valueAt(weight);
                        if(srcpos < 0) {
                                n = -srcpos;
                        } else if(srcpos >= srcwidth) {
                                n = (srcwidth - srcpos) + srcwidth - 1;
                        } else {
                                n = srcpos;
                        }
                        k = contrib->n++;
                        contrib->p[k].m_pixel = n;
                        contrib->p[k].m_weight = weight;
                }
        }
        return 0;
} /* calc_x_contrib */
