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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <qwmatrix.h>

#include <kdebug.h>
#include <klocale.h>

#include "kis_paint_device.h"
#include "kis_scale_visitor.h"
#include "kis_progress_display_interface.h"

double KisSimpleScaleFilterStrategy::valueAt(double t) const {
        /* f(t) = 2|t|^3 - 3|t|^2 + 1, -1 <= t <= 1 */
        if(t < 0.0) t = -t;
        if(t < 1.0) return((2.0 * t - 3.0) * t * t + 1.0);
        return(0.0);
}

double KisBoxScaleFilterStrategy::valueAt(double t) const {
        if((t > -0.5) && (t <= 0.5)) return(1.0);
        return(0.0);
}

double KisTriangleScaleFilterStrategy::valueAt(double t) const {
        if(t < 0.0) t = -t;
        if(t < 1.0) return(1.0 - t);
        return(0.0);
}

double KisBellScaleFilterStrategy::valueAt(double t) const {
        if(t < 0) t = -t;
        if(t < .5) return(.75 - (t * t));
        if(t < 1.5) {
                t = (t - 1.5);
                return(.5 * (t * t));
        }
        return(0.0);
}

double KisBSplineScaleFilterStrategy::valueAt(double t) const {
        double tt;

        if(t < 0) t = -t;
        if(t < 1) {
                tt = t * t;
                return((.5 * tt * t) - tt + (2.0 / 3.0));
        } else if(t < 2) {
                t = 2 - t;
                return((1.0 / 6.0) * (t * t * t));
        }
        return(0.0);
}

double KisLanczos3ScaleFilterStrategy::valueAt(double t) const {
        if(t < 0) t = -t;
        if(t < 3.0) return(sinc(t) * sinc(t/3.0));
        return(0.0);
}

double KisLanczos3ScaleFilterStrategy::sinc(double x) const {
        const double pi=3.1415926535897932385;
        x *= pi;
        if(x != 0) return(sin(x) / x);
        return(1.0);
}

double KisMitchellScaleFilterStrategy::valueAt(double t) const {
        const double B=1.0/3.0;
        const double C=1.0/3.0;
        double tt;

        tt = t * t;
        if(t < 0) t = -t;
        if(t < 1.0) {
                t = (((12.0 - 9.0 * B - 6.0 * C) * (t * tt)) + ((-18.0 + 12.0 * B + 6.0 * C) * tt) + (6.0 - 2 * B));
                return(t / 6.0);
        } else if(t < 2.0) {
                t = (((-1.0 * B - 6.0 * C) * (t * tt)) + ((6.0 * B + 30.0 * C) * tt) + ((-12.0 * B - 48.0 * C) * t) + (8.0 * B + 24 * C));
                return(t / 6.0);
                }
        return(0.0);
}


