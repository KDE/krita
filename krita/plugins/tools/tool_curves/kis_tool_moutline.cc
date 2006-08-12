/*
 *  kis_tool_moutline.cc -- part of Krita
 *
 *  Copyright (c) 2006 Emanuele Tamponi <emanuele@valinor.it>
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

#include <math.h>

#include <qpainter.h>
#include <qlayout.h>
#include <qrect.h>

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kdebug.h>
#include <knuminput.h>

#include "kis_global.h"
#include <kis_iterators_pixel.h>
#include "kis_colorspace.h"
#include "kis_channelinfo.h"
#include "kis_doc.h"
#include "kis_painter.h"
#include "kis_point.h"
#include "kis_canvas_subject.h"
#include "kis_canvas_controller.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_move_event.h"
#include "kis_canvas.h"
#include "kis_canvas_painter.h"
#include "kis_cursor.h"
#include "kis_tool_controller.h"
#include "kis_vec.h"
#include "kis_selection.h"
#include "kis_selection_options.h"
#include "kis_selected_transaction.h"
#include "kis_paintop_registry.h"
#include "kis_convolution_painter.h"

#include "kis_tool_moutline.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define RMS(a, b) (sqrt ((a) * (a) + (b) * (b)))
#define ROUND(x) ((int) ((x) + 0.5))

KisKernelSP createKernel( Q_INT32 i0, Q_INT32 i1, Q_INT32 i2, Q_INT32 i3, Q_INT32 i4,
                          Q_INT32 i5, Q_INT32 i6, Q_INT32 i7, Q_INT32 i8, Q_INT32 i9,
                          Q_INT32 i10, Q_INT32 i11, Q_INT32 i12, Q_INT32 i13, Q_INT32 i14,
                          Q_INT32 i15, Q_INT32 i16, Q_INT32 i17, Q_INT32 i18, Q_INT32 i19,
                          Q_INT32 i20, Q_INT32 i21, Q_INT32 i22, Q_INT32 i23, Q_INT32 i24,
                          Q_INT32 factor, Q_INT32 offset )
{
    KisKernelSP kernel = new KisKernel();
    kernel->width = 5;
    kernel->height = 5;

    kernel->factor = factor;
    kernel->offset = offset;

    kernel->data = new Q_INT32[25];
    kernel->data[0] = i0;
    kernel->data[1] = i1;
    kernel->data[2] = i2;
    kernel->data[3] = i3;
    kernel->data[4] = i4;
    kernel->data[5] = i5;
    kernel->data[6] = i6;
    kernel->data[7] = i7;
    kernel->data[8] = i8;
    kernel->data[9] = i9;
    kernel->data[10] = i10;
    kernel->data[11] = i11;
    kernel->data[12] = i12;
    kernel->data[13] = i13;
    kernel->data[14] = i14;
    kernel->data[15] = i15;
    kernel->data[16] = i16;
    kernel->data[17] = i17;
    kernel->data[18] = i18;
    kernel->data[19] = i19;
    kernel->data[20] = i20;
    kernel->data[21] = i21;
    kernel->data[22] = i22;
    kernel->data[23] = i23;
    kernel->data[24] = i24;

    return kernel;
}

KisKernelSP createKernel( Q_INT32 i0, Q_INT32 i1, Q_INT32 i2,
                          Q_INT32 i3, Q_INT32 i4, Q_INT32 i5,
                          Q_INT32 i6, Q_INT32 i7, Q_INT32 i8,
                          Q_INT32 factor, Q_INT32 offset )
{
    KisKernelSP kernel = new KisKernel();
    kernel->width = 3;
    kernel->height = 3;

    kernel->factor = factor;
    kernel->offset = offset;

    kernel->data = new Q_INT32[9];
    kernel->data[0] = i0;
    kernel->data[1] = i1;
    kernel->data[2] = i2;
    kernel->data[3] = i3;
    kernel->data[4] = i4;
    kernel->data[5] = i5;
    kernel->data[6] = i6;
    kernel->data[7] = i7;
    kernel->data[8] = i8;

    return kernel;
}

KisCurveMagnetic::KisCurveMagnetic (KisToolMagnetic *parent)
    : m_parent(parent)
{

}

KisCurveMagnetic::~KisCurveMagnetic ()
{

}

void KisCurveMagnetic::calculateCurve (KisCurve::iterator p1, KisCurve::iterator p2, KisCurve::iterator /*it*/)
{
    QPoint start = (*p1).point().roundQPoint();
    QPoint end = (*p2).point().roundQPoint();
    QRect rc = QRect(start,end).normalize();
    rc.setTopLeft(rc.topLeft()+QPoint(-10,-10));         // Enlarge the view, so problems with gaussian blur can be removed
    rc.setBottomRight(rc.bottomRight()+QPoint(10,10));   // and we are able to find paths that go beyond the rect.

    KisPaintDeviceSP src = m_parent->m_currentImage->activeDevice();
    GrayMatrix       dst = GrayMatrix(rc.width());

    detectEdges  (rc, src, dst);
    reduceMatrix (rc, dst, 3, 3, 3, 3);

    uint tlx = rc.topLeft().x();
    uint tly = rc.topLeft().y();
    start -= QPoint(tlx,tly);
    end -= QPoint(tlx,tly);
    showMatrixValues (rc, dst, start, end);
}

