/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
#include <qrect.h>
#include <qwmatrix.h>
#include <qimage.h>

#include <kcommand.h>
#include <klocale.h>
#include <kdebug.h>

#include <koStore.h>

#include "kis_global.h"
#include "kis_types.h"
#include "kis_paint_device.h"
#include "kistile.h"
#include "kispixeldata.h"
#include "kis_painter.h"
#include "kis_fill_painter.h"
#include "kis_undo_adapter.h"
#include "kis_iterators_quantum.h"
#include "kis_iterators_pixel.h"

namespace {
        class KisResizeDeviceCmd : public KNamedCommand {
                typedef KNamedCommand super;

        public:
                KisResizeDeviceCmd(KisUndoAdapter *adapter, KisPaintDeviceSP dev, KisTileMgrSP before, KisTileMgrSP after) : super(i18n("Resize"))
                {
                        m_adapter = adapter;
                        m_dev = dev;
                        m_before = before;
                        m_after = after;
                }

                virtual ~KisResizeDeviceCmd()
                {
                }

        public:
                virtual void execute()
                {
                        KisImageSP owner;

                        m_adapter -> setUndo(false);
                        m_dev -> setTiles(m_after);
                        m_adapter -> setUndo(true);
                        owner = m_dev -> image();
                        owner -> notify();
                }

                virtual void unexecute()
                {
                        KisImageSP owner;

                        m_adapter -> setUndo(false);
                        m_dev -> setTiles(m_before);
                        m_adapter -> setUndo(true);
                        owner = m_dev -> image();
                        owner -> notify();
                }

        private:
                KisUndoAdapter *m_adapter;
                KisPaintDeviceSP m_dev;
                KisTileMgrSP m_before;
                KisTileMgrSP m_after;
        };
}

KisPaintDevice::KisPaintDevice(Q_INT32 width, Q_INT32 height, KisStrategyColorSpaceSP colorStrategy, const QString& name)
{
	init();
	m_x = 0;
	m_y = 0;
	m_offX = 0;
	m_offY = 0;
	m_offW = 0;
	m_offH = 0;
	m_tiles = new KisTileMgr(colorStrategy -> depth(), width, height);
	m_visible = true;
	m_owner = 0;
	m_name = name;
	m_compositeOp = COMPOSITE_OVER;
	m_colorStrategy = colorStrategy;
}

KisPaintDevice::KisPaintDevice(KisImage *img, Q_INT32 width, Q_INT32 height, KisStrategyColorSpaceSP colorStrategy, const QString& name)
{
        init();
        configure(img, width, height, colorStrategy, name, COMPOSITE_OVER);
}

KisPaintDevice::KisPaintDevice(KisTileMgrSP tm, KisImage *img, const QString& name)
{
        init();
        m_x = 0;
        m_y = 0;
        m_offX = 0;
        m_offY = 0;
        m_offW = 0;
        m_offH = 0;
        m_tiles = tm;
        m_visible = true;
        m_owner = img;
        m_name = name;
        m_compositeOp = COMPOSITE_OVER;
}

KisPaintDevice::KisPaintDevice(const KisPaintDevice& rhs) : QObject(), super(rhs)
{
        if (this != &rhs) {
                m_owner = 0;

                if (rhs.m_tiles)
                        m_tiles = new KisTileMgr(*rhs.m_tiles);

                if (rhs.m_shadow)
                        m_shadow = new KisTileMgr(*rhs.m_shadow);

                m_visible = rhs.m_visible;
                m_x = rhs.m_x;
                m_y = rhs.m_y;
                m_offX = rhs.m_offX;
                m_offY = rhs.m_offY;
                m_offW = rhs.m_offW;
                m_offH = rhs.m_offH;
                m_quantumSize = rhs.m_quantumSize;
                m_name = rhs.m_name;
                m_compositeOp = COMPOSITE_OVER;
		m_colorStrategy = rhs.m_colorStrategy;
        }
}

KisPaintDevice::~KisPaintDevice()
{
}

Q_INT32 KisPaintDevice::tileNum(Q_INT32, Q_INT32) const
{
        return 0;
}

void KisPaintDevice::configure(KisImage *image,
                               Q_INT32 width, Q_INT32 height,
                               KisStrategyColorSpaceSP colorStrategy,
                               const QString& name,
                               CompositeOp compositeOp)
{
	if (image == 0 || name.isEmpty())
		return;

	m_x = 0;
	m_y = 0;
	m_offX = 0;
	m_offY = 0;
	m_offW = 0;
	m_offH = 0;
	m_tiles = new KisTileMgr(colorStrategy -> depth(), width, height);
	m_visible = true;
	m_owner = image;
	m_name = name;
	m_compositeOp = compositeOp;
	m_colorStrategy = colorStrategy;
}