void KisScaleVisitor::scale(double xscale, double yscale, KisProgressDisplayInterface *m_progress, enumFilterType filterType)
{
        double fwidth = 0;

        KisScaleFilterStrategy *filterStrategy = 0;

        switch(filterType){
                case BOX_FILTER:
                        filterStrategy = new KisBoxScaleFilterStrategy();
                        break;
                case TRIANGLE_FILTER:
                        filterStrategy = new KisTriangleScaleFilterStrategy();
                        break;
                case BELL_FILTER:
                        filterStrategy = new KisBellScaleFilterStrategy();
                        break;
                case B_SPLINE_FILTER:
                        filterStrategy = new KisBSplineScaleFilterStrategy();
                        break;
                case FILTER:
                        filterStrategy = new KisSimpleScaleFilterStrategy();
                        break;
                case LANCZOS3_FILTER:
                        filterStrategy = new KisLanczos3ScaleFilterStrategy();
                        break;
                case MITCHELL_FILTER:
                        filterStrategy = new KisMitchellScaleFilterStrategy();
                        break;
        }

        fwidth = filterStrategy->support();

        // target image data
        Q_INT32 targetW;
        Q_INT32 targetH;

        Q_INT32 width = m_dev->image()->width();
        Q_INT32 height = m_dev->image()->height();

        // compute size of target image
        // (this bit seems to be mostly from QImage.xForm)
        QWMatrix scale_matrix;
	scale_matrix.scale(xscale, yscale);
	scale_matrix = QPixmap::trueMatrix( scale_matrix, width, height );
        if ( scale_matrix.m11() == 1.0F && scale_matrix.m22() == 1.0F ) {
//                 kdDebug() << "Identity matrix, do nothing.\n";
                return;
        }
        targetW = qRound( scale_matrix.m11() * width );
        targetH = qRound( scale_matrix.m22() * height );
        targetW = QABS( targetW );
        targetH = QABS( targetH );

        QUANTUM * newData = new QUANTUM[targetW * targetH * m_dev -> pixelSize() * sizeof(QUANTUM)];
	Q_CHECK_PTR(newData);

        int n;				/* pixel number */
        double center, left, right;	/* filter calculation variables */
        double m_width, fscale, weight[m_dev -> pixelSize()];	/* filter calculation variables */

        QUANTUM *pel = new QUANTUM[m_dev -> pixelSize() * sizeof(QUANTUM)];
	Q_CHECK_PTR(pel);

        QUANTUM *pel2 = new QUANTUM[m_dev -> pixelSize() * sizeof(QUANTUM)];
	Q_CHECK_PTR(pel2);

        bool bPelDelta[m_dev -> pixelSize()];
        CLIST	*contribY;		/* array of contribution lists */
        CLIST	contribX;
        int		nRet = -1;
        const Q_INT32 BLACK_PIXEL=0;
        const Q_INT32 WHITE_PIXEL=255;


        // create intermediate column to hold horizontal dst column zoom
        QUANTUM * tmp = new QUANTUM[height * m_dev -> pixelSize() * sizeof(QUANTUM)];
	Q_CHECK_PTR(tmp);

        //progress info
        m_cancelRequested = false;
        m_progress -> setSubject(this, true, true);

        /* Build y weights */
        /* pre-calculate filter contributions for a column */
        contribY = new CLIST[ targetH ];
        int k;

        if(yscale < 1.0)
        {
                m_width = fwidth / yscale;
                fscale = 1.0 / yscale;
                for(int y = 0; y < targetH; y++)
                {
                        contribY[y].n = 0;
                        contribY[y].p = new CONTRIB[ (int)(m_width * 2 + 1) ];
                        
                        center = (double) y / yscale;
                        left = ceil(center - m_width);
                        right = floor(center + m_width);
                        for(int xx = (int)left; xx <= right; xx++) {
                                weight[0] = center - (double) xx;
                                weight[0] = filterStrategy->valueAt(weight[0] / fscale) / fscale;
                                if(xx < 0) {
                                        n = -xx;
                                } else if(xx >= height) {
                                        n = (height - xx) + height - 1;
                                } else {
                                        n = xx;
                                }
                                k = contribY[y].n++;
                                contribY[y].p[k].m_pixel = n;
                                contribY[y].p[k].m_weight = weight[0];
                        }
                }
        } else {
                for(int y = 0; y < targetH; y++) {
                        contribY[y].n = 0;
                        contribY[y].p = new CONTRIB[ (int)(fwidth * 2 + 1) ];
                        
                        center = (double) y / yscale;
                        left = ceil(center - fwidth);
                        right = floor(center + fwidth);
                        for(int xx = (int)left; xx <= right; xx++) {
                                weight[0] = center - (double) xx;
                                weight[0] = filterStrategy->valueAt(weight[0]);
                                if(xx < 0) {
                                        n = -xx;
                                } else if(xx >= height) {
                                        n = (height - xx) + height - 1;
                                } else {
                                        n = xx;
                                }
                                k = contribY[y].n++;
                                contribY[y].p[k].m_pixel = n;
                                contribY[y].p[k].m_weight = weight[0];
                        }
                }
        }

        //progress info
        emit notifyProgressStage(this,i18n("Scaling layer..."),0);

        for(int x = 0; x < targetW; x++)
        {
                //progress info
                emit notifyProgress(this,(x * 100) / targetW);
                if (m_cancelRequested) {
                        break;
                }

                calc_x_contrib(&contribX, xscale, fwidth, targetW, width, filterStrategy, x);
                /* Apply horz filter to make dst column in tmp. */
                for(int y = 0; y < height; y++)
                {
                        for(int channel = 0; channel < m_dev -> pixelSize(); channel++){
                                weight[channel] = 0.0;
                                bPelDelta[channel] = FALSE;
                        }
                        m_dev -> readBytes(pel, contribX.p[0].m_pixel, y, 1, 1);
                        for(int xx = 0; xx < contribX.n; xx++)
                        {
                                if (!(contribX.p[xx].m_pixel < 0 || contribX.p[xx].m_pixel >= width)){
                                        m_dev -> readBytes(pel2, contribX.p[xx].m_pixel, y, 1, 1);
                                        for(int channel = 0; channel < m_dev -> pixelSize(); channel++)
                                        {
                                                if(pel2[channel] != pel[channel]) bPelDelta[channel] = TRUE;
                                                weight[channel] += pel2[channel] * contribX.p[xx].m_weight;
                                        }
                                }
                        }

                        for(int channel = 0; channel < m_dev -> pixelSize(); channel++){
                                weight[channel] = bPelDelta[channel] ? static_cast<int>(qRound(weight[channel])) : pel[channel];
                                tmp[y*m_dev -> pixelSize()+channel] = static_cast<QUANTUM>(CLAMP(weight[channel], BLACK_PIXEL, WHITE_PIXEL));
                        }
                } /* next row in temp column */
                delete[] contribX.p;

                /* The temp column has been built. Now stretch it
                vertically into dst column. */
                for(int y = 0; y < targetH; y++)
                {
                        for(int channel = 0; channel < m_dev -> pixelSize(); channel++){
                                weight[channel] = 0.0;
                                bPelDelta[channel] = FALSE;
                                pel[channel] = tmp[contribY[y].p[0].m_pixel*m_dev -> pixelSize()+channel];
                        }
                        for(int xx = 0; xx < contribY[y].n; xx++)
                        {
                                for(int channel = 0; channel < m_dev -> pixelSize(); channel++){
                                        pel2[channel] = tmp[contribY[y].p[xx].m_pixel*m_dev -> pixelSize()+channel];
                                        if(pel2[channel] != pel[channel]) bPelDelta[channel] = TRUE;
                                        weight[channel] += pel2[channel] * contribY[y].p[xx].m_weight;
                                }
                        }
                        for(int channel = 0; channel < m_dev -> pixelSize(); channel++){
                                weight[channel] = bPelDelta[channel] ? static_cast<int>(qRound(weight[channel])) : pel[channel];
                                int currentPos = (y*targetW+x) * m_dev -> pixelSize(); // try to be at least a little efficient
                                if (weight[channel]<0) newData[currentPos + channel] = 0;
                                else if (weight[channel]>255) newData[currentPos + channel] = 255;
                                else newData[currentPos + channel] = (uchar)weight[channel];
                       }
                } /* next dst row */
        } /* next dst column */
        if(!m_cancelRequested){
                m_dev -> writeBytes( newData, 0, 0, targetW, targetH);
                m_dev -> crop(0, 0, targetW, targetH);
                nRet = 0; /* success */
        }

        /* free the memory allocated for vertical filter weights */
        for(int y = 0; y < targetH; y++)
                delete[] contribY[y].p;
        delete[] contribY;

        delete filterStrategy;
        delete[] newData;
        delete[] pel;
        delete[] pel2;
        delete[] tmp;

        //progress info
        emit notifyProgressDone(this);

        //return nRet;
        return;
}