void KisCurveMagnetic::showMatrixValues(const QRect& rc, const GrayMatrix& dst, const QPoint& start, const QPoint& end)
{
    QString line = "---|";
    for (uint col = 0; col < dst.count() && col < 59; col++)
        line += QString("%1|").arg(col,3);
    kdDebug(0) << line << endl;
    for (uint row = 0; row < dst[0].count(); row++) {
        line = QString("%1|").arg(row,3);
        for (uint col = 0; col < dst.count(); col++) {
            if (col >= 58) {
                line += "....";
                break;
            }
            if (col == start.x() && row == start.y())
                line += "SSS:";
            else if (col == end.x() && row == end.y())
                line += "EEE:";
            else if (dst[col][row])
                line += QString("%1:").arg(dst[col][row],3);
            else
                line += "   :";
        }
        kdDebug(0) << line << endl;
    }
    kdDebug(0) << "WIDTH: " << rc.width() << " HEIGHT: " << rc.height() << endl;
    kdDebug(0) << " COLS: " << dst.count() << "   ROWS: " << dst[0].count() << endl;
}

void KisCurveMagnetic::reduceMatrix (QRect& rc, GrayMatrix& m, int top, int right, int bottom, int left)
{
    QPoint topleft(top, left);
    QPoint bottomright(bottom, right);

    rc.setTopLeft(rc.topLeft()+topleft);
    rc.setBottomRight(rc.bottomRight()-bottomright);

    if (left)
        m.erase(m.begin(),m.begin()+left);
    if (right)
        m.erase(m.end()-right,m.end());
    if (top) {
        for (uint i = 0; i < m.count(); i++)
            m[i].erase(m[i].begin(),m[i].begin()+top);
    }
    if (bottom) {
        for (uint i = 0; i < m.count(); i++)
            m[i].erase(m[i].end()-bottom,m[i].end());
    }
}

void KisCurveMagnetic::detectEdges (const QRect & rect, KisPaintDeviceSP src, GrayMatrix& dst)
{
    GrayMatrix graysrc(rect.width());
    GrayMatrix xdeltas(rect.width());
    GrayMatrix ydeltas(rect.width());
    GrayMatrix magnitude(rect.width());
    KisPaintDeviceSP smooth = new KisPaintDevice(src->colorSpace());

    gaussianBlur(rect, src, smooth);
    toGrayScale(rect, smooth, graysrc);
    getDeltas(graysrc, xdeltas, ydeltas);
    getMagnitude(xdeltas, ydeltas, magnitude);
    nonMaxSupp(magnitude, xdeltas, ydeltas, dst);
}