// void KisPaintDevice::update()
// {
//         update(0, 0, width(), height());
// }

// void KisPaintDevice::update(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h)
// {
//         if (x < m_offX)
//                 x = m_offX;

//         if (y < m_offY)
//                 y = m_offY;

//         if (w > m_offW)
//                 w = m_offW;

//         if (h > m_offH)
//                 h = m_offH;

// }

void KisPaintDevice::move(Q_INT32 x, Q_INT32 y)
{
        m_x = x;
        m_y = y;
        emit positionChanged(this);
}

void KisPaintDevice::move(const QPoint& pt)
{
        move(pt.x(), pt.y());
}

bool KisPaintDevice::contains(Q_INT32 x, Q_INT32 y) const
{
        QRect rc(m_x, m_y, width(), height());

        return rc.contains(x, y);
}

bool KisPaintDevice::contains(const QPoint& pt) const
{
        return contains(pt.x(), pt.y());
}

bool KisPaintDevice::shouldDrawBorder() const
{
        return false;
}

QString KisPaintDevice::name() const
{
        return m_name;
}

void KisPaintDevice::setName(const QString& name)
{
        if (!name.isEmpty())
                m_name = name;
}

void KisPaintDevice::maskBounds(Q_INT32 *, Q_INT32 *, Q_INT32 *, Q_INT32 *)
{
}

void KisPaintDevice::maskBounds(QRect *rc)
{
        Q_INT32 x1;
        Q_INT32 y1;
        Q_INT32 x2;
        Q_INT32 y2;

        maskBounds(&x1, &y1, &x2, &y2);
        rc -> setRect(x1, y1, x2 - x1, y2 - y1);
}

void KisPaintDevice::init()
{
        m_visible = false;
        m_quantumSize = 0;
        m_offX = 0;
        m_offY = 0;
        m_offW = 0;
        m_offH = 0;
        m_x = 0;
        m_y = 0;
	m_colorStrategy = 0;
}


void KisPaintDevice::setTiles(KisTileMgrSP mgr)
{
        Q_ASSERT(mgr);

        if (m_owner) {
                KisUndoAdapter *doc = m_owner -> undoAdapter();

                if (doc && doc -> undo())
                        doc -> addCommand(new KisResizeDeviceCmd(doc, this, m_tiles, mgr));
        }

        m_tiles = mgr;
}


void KisPaintDevice::resize(Q_INT32 w, Q_INT32 h)
{
        KisTileMgrSP old = tiles();
        KisTileMgrSP tm = new KisTileMgr(old, colorStrategy() -> depth(), w, h);
        Q_INT32 oldW = width();
        Q_INT32 oldH = height();
        KisFillPainter painter;

        setTiles(tm);

        painter.begin(this);

        if (oldW < w)
                painter.eraseRect(oldW, 0, w, h);

        if (oldH < h)
                painter.eraseRect(0, oldH, w, h);

        painter.end();
}