int KisScaleVisitor::calc_x_contrib(CLIST *contribX, double xscale, double fwidth, int /*dstwidth*/, int srcwidth, KisScaleFilterStrategy* filterStrategy, Q_INT32 x)
{
        //CLIST* contribX: receiver of contrib info
        //double xscale: horizontal zooming scale
        //double fwidth: Filter sampling width
        //int dstwidth: Target bitmap width
        //int srcwidth: Source bitmap width
        //double (*filterf)(double): Filter proc
        //int i: Pixel column in source bitmap being processed

        double width;
        double fscale;
        double center, left, right;
        double weight;
        Q_INT32 k, n;

        if(xscale < 1.0)
        {
                /* Shrinking image */
                width = fwidth / xscale;
                fscale = 1.0 / xscale;

                contribX->n = 0;
                contribX->p = new CONTRIB[ (int)(width * 2 + 1) ];

                center = (double) x / xscale;
                left = ceil(center - width);
                right = floor(center + width);
                for(int xx = (int)left; xx <= right; ++xx)
                {
                        weight = center - (double) xx;
                        weight = filterStrategy->valueAt(weight / fscale) / fscale;
                        if(xx < 0)
                                n = -xx;
                        else if(xx >= srcwidth)
                                n = (srcwidth - xx) + srcwidth - 1;
                        else
                                n = xx;

                        k = contribX->n++;
                        contribX->p[k].m_pixel = n;
                        contribX->p[k].m_weight = weight;
                }
        }
        else
        {
                /* Expanding image */
                contribX->n = 0;
                contribX->p = new CONTRIB[ (int)(fwidth * 2 + 1) ];
                
                center = (double) x / xscale;
                left = ceil(center - fwidth);
                right = floor(center + fwidth);

                for(int xx = (int)left; xx <= right; ++xx)
                {
                        weight = center - (double) xx;
                        weight = filterStrategy->valueAt(weight);
                        if(xx < 0) {
                                n = -xx;
                        } else if(xx >= srcwidth) {
                                n = (srcwidth - xx) + srcwidth - 1;
                        } else {
                                n = xx;
                        }
                        k = contribX->n++;
                        contribX->p[k].m_pixel = n;
                        contribX->p[k].m_weight = weight;
                }
        }
        return 0;
} /* calc_x_contrib */