void KisCurveMagnetic::gaussianBlur (const QRect& rect, KisPaintDeviceSP src, KisPaintDeviceSP dst)
{
    int grectx = rect.x();
    int grecty = rect.y();
    int grectw = rect.width();
    int grecth = rect.height();
    if (dst != src) {
        KisPainter gc(dst);
        gc.bitBlt(grectx, grecty, COMPOSITE_COPY, src, grectx, grecty, grectw, grecth);
        gc.end();
    }

    KisConvolutionPainter painter( dst );
    // FIXME createKernel could create dynamic gaussian kernels having sigma as argument
    KisKernelSP kernel = createKernel( 2, 4, 5, 4, 2,
                                       4, 9,12, 9, 4,
                                       5,12,15,12, 5,
                                       4, 9,12, 9, 4,
                                       2, 4, 5, 4, 2, 115, 0 );
//     KisKernelSP kernel = createKernel( 1, 2, 1, 2, 4, 2, 1, 2, 1, 11, 0);
    painter.applyMatrix(kernel, grectx, grecty, grectw, grecth, BORDER_AVOID);
}

void KisCurveMagnetic::toGrayScale (const QRect& rect, KisPaintDeviceSP src, GrayMatrix& dst)
{
    int grectx = rect.x();
    int grecty = rect.y();
    int grectw = rect.width();
    int grecth = rect.height();
    QColor c;
    KisColorSpace *cs = src->colorSpace();

    for (int row = 0; row < grecth; row++) {
        KisHLineIteratorPixel srcIt = src->createHLineIterator(grectx, grecty+row, grectw, false);
        for (int col = 0; col < grectw; col++) {
            cs->toQColor(srcIt.rawData(),&c);
            dst[col].push_back(qGray(c.rgb()));
            ++srcIt;
        }
    }
}

void KisCurveMagnetic::getDeltas (const GrayMatrix& src, GrayMatrix& xdelta, GrayMatrix& ydelta)
{
    uint start = 1, xend = src[0].count()-1, yend = src.count()-1;
    Q_INT16 deri;
    for (uint col = 0; col < src.count(); col++) {
        for (uint row = 0; row < src[col].count(); row++) {
            if (row >= start && row < xend) {
                deri = src[col][row+1] - src[col][row-1];
                xdelta[col].push_back(deri);
            } else
                xdelta[col].push_back(0);
            if (col >= start && col < yend) {
                deri = src[col+1][row] - src[col-1][row];
                ydelta[col].push_back(deri);
            } else
                ydelta[col].push_back(0);
        }
    }
}

void KisCurveMagnetic::getMagnitude (const GrayMatrix& xdelta, const GrayMatrix& ydelta, GrayMatrix& gradient)
{
    for (uint col = 0; col < xdelta.count(); col++) {
        for (uint row = 0; row < xdelta[col].count(); row++)
            gradient[col].push_back((Q_INT16)(ROUND(RMS(xdelta[col][row],ydelta[col][row]))));
    }
}