void KisPaintDevice::scale(double xscale, double yscale) 
{
        
        //define filter supports
        const double filter_support=1.0;
        const double box_support=0.5;
        const double triangle_support=1.0;
        const double bell_support=1.5;
        const double B_spline_support=2.0;
        const double Lanczos3_support=3.0;
        const double Mitchell_support=2.0;
       
        double (KisPaintDevice::*filterf)(double) = &KisPaintDevice::Mitchell_filter; //pointer to filter function
        double fwidth;
        
        // target image data
        Q_INT32 targetW;
        Q_INT32 targetH;
        
        // compute size of target image
        // (this bit seems to be mostly from QImage.xForm)
        QWMatrix scale_matrix;
	scale_matrix.scale(xscale, yscale);
        scale_matrix = QPixmap::trueMatrix( scale_matrix, width(), height() );
        if ( scale_matrix.m11() == 1.0F && scale_matrix.m22() == 1.0F ) {
                kdDebug() << "Identity matrix, do nothing.\n";
                return;
        }
        targetW = qRound( scale_matrix.m11() * width() );
        targetH = qRound( scale_matrix.m22() * height() );
        targetW = QABS( targetW );
        targetH = QABS( targetH );

        //set filter type
        enum filterType {
                BOX_FILTER,
                TRIANGLE_FILTER,
                BELL_FILTER,
                B_SPLINE_FILTER,
                FILTER,
                LANCZOS3_FILTER,
                MITCHELL_FILTER
        };
        filterType filter=MITCHELL_FILTER;
        switch(filter){
                case BOX_FILTER: filterf=&KisPaintDevice::box_filter; fwidth=box_support; break;
                case TRIANGLE_FILTER: filterf=&KisPaintDevice::triangle_filter; fwidth=triangle_support; break;
                case BELL_FILTER: filterf=&KisPaintDevice::bell_filter; fwidth=bell_support; break;
                case B_SPLINE_FILTER: filterf=&KisPaintDevice::B_spline_filter; fwidth=B_spline_support; break;
                case FILTER: filterf=&KisPaintDevice::filter; fwidth=filter_support; break;
                case LANCZOS3_FILTER: filterf=&KisPaintDevice::Lanczos3_filter; fwidth=Lanczos3_support; break;
                case MITCHELL_FILTER: filterf=&KisPaintDevice::Mitchell_filter; fwidth=Mitchell_support; break;
        }
         
        KisTileMgrSP tm = new KisTileMgr(colorStrategy() -> depth(), targetW, targetH);
        QUANTUM * newData = new QUANTUM[targetW * targetH * depth() * sizeof(QUANTUM)];
        int n;				/* pixel number */
        double center, left, right;	/* filter calculation variables */
        double m_width, fscale, weight[depth()];	/* filter calculation variables */
        QUANTUM *pel = new QUANTUM[depth() * sizeof(QUANTUM)];
        QUANTUM *pel2 = new QUANTUM[depth() * sizeof(QUANTUM)];
        bool bPelDelta[depth()];
        CLIST	*contribY;		/* array of contribution lists */
        CLIST	contribX;
        int		nRet = -1;
        const Q_INT32 BLACK_PIXEL=0;
        const Q_INT32 WHITE_PIXEL=255;
        
        
        // create intermediate column to hold horizontal dst column zoom
        QUANTUM * tmp = new QUANTUM[height() * depth() * sizeof(QUANTUM)];
        
        /* Build y weights */
        /* pre-calculate filter contributions for a column */
        contribY = (CLIST *)calloc(targetH, sizeof(CLIST));
        int k;
        if(yscale < 1.0)
        {
                m_width = fwidth / yscale;
                fscale = 1.0 / yscale;
                for(int y = 0; y < targetH; y++)
                {
                        contribY[y].n = 0;
                        contribY[y].p = (CONTRIB *)calloc((int) (m_width * 2 + 1), sizeof(CONTRIB));
                        center = (double) y / yscale;
                        left = ceil(center - m_width);
                        right = floor(center + m_width);
                        for(int xx = (int)left; xx <= right; xx++) {
                                weight[0] = center - (double) xx;
                                weight[0] = (this->*filterf)(weight[0] / fscale) / fscale;
                                if(xx < 0) {
                                        n = -xx;
                                } else if(xx >= height()) {
                                        n = (height() - xx) + height() - 1;
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
                        contribY[y].p = (CONTRIB *)calloc((int) (fwidth * 2 + 1), sizeof(CONTRIB));
                        center = (double) y / yscale;
                        left = ceil(center - fwidth);
                        right = floor(center + fwidth);
                        for(int xx = (int)left; xx <= right; xx++) {
                                weight[0] = center - (double) xx;
                                weight[0] = (this->*filterf)(weight[0]);
                                if(xx < 0) {
                                        n = -xx;
                                        } else if(xx >= height()) {
                                                n = (height() - xx) + height() - 1;
                                        } else {
                                                n = xx;
                                }
                                k = contribY[y].n++;
                                contribY[y].p[k].m_pixel = n;
                                contribY[y].p[k].m_weight = weight[0];
                        }
                }
        }


        for(int x = 0; x < targetW; x++)
        {
                calc_x_contrib(&contribX, xscale, fwidth, targetW, height(), filterf, x);
                /* Apply horz filter to make dst column in tmp. */
                for(int y = 0; y < height(); y++)
                {
                        for(int channel = 0; channel < depth(); channel++){
                                weight[channel] = 0.0;
                                bPelDelta[channel] = FALSE;
                        }
                        tiles()->readPixelData(contribX.p[0].m_pixel, y, contribX.p[0].m_pixel, y, pel, depth());
                        for(int xx = 0; xx < contribX.n; xx++)
                        {
                                if (!(contribX.p[xx].m_pixel < 0 || contribX.p[xx].m_pixel >= width())){
                                        tiles()->readPixelData(contribX.p[xx].m_pixel, y, contribX.p[xx].m_pixel, y, pel2, depth());
                                        for(int channel = 0; channel < depth(); channel++)
                                        {
                                                if(pel2[channel] != pel[channel]) bPelDelta[channel] = TRUE;
                                                weight[channel] += pel2[channel] * contribX.p[xx].m_weight;
                                        }
                                }
                        }
                        
                        for(int channel = 0; channel < depth(); channel++){
                                weight[channel] = bPelDelta[channel] ? static_cast<int>(qRound(weight[channel])) : pel[channel];
                                tmp[y*depth()+channel] = static_cast<QUANTUM>(CLAMP(weight[channel], BLACK_PIXEL, WHITE_PIXEL));
                        }
                } /* next row in temp column */
                free(contribX.p);

                /* The temp column has been built. Now stretch it 
                vertically into dst column. */
                for(int y = 0; y < targetH; y++)
                {
                        for(int channel = 0; channel < depth(); channel++){
                                weight[channel] = 0.0;
                                bPelDelta[channel] = FALSE;
                                pel[channel] = tmp[contribY[y].p[0].m_pixel*depth()+channel];
                        }
                        for(int xx = 0; xx < contribY[y].n; xx++)
                        {
                                for(int channel = 0; channel < depth(); channel++){
                                        pel2[channel] = tmp[contribY[y].p[xx].m_pixel*depth()+channel];
                                        if(pel2[channel] != pel[channel]) bPelDelta[channel] = TRUE;
                                        weight[channel] += pel2[channel] * contribY[y].p[xx].m_weight;
                                }
                        }
                        for(int channel = 0; channel < depth(); channel++){
                                weight[channel] = bPelDelta[channel] ? static_cast<int>(qRound(weight[channel])) : pel[channel];
                                int currentPos = (y*targetW+x) * depth(); // try to be at least a little efficient
                                if (weight[channel]<0) newData[currentPos + channel] = 0;
                                else if (weight[channel]>255) newData[currentPos + channel] = 255;
                                else newData[currentPos + channel] = (uchar)weight[channel];
                       }
                } /* next dst row */
        } /* next dst column */
        tm -> writePixelData(0, 0, targetW - 1, targetH - 1, newData, targetW * depth());
        setTiles(tm); // Also sets width and height correctly
        nRet = 0; /* success */

        free(tmp);

        /* free the memory allocated for vertical filter weights */
        for(int y = 0; y < targetH; y++)
                free(contribY[y].p);
        free(contribY);

        //return nRet;
        return;
}

int KisPaintDevice::calc_x_contrib(CLIST *contribX, double xscale, double fwidth, int /*dstwidth*/, int srcwidth, double (KisPaintDevice::*filterf)(double), Q_INT32 x)
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
                contribX->p = (CONTRIB *)calloc((int) (width * 2 + 1), sizeof(CONTRIB));
                center = (double) x / xscale;
                left = ceil(center - width);
                right = floor(center + width);
                for(int xx = (int)left; xx <= right; ++xx)
                {
                        weight = center - (double) xx;
                        weight = (this->*filterf)(weight / fscale) / fscale;
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
                contribX->p = (CONTRIB *)calloc((int) (fwidth * 2 + 1), sizeof(CONTRIB));
                center = (double) x / xscale;
                left = ceil(center - fwidth);
                right = floor(center + fwidth);

                for(int xx = (int)left; xx <= right; ++xx)
                {
                        weight = center - (double) xx;
                        weight = (this->*filterf)(weight);
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

/* Filter function definitions */

double KisPaintDevice::filter(double t)
{
        /* f(t) = 2|t|^3 - 3|t|^2 + 1, -1 <= t <= 1 */
        if(t < 0.0) t = -t;
        if(t < 1.0) return((2.0 * t - 3.0) * t * t + 1.0);
        return(0.0);
}

double KisPaintDevice::box_filter(double t)
{
        if((t > -0.5) && (t <= 0.5)) return(1.0);
        return(0.0);
}

double KisPaintDevice::triangle_filter(double t)
{
        if(t < 0.0) t = -t;
        if(t < 1.0) return(1.0 - t);
        return(0.0);
}

double KisPaintDevice::bell_filter(double t)
{
        if(t < 0) t = -t;
        if(t < .5) return(.75 - (t * t));
        if(t < 1.5) {
                t = (t - 1.5);
                return(.5 * (t * t));
        }
        return(0.0);
}

double KisPaintDevice::B_spline_filter(double t)
{
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

double KisPaintDevice::sinc(double x)
{
        const double pi=3.1415926535897932385;
        x *= pi;
        if(x != 0) return(sin(x) / x);
        return(1.0);
}

double KisPaintDevice::Lanczos3_filter(double t)
{
        if(t < 0) t = -t;
        if(t < 3.0) return(sinc(t) * sinc(t/3.0));
        return(0.0);
}

double KisPaintDevice::Mitchell_filter(double t)
{
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


// XXX: also allow transform on part of paint device?
void KisPaintDevice::transform(const QWMatrix & matrix)
{
        if (tiles() == 0) {
                kdDebug() << "No tilemgr.\n";
                return;
        }

        /* No, we're NOT duplicating the entire image, at the moment krita uses
           too much memory already */
        QUANTUM *origPixel = new QUANTUM[depth() * sizeof(QUANTUM)];
        // target image data
        Q_INT32 targetW;
        Q_INT32 targetH;

        // compute size of target image
        // (this bit seems to be mostly from QImage.xForm)
        QWMatrix mat = QPixmap::trueMatrix( matrix, width(), height() );
        if ( mat.m12() == 0.0F && mat.m21() == 0.0F ) {
                kdDebug() << "Scaling layer: " << m_name << "\n";
                if ( mat.m11() == 1.0F && mat.m22() == 1.0F ) {
                        kdDebug() << "Identity matrix, do nothing.\n";
                        return;
                }
                targetW = qRound( mat.m11() * width() );
                targetH = qRound( mat.m22() * height() );
                targetW = QABS( targetW );
                targetH = QABS( targetH );
        } else {
                kdDebug() << "Rotating or shearing layer " << m_name << "\n";
                QPointArray a( QRect(0, 0, width(), height()) );
                a = mat.map( a );
                QRect r = a.boundingRect().normalize();
                targetW = r.width();
                targetH = r.height();
        }

        // Create target pixel buffer which we'll read into a tile manager
        // when done.
        QUANTUM * newData = new QUANTUM[targetW * targetH * depth() * sizeof(QUANTUM)];
        /* This _has_ to be fixed; horribly layertype dependent */
        // XXX: according to man memset, this param order is wrong.
        // memset(newData, targetW * targetH * depth() * sizeof(QUANTUM), 0);
        memset(newData, 0, targetW * targetH * depth() * sizeof(QUANTUM));

        bool invertible;
        QWMatrix targetMat = mat.invert( &invertible ); // invert matrix
        if ( targetH == 0 || targetW == 0 || !invertible ) {
                kdDebug() << "Error, return null image\n";
                return;
        }

        // BSAR: I would have thought that one would take the source pixels,
        // do the computation, and write the target pixels.
        // Apparently, that's not true: one looks through the target
        // pixels, and computes what should be there from the source
        // pixels. I doubt I will ever completely understand this
        // stuff.
        // BC: I guess this makes it easier to make the transform anti-aliased,
        // as you can easily take the weighted mean of the square the destination
        // pixel has it's origin in.
	// BSAR: Ah, ok. I get it now...

        /* For each target line of target pixels, the original pixel is located (in the
           surrounding of)
           x = m11*x' + m21*y' + dx
           y = m22*y' + m12*x' + dy
        */

        for (Q_INT32 y = 0; y < targetH; y++) {
                /* at the moment, just round the original coordinates; but the unrounded
                   values should be used for AA */
                for (Q_INT32 x = 0; x < targetW; x++) {
                        Q_INT32 orX = qRound(targetMat.m11() * x + targetMat.m21() * y + targetMat.dx());
                        Q_INT32 orY = qRound(targetMat.m22() * y + targetMat.m12() * x + targetMat.dy());

                        int currentPos = (y*targetW+x) * depth(); // try to be at least a little efficient
                        if (!(orX < 0 || orY < 0 || orX >= width() || orY >= height())) {
                                tiles() -> readPixelData(orX, orY, orX, orY, origPixel, depth());
                                for(int i = 0; i < depth(); i++)
                                        newData[currentPos + i] = origPixel[i];
                        }
                }
        }

        KisTileMgrSP tm = new KisTileMgr(colorStrategy() -> depth(), targetW, targetH);
        tm -> writePixelData(0, 0, targetW - 1, targetH - 1, newData, targetW * depth());

        setTiles(tm); // Also sets width and height correctly

        delete[] origPixel;
        delete[] newData;

}

void KisPaintDevice::mirrorX()
{
        /* For each line, swap the liness at equal distances from the X axis*/
        /* Should be bit depth independent, but I don't have anything to test that with.
           I don't know about colour strategy, but if bit depth works that should too */

        QUANTUM *line1 = new QUANTUM[width() * depth() * sizeof(QUANTUM)];
        QUANTUM *line2 = new QUANTUM[width() * depth() * sizeof(QUANTUM)];
        KisTileMgrSP tm = new KisTileMgr(colorStrategy() -> depth(), width(), height());

        int cutoff = static_cast<int>(height()/2);

        for(int i = 0; i < cutoff; i++) {
                tiles() -> readPixelData(0, i, width() - 1, i, line1, width() * depth());
                tiles() -> readPixelData(0, height() - i - 1, width() - 1, height() - i - 1, line2, width() * depth());
                tm -> writePixelData(0, height() - i - 1, width() - 1, height() - i - 1, line1, width() * depth());
                tm -> writePixelData(0, i, width() - 1, i, line2, width() * depth());
        }

        setTiles(tm);

        delete[] line1;
        delete[] line2;
}

void KisPaintDevice::mirrorY()
{
        /* For each line, swap the pixels at equal distances from the Y axis */
        /* Note: I get the idea that this could be done faster with direct access to
           the pixel data. Now I have to copy the pixels twice only to get them
           and put them back in place. */
        /* Should be bit depth and arch independent, but I don't have anything to test
           that with I don't know about colour strategy, but if bit depth works that
           should too */
        QUANTUM *pixel = new QUANTUM[depth() * sizeof(QUANTUM)]; // the right pixel
        QUANTUM *line = new QUANTUM[width() * depth() * sizeof(QUANTUM)];
        KisTileMgrSP tm = new KisTileMgr(colorStrategy() -> depth(), width(), height());
        int cutoff = static_cast<int>(width()/2);

        for(int i = 0; i < height(); i++) {
                tiles() -> readPixelData(0, i, width() - 1, i, line, width() * depth());
                for(int j = 0; j < cutoff; j++) {
                        for(int k = 0; k < depth(); k++) {
                                pixel[k] = line[(width()-1)*depth() - j*depth() + k];
                                line[(width()-1)*depth() - j*depth() + k] = line[j*depth()+k];
                                line[j*depth()+k] = pixel[k];
                        }
                }
                tm -> writePixelData(0, i, width() - 1, i, line, width() * depth());
        }

        setTiles(tm); // Act like this is a resize; this should get it's own undo 'name'

        delete[] line;
        delete[] pixel;
}

void KisPaintDevice::expand(Q_INT32 w, Q_INT32 h)
{
        w = QMAX(w, width());
        h = QMAX(h, height());
        resize(w, h);
}

void KisPaintDevice::expand(const QSize& size)
{
        expand(size.width(), size.height());
}

void KisPaintDevice::anchor()
{
}

void KisPaintDevice::offsetBy(Q_INT32 x, Q_INT32 y)
{
        if (x < 0)
                x = 0;

        if (y < 0)
                y = 0;

        KisTileMgrSP old = tiles();
        KisTileMgrSP tm = new KisTileMgr(colorStrategy() -> depth(), x + old -> width(), y + old -> height());
        KisPixelDataSP dst;
        KisPixelDataSP src;

        setTiles(tm);
        src = old -> pixelData(0, 0, old -> width() - 1, old -> height() - 1, TILEMODE_READ);
        Q_ASSERT(src);
        dst = tm -> pixelData(x, y, x + old -> width() - 1, y + old -> height() - 1, TILEMODE_WRITE);
        Q_ASSERT(dst);
        memcpy(dst -> data, src -> data, sizeof(QUANTUM) * src -> width * src -> height * src -> depth);
        tm -> releasePixelData(dst);
        m_x -= x;
        m_y -= y;
}

bool KisPaintDevice::write(KoStore *store)
{
        KisTileMgrSP tm = tiles();
        Q_INT32 totalBytes = height() * width() * depth() * sizeof(QUANTUM);
        Q_INT32 nbytes = 0;

        Q_ASSERT(store);
        Q_ASSERT(tm);

        for (Q_INT32 y = 0; y < height(); y += TILE_HEIGHT) {
                for (Q_INT32 x = 0; x < width(); x += TILE_WIDTH) {
                        KisTileSP tile = tm -> tile(x, y, TILEMODE_READ);
                        Q_INT32 tileBytes = tile -> height() * tile -> width() * depth() * sizeof(QUANTUM);

                        tile -> lock();

                        if (store -> write(reinterpret_cast<char*>(tile -> data()), tileBytes) != tileBytes) {
                                tile -> release();
                                return false;
                        }

                        tile -> release();
                        nbytes += tileBytes;
                        emit ioProgress(nbytes * 100 / totalBytes);
                }
        }

        return true;
}

bool KisPaintDevice::read(KoStore *store)
{
        KisTileMgrSP tm = tiles();
        Q_INT32 totalBytes = height() * width() * depth() * sizeof(QUANTUM);
        Q_INT32 nbytes = 0;

        Q_ASSERT(store);
        Q_ASSERT(tm);

        for (Q_INT32 y = 0; y < height(); y += TILE_HEIGHT) {
                for (Q_INT32 x = 0; x < width(); x += TILE_WIDTH) {
                        KisTileSP tile = tm -> tile(x, y, TILEMODE_WRITE);
                        Q_INT32 tileBytes = tile -> height() * tile -> width() * depth() * sizeof(QUANTUM);

                        tile -> lock();

                        if (store -> read(reinterpret_cast<char*>(tile -> data()), tileBytes) != tileBytes) {
                                tile -> release();
                                return false;
                        }

                        tile -> release();
                        nbytes += tileBytes;
                        emit ioProgress(nbytes * 100 / totalBytes);
                }
        }

        return true;
}

void KisPaintDevice::convertTo(KisStrategyColorSpaceSP dstCS)
{
	KisPaintDevice dst(width(), height(), dstCS, "");
	KisStrategyColorSpaceSP srcCS = colorStrategy();
	if(srcCS == dstCS)
	{
		return;
	}
	KisIteratorLinePixel dstLIt = dst.iteratorPixelBegin(0);
	KisIteratorLinePixel endLIt = dst.iteratorPixelEnd(0);
	KisIteratorLinePixel srcLIt = this->iteratorPixelBegin(0);
	while( dstLIt <= endLIt)
	{
		KisIteratorPixel dstIt = dstLIt.begin();
		KisIteratorPixel endIt = endLIt.begin();
		KisIteratorPixel srcIt = srcLIt.begin();
		while(dstIt <= endIt )
		{
			KisPixelRepresentation srcPr = srcIt;
			KisPixelRepresentation dstPr = dstIt;
			srcCS->convertTo(srcPr, dstPr, dstCS);
			++dstIt; ++srcIt;
		}
		++dstLIt; ++srcLIt;
	}
	setTiles(dst.tiles());
}



void KisPaintDevice::convertFromImage(const QImage& img)
{
	KoColor c;
	QRgb rgb;
	Q_INT32 opacity;

	if (img.isNull())
		return;

	for (Q_INT32 y = 0; y < height(); y++) {
		for (Q_INT32 x = 0; x < width(); x++) {
			rgb = img.pixel(x, y);
			c.setRGB(upscale(qRed(rgb)), upscale(qGreen(rgb)), upscale(qBlue(rgb)));

			if (img.hasAlphaBuffer())
				opacity = qAlpha(rgb);
			else
				opacity = OPACITY_OPAQUE;

			setPixel(x, y, c, opacity);
		}
	}
}

QImage KisPaintDevice::convertToImage(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h)
{
	if (w < 0) {
		w = width();
	}

	if (h < 0) {
		h = height();
	}

	if (x < 0) {
		x = 0;
	}
	else
	if (x > width() - 1) {
		x = width() - 1;
	}

	if (y < 0) {
		y = 0;
	}
	else
	if (y > height() - 1) {
		y = height() - 1;
	}

	Q_INT32 x1 = x;
	Q_INT32 y1 = y;

	// These coordinates are inclusive.
	Q_INT32 x2 = x1 + w - 1;
	Q_INT32 y2 = y1 + h - 1;

	if (x2 > width() - 1) {
		x2 = width() - 1;
	}

	if (y2 > height() - 1) {
		y2 = height() - 1;
	}

	QImage image;

	if (x2 - x1 + 1 > 0 && y2 - y1 + 1 > 0 && tiles()) {
		KisPixelDataSP pd = new KisPixelData;

		pd -> mgr = 0;
		pd -> tile = 0;
		pd -> mode = TILEMODE_READ;
		pd -> x1 = x1;
		pd -> x2 = x2;
		pd -> y1 = y1;
		pd -> y2 = y2;
		pd -> width = pd -> x2 - pd -> x1 + 1;
		pd -> height = pd -> y2 - pd -> y1 + 1;
		pd -> depth = depth();
		pd -> stride = pd -> depth * pd -> width;

		// XXX: The previous code used a statically allocated buffer
		// of size RENDER_WIDTH * RENDER_HEIGHT * depth. We could do
		// this too if profiling shows this is too slow...
		pd -> owner = true;
		pd -> data = new QUANTUM[pd -> depth * pd -> width * pd -> height];
		tiles() -> readPixelData(pd);

		image = colorStrategy() -> convertToImage(pd -> data, pd -> width, pd -> height, pd -> stride);
	}

	return image;
}

KisIteratorLineQuantum KisPaintDevice::iteratorQuantumBegin(KisTileCommand* command)
{
        return KisIteratorLineQuantum( this, command, 0);
}

KisIteratorLineQuantum KisPaintDevice::iteratorQuantumBegin(KisTileCommand* command, Q_INT32 xstart, Q_INT32 xend, Q_INT32 ystart)
{
        return KisIteratorLineQuantum( this, command, ystart, xstart, xend);
}

KisIteratorLineQuantum KisPaintDevice::iteratorQuantumEnd(KisTileCommand* command)
{
        return KisIteratorLineQuantum( this, command, height() - 1);
}

KisIteratorLineQuantum KisPaintDevice::iteratorQuantumEnd(KisTileCommand* command, Q_INT32 xstart, Q_INT32 xend, Q_INT32 yend)
{
        return KisIteratorLineQuantum( this, command, yend, xstart, xend);
}

KisIteratorLineQuantum KisPaintDevice::iteratorQuantumSelectionBegin(KisTileCommand* command)
{
        return KisIteratorLineQuantum( this, command, 0);
}

KisIteratorLineQuantum KisPaintDevice::iteratorQuantumSelectionBegin(KisTileCommand* command, Q_INT32 xstart, Q_INT32 xend, Q_INT32 ystart)
{
        return KisIteratorLineQuantum( this, command, ystart,  xstart, xend);
}

KisIteratorLineQuantum KisPaintDevice::iteratorQuantumSelectionEnd(KisTileCommand* command)
{
        return KisIteratorLineQuantum( this, command, height() - 1);
}

KisIteratorLineQuantum KisPaintDevice::iteratorQuantumSelectionEnd(KisTileCommand* command, Q_INT32 xstart, Q_INT32 xend, Q_INT32 yend)
{
        return KisIteratorLineQuantum( this, command, yend, xstart, xend);
}

KisIteratorLinePixel KisPaintDevice::iteratorPixelBegin(KisTileCommand* command)
{
        return KisIteratorLinePixel( this, command, 0);
}

KisIteratorLinePixel KisPaintDevice::iteratorPixelBegin(KisTileCommand* command, Q_INT32 xstart, Q_INT32 xend, Q_INT32 ystart)
{
        return KisIteratorLinePixel( this, command, ystart, xstart, xend);
}

KisIteratorLinePixel KisPaintDevice::iteratorPixelEnd(KisTileCommand* command)
{
        return KisIteratorLinePixel( this, command, height() - 1);
}

KisIteratorLinePixel KisPaintDevice::iteratorPixelEnd(KisTileCommand* command, Q_INT32 xstart, Q_INT32 xend, Q_INT32 yend)
{
        return KisIteratorLinePixel( this, command, yend, xstart, xend);
}

KisIteratorLinePixel KisPaintDevice::iteratorPixelSelectionBegin(KisTileCommand* command)
{
        return KisIteratorLinePixel( this, command, 0);
}

KisIteratorLinePixel KisPaintDevice::iteratorPixelSelectionBegin(KisTileCommand* command, Q_INT32 xstart, Q_INT32 xend, Q_INT32 ystart)
{
        return KisIteratorLinePixel( this, command, ystart,  xstart, xend);
}

KisIteratorLinePixel KisPaintDevice::iteratorPixelSelectionEnd(KisTileCommand* command)
{
        return KisIteratorLinePixel( this, command, height() - 1);
}

KisIteratorLinePixel KisPaintDevice::iteratorPixelSelectionEnd(KisTileCommand* command, Q_INT32 xstart, Q_INT32 xend, Q_INT32 yend)
{
        return KisIteratorLinePixel( this, command, yend, xstart, xend);
}


#include "kis_paint_device.moc"
