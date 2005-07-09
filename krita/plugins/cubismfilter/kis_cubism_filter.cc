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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <stdlib.h>
#include <time.h>
#include <vector>

#include <qpoint.h>
#include <qspinbox.h>

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
#include <kis_view.h>
#include <kis_progress_display_interface.h>
#include <kis_vec.h>

#include "kis_multi_integer_filter_widget.h"
#include "kis_cubism_filter.h"
#include "kis_polygon.h"

#define RANDOMNESS       5
#define SUPERSAMPLE      4
#define CLAMP(x,l,u) ((x)<(l)?(l):((x)>(u)?(u):(x)))
#define SQR(x) ((x) * (x))

KisCubismFilter::KisCubismFilter(KisView * view) : KisFilter(id(), view)
{
}

void KisCubismFilter::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* configuration, const QRect& rect)
{
        Q_INT32 x = rect.x(), y = rect.y();
        Q_INT32 width = rect.width();
        Q_INT32 height = rect.height();
        
        //read the filter configuration values from the KisFilterConfiguration object
        Q_UINT32 tileSize = ((KisCubismFilterConfiguration*)configuration)->tileSize();
        Q_UINT32 tileSaturation = ((KisCubismFilterConfiguration*)configuration)->tileSaturation();
        
        //pixelize(src, dst, x, y, width, height, pixelWidth, pixelHeight);
}

void KisCubismFilter::randomize_indices (Q_INT32 count, Q_INT32* indices)
{
        Q_INT32 index1, index2;
        Q_INT32 tmp;
        
        //initialize random number generator with time
        srand(static_cast<unsigned int>(time(0)));
        
        for (Q_INT32 i = 0; i < count * RANDOMNESS; i++)
        {
                index1 = randomIntRange(0, count);
                index2 = randomIntRange(0, count);
                tmp = indices[index1];
                indices[index1] = indices[index2];
                indices[index2] = tmp;
        }
}

Q_INT32 KisCubismFilter::randomIntRange(Q_INT32 lowestNumber, Q_INT32 highestNumber)
{
        if(lowestNumber > highestNumber)
        {
                Q_INT32 temp = lowestNumber;
                lowestNumber = highestNumber;
                highestNumber = temp;
        }

        Q_INT32 range = highestNumber - lowestNumber + 1;
        return lowestNumber + static_cast<Q_INT32>(range *rand()/(RAND_MAX+1.0));
}

double KisCubismFilter::calcAlphaBlend (double* vec, double  oneOverDist, double  x, double  y)
{       
        double r;
        
        if ( oneOverDist==0 )
                return 1.0;
        else
        {
                r = (vec[0] * x + vec[1] * y) * oneOverDist;
                if (r < 0.2)
                        r = 0.2;
                else if (r > 1.0)
                        r = 1.0;
        }
        return r;
}

void KisCubismFilter::convertSegment (Q_INT32 x1, Q_INT32 y1, Q_INT32 x2, Q_INT32  y2, Q_INT32 offset, Q_INT32* min, Q_INT32* max)
{
        if (y1 > y2)
        {
                Q_INT32 tmp = y2; y2 = y1; y1 = tmp;
                tmp = x2; x2 = x1; x1 = tmp;
        }
        Q_INT32 ydiff = (y2 - y1);
        
        if (ydiff)
        {
                double xinc = static_cast<double>(x2 - x1) / static_cast<double>(ydiff);
                double xstart = x1 + 0.5 * xinc;
                for (Q_INT32 y = y1 ; y < y2; y++)
                {
                        if (xstart < min[y - offset])
                        {
                                min[y-offset] = xstart;
                        }
                        if (xstart > max[y - offset])
                        {
                                max[y-offset] = xstart;
                        }
                        xstart += xinc;
                }
        }
}