void KisCurveMagnetic::nonMaxSupp (const GrayMatrix& magnitude, const GrayMatrix& xdelta, const GrayMatrix& ydelta, GrayMatrix& nms)
{
    // Directions:
    // 1:   0 - 22.5 degrees
    // 2:   22.5 - 67.5 degrees
    // 3:   67.5 - 90 degrees
    // Second direction is relative to a quadrant. The quadrant is known by looking at x/y derivatives
    // First quadrant:  Gx < 0  & Gy >= 0
    // Second quadrant: Gx < 0  & Gy < 0
    // Third quadrant:  Gx >= 0 & Gy < 0
    // Fourth quadrant: Gx >= 0 & Gy >= 0
    // For this reason: first direction is relative to Gy only and third direction to Gx only
    
    double  theta;      // theta = invtan (|Gy| / |Gx|) This give the direction
    Q_INT16 mag;        // Current magnitude
    Q_INT16 lmag;       // Magnitude at the left (So this pixel is "more internal" than the current
    Q_INT16 rmag;       // Magnitude at the right (So this pixel is "more external")
    double  xdel;       // Current xdelta
    double  ydel;       // Current ydelta
    Q_INT16 result;

    for (uint col = 0; col < magnitude.count(); col++) {
        for (uint row = 0; row < magnitude[col].count(); row++) {
            mag = magnitude[col][row];
            if (!mag || row == 0 || row == (magnitude[col].count()-1) ||
                        col == 0 || col == (magnitude.count()-1))
            {
                result = 0;
            } else {
                xdel = (double)xdelta[col][row];
                ydel = (double)ydelta[col][row];
                theta = atan(fabs(ydel)/fabs(xdel));
                if (theta < 0)
                    theta = fabs(theta)+M_PI_2;
                theta = (theta * 360.0) / (2.0*M_PI); // Radians -> degrees
                if (theta >= 0 && theta < 22.5) { // .0 - .3926990816
                    if (ydel >= 0) {
                        lmag = magnitude[col][row-1];
                        rmag = magnitude[col][row+1];
                    } else {
                        lmag = magnitude[col][row+1];
                        rmag = magnitude[col][row-1];
                    }
                }
                if (theta >= 22.5 && theta < 67.5) { // .3926990816 - 1.1780972449
                    if (xdel >= 0) {
                        if (ydel >= 0) {
                            lmag = magnitude[col-1][row-1];
                            rmag = magnitude[col+1][row+1];
                        } else {
                            lmag = magnitude[col+1][row-1];
                            rmag = magnitude[col-1][row+1];
                        }
                    } else {
                        if (ydel >= 0) {
                            lmag = magnitude[col-1][row+1];
                            rmag = magnitude[col+1][row-1];
                        } else {
                            lmag = magnitude[col+1][row+1];
                            rmag = magnitude[col-1][row-1];
                        }
                    }
                }
                if (theta >= 67.5 && theta <= 90.0) { // 1.1780972449 - 1.5707963266
                    if (xdel >= 0) {
                        lmag = magnitude[col+1][row];
                        rmag = magnitude[col-1][row];
                    } else {
                        lmag = magnitude[col-1][row];
                        rmag = magnitude[col+1][row];
                    }
                }

                if ((mag < lmag) || (mag < rmag)) {
                    result = 0;
                } else {
                    if (rmag == mag) // If the external magnitude is equal to the current, take the external and suppress this.
                        result = 0;
                    else
                        result = (mag > 255) ? 255 : mag;
                }
            }
            nms[col].push_back(result);
        }
    }
}

KisToolMagnetic::KisToolMagnetic ()
    : super("Magnetic Outline Tool")
{
    setName("tool_moutline");
    setCursor(KisCursor::load("tool_moutline_cursor.png", 6, 6));

    m_derived = 0;
    m_curve = 0;

    m_transactionMessage = i18n("Magnetic Outline Selection");
}

KisToolMagnetic::~KisToolMagnetic ()
{
    delete m_derived;
    m_curve = 0;
}

void KisToolMagnetic::activate ()
{
    super::activate();
    m_derived = new KisCurveMagnetic(this);
    m_curve = m_derived;
}

void KisToolMagnetic::setup(KActionCollection *collection)
{
    m_action = static_cast<KRadioAction *>(collection->action(name()));

    if (m_action == 0) {
        KShortcut shortcut(Qt::Key_Plus);
        shortcut.append(KShortcut(Qt::Key_F9));
        m_action = new KRadioAction(i18n("Magnetic Outline"),
                                    "tool_moutline",
                                    shortcut,
                                    this,
                                    SLOT(activate()),
                                    collection,
                                    name());
        Q_CHECK_PTR(m_action);

        m_action->setToolTip(i18n("Magnetic Selection: click around an edge to select the area inside."));
        m_action->setExclusiveGroup("tools");
        m_ownAction = true;
    }
}

#include "kis_tool_moutline.moc"