void KisCubismFilter::fillPolyColor (KisPaintDeviceSP src, KisPaintDeviceSP dst, KisPolygon* poly, Q_UINT8* col, Q_UINT8* dest)
{
        Q_INT32         *maxScanlines, *maxScanlinesIter;
        Q_INT32         *minScanlines, *minScanlinesIter;
        Q_INT32          val;
        Q_INT32          alpha;
        Q_UINT8        buf[4];
        Q_INT32          i, j, x, y;
        double       xx, yy;
        double       vec[2];
        Q_INT32          x1, y1, x2, y2;
        Q_INT32          selWidth, selHeight;
        Q_INT32         *vals, *valsIter, *valsEnd;
        Q_INT32          b;
        Q_INT32 xs, ys, xe, ye;

        Q_INT32 sx = 0;//poly->pts[0].x;
        Q_INT32 sy = 0;//poly->pts[0].y;
        Q_INT32 ex = 0;//poly->pts[1].x;
        Q_INT32 ey = 0;//poly->pts[1].y;

        double dist = sqrt (SQR (ex - sx) + SQR (ey - sy));
        double oneOverDist;
        if (dist > 0.0)
        {
                double oneOverDist = 1/dist;
                vec[0] = (ex - sx) * oneOverDist;
                vec[1] = (ey - sy) * oneOverDist;
        }
        else
        {
                oneOverDist = 0.0;
        }

        Q_INT32 pixelSize = src -> pixelSize();

        double dMinX, dMinY, dMaxX, dMaxY;
        //poly-> extents (dMinX, dMinY, dMaxX, dMaxY);
        Q_INT32 minX = static_cast<Q_INT32>(dMinX);
        Q_INT32 minY = static_cast<Q_INT32>(dMinY);
        Q_INT32 maxX = static_cast<Q_INT32>(dMaxX);
        Q_INT32 maxY = static_cast<Q_INT32>(dMaxY);

        Q_INT32 sizeX = (maxX - minX) * SUPERSAMPLE;
        Q_INT32 sizeY = (maxY - minY) * SUPERSAMPLE;

        minScanlines = minScanlinesIter = new Q_INT32[sizeX];
        maxScanlines = maxScanlinesIter = new Q_INT32[sizeY];
        for (Q_INT32 i = 0; i < sizeY; i++)
        {
                minScanlines[i] = maxX * SUPERSAMPLE;
                maxScanlines[i] = minX * SUPERSAMPLE;
        }
        if (0)//poly->npts)
        {
                Q_INT32 polyNpts = 0;//poly->npts;
                KisVector2D *curptr;

                //xs = static_cast<Q_INT32>(poly->pts[poly_npts-1].x);
                //ys = static_cast<Q_INT32>(poly->pts[poly_npts-1].y);
                //xe = static_cast<Q_INT32>(poly->pts[0].x);
                //ye = static_cast<Q_INT32>(poly->pts[0].y);

                xs *= SUPERSAMPLE;
                ys *= SUPERSAMPLE;
                xe *= SUPERSAMPLE;
                ye *= SUPERSAMPLE;

                convertSegment (xs, ys, xe, ye, minY * SUPERSAMPLE, minScanlines, maxScanlines);

                for (i = 1, curptr = 0/*&poly->pts[0]*/; i < polyNpts; i++)
                {
                        xs = static_cast<Q_INT32>(curptr->x());
                        ys = static_cast<Q_INT32>(curptr->y());
                        curptr++;
                        xe = static_cast<Q_INT32>(curptr->x());
                        ye = static_cast<Q_INT32>(curptr->y());
                
                        xs *= SUPERSAMPLE;
                        ys *= SUPERSAMPLE;
                        xe *= SUPERSAMPLE;
                        ye *= SUPERSAMPLE;
                
                        convertSegment (xs, ys, xe, ye, minY * SUPERSAMPLE, minScanlines, maxScanlines);
                }
        }

        vals = new Q_INT32[sizeX];

        for (Q_INT32 i = 0; i < sizeY; i++, minScanlinesIter++, maxScanlinesIter++)
        {
                if (! (i % SUPERSAMPLE))
                {
                        memset (vals, 0, sizeof (Q_INT32) * sizeX);
                }
        
                yy = static_cast<double>(i) / static_cast<double>(SUPERSAMPLE) + minY;
                
                for (Q_INT32 j = *minScanlinesIter; j < *maxScanlinesIter; j++)
                {
                        x = j - minX * SUPERSAMPLE;
                        vals[x] += 255;
                }
        
                if (! ((i + 1) % SUPERSAMPLE))
                {
                        y = (i / SUPERSAMPLE) + minY;
        
                        if (y >= y1 && y < y2)
                        {
                                for (Q_INT32 j = 0; j < sizeX; j += SUPERSAMPLE)
                                {
                                        x = (j / SUPERSAMPLE) + minX;
        
                                        if (x >= x1 && x < x2)
                                        {
                                                for (val = 0, valsIter = &vals[j], valsEnd = &valsIter[SUPERSAMPLE]; valsIter < valsEnd; valsIter++)
                                                {
                                                        val += *valsIter;
                                                }
                                                val /= SQR(SUPERSAMPLE);
        
                                                if (val > 0)
                                                {
                                                        xx = static_cast<double>(j) / static_cast<double>(SUPERSAMPLE) + minX;
                                                        alpha = static_cast<Q_INT32>(val * calcAlphaBlend (vec, oneOverDist, xx - sx, yy - sy));
                                                        //gimp_pixel_rgn_get_pixel (&src_rgn, buf, x, y);
        #ifndef USE_READABLE_BUT_SLOW_CODE
                                                        Q_UINT8 *bufIter = buf,
                                                        *colIter = col,
                                                        *bufEnd = buf+pixelSize;
        
                                                        for(; bufIter < bufEnd; bufIter++, colIter++)
                                                        *bufIter = (static_cast<Q_UINT8>(*colIter * alpha)
                                                                        + (static_cast<Q_UINT8>(*bufIter)
                                                                        * (256 - alpha))) >> 8;
        #else // original, pre-ECL code 
                                                        for (b = 0; b < pixelSize; b++)
                                                        buf[b] = ((col[b] * alpha) + (buf[b] * (255 - alpha))) / 255;
        #endif
                                
                                                        //gimp_pixel_rgn_set_pixel (&src_rgn, buf, x, y);
                                
                                                }
                                        }
                                }
                        }
                }       
        }

  delete[] vals;
  delete[] minScanlines;
  delete[] maxScanlines;
}

QWidget* KisCubismFilter::createConfigurationWidget(QWidget* parent)
{
	vKisIntegerWidgetParam param;
	param.push_back( KisIntegerWidgetParam( 2, 40, 10, i18n("Tile size") ) );
	param.push_back( KisIntegerWidgetParam( 2, 40, 10, i18n("Tile saturation") ) );
	return new KisMultiIntegerFilterWidget(this, parent, id().id().ascii(), id().id().ascii(), param );
}

KisFilterConfiguration* KisCubismFilter::configuration(QWidget* nwidget)
{
	KisMultiIntegerFilterWidget* widget = (KisMultiIntegerFilterWidget*) nwidget;
	if( widget == 0 )
	{
		return new KisCubismFilterConfiguration( 10, 10);
	} else {
		return new KisCubismFilterConfiguration( widget->valueAt( 0 ), widget->valueAt( 1 ) );
	}
}
