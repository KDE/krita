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

#include "kis_tool_moutline.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define RMS(a, b) (sqrt ((a) * (a) + (b) * (b)))
#define ROUND(x) ((int) ((x) + 0.5))

KisCurveMagnetic::KisCurveMagnetic (KisToolMagnetic *parent)
    : m_parent(parent)
{

}

KisCurveMagnetic::~KisCurveMagnetic ()
{

}

void KisCurveMagnetic::calculateCurve (KisCurve::iterator p1, KisCurve::iterator p2, KisCurve::iterator it)
{
    KisPaintDeviceSP src = m_parent->m_currentImage->activeDevice();
    QPoint start = (*p1).point().roundQPoint();
    QPoint end = (*p2).point().roundQPoint();
    QRect rc = QRect(start,end).normalize();

    rc.setTopLeft(rc.topLeft()+QPoint(-3,-3));         // Enlarge the view
    rc.setBottomRight(rc.bottomRight()+QPoint(3,3));   // Enlarge the view

    uint tlx = rc.topLeft().x();
    uint tly = rc.topLeft().y();

    Matrix edges = sobel (rc, src);
/*    
    KisVector2D base, first, dist1, dist2;
    first = dist1 = dist2 = KisVector2D(0,0);

    x = (start.x()-tlx);
    y = (start.y()-tly); // We need x,y values for the edges matrix
    base = KisVector2D(x,y);

    kdDebug(0) << "X: " << x << " Y: " << y << endl;
    
    kdDebug(0) << "Search the first edge at the left of start" << endl;
    for (i = x; i > (x-10); i--) {
        if (edges[y][i]) {
            first = KisVector2D(i,y);
            break;
        }
    }
    if (first != KisVector2D(0,0))
        dist1 = base - first;
    else
        dist1 = KisVector2D(1000,0);
    kdDebug(0) << "FIRST EDGE AFTER LEFT CHECK: " << first.x() << " " << first.y() << endl;
    kdDebug(0) << "Search the first edge at the right of start" << endl;
    for (i = x; i < (x+10); i++) {
        if (edges[y][i]) {
            dist2 = KisVector2D(i,y) - base;
            if (dist2.length() < dist1.length()) {
                first = KisVector2D(i,y);
                dist1 = dist2;
            }
            break;
        }
    }
    kdDebug(0) << "FIRST EDGE AFTER RIGHT CHEC: " << first.x() << " " << first.y() << endl;
    kdDebug(0) << "Search the first edge at the bottom of start" << endl;
    for (i = y; i < (y+10); i++) {
        if (edges[i][x]) {
            dist2 = KisVector2D(x,i) - base;
            if (dist2.length() < dist1.length()) {
                first = KisVector2D(x,i);
                dist1 = dist2;
            }
            break;
        }
    }
    kdDebug(0) << "FIRST EDGE AFTER BOTTOM CHE: " << first.x() << " " << first.y() << endl;
    kdDebug(0) << "Search the first edge at the top of start" << endl;
    for (i = y; i > (y-10); i--) {
        if (edges[i][x]) {
            dist2 = KisVector2D(x,i) - base;
            if (dist2.length() < dist1.length()) {
                first = KisVector2D(x,i);
                dist1 = dist2;
            }
            break;
        }
    }
    kdDebug(0) << "FIRST EDGE AFTER TOP CHECK : " << first.x() << " " << first.y() << endl;
*/
    uint /*i, j, */x, y;
    x = (start.x()-tlx);
    y = (start.y()-tly);
    KisVector2D first = findNearestGradient(KisVector2D(x,y), edges);
    first = KisVector2D(first.x()+tlx,first.y()+tly);
    GMap gmap = convertMatrixToGMap(edges);
/*
    gmap.prepend(start);
    gmap.prepend(KisVector2D(first.x()+tlx,first.y()+tly));
    gmap.append(end);
*/
    addPoint(it,first.toKisPoint(),false,false,LINEHINT);
    first = findNextGradient(gmap,start,end);
    int iterate = 99;
    while (first != KisVector2D(-1,-1) && iterate) {
        addPoint(it,first.toKisPoint(),false,false,LINEHINT);
        first = findNextGradient(gmap,first,end);
        --iterate;

        kdDebug(0) << "POINT: " << first.x() << " " << first.y() << endl;
    }
    /*
    QString line;
    Q_INT32 grad = edges.last()[0];
    for (i = 0; i < edges.count(); i++) {
        line = "";
        for (j = 0; j < edges[i].count(); j++) {
            if (j == first.x() && i == first.y())
                line += " 4 |";
            else if (j == x && i == y)
                line += " 2 |";
            else if (edges[i][j] && (edges[i][j] > (grad-10)) && (edges[i][j] < (grad+10)))
                line += QString::number((int)(100+edges[i][j]))+"|";
            else
                line += " 0 |";
        }
        kdDebug(0) << line << endl;
    }
    kdDebug(0) << "ROWS: " << edges.count() << " COLS: " << edges[0].count() << endl;
    kdDebug(0) << "HEIG: " << rc.height() << " WIDT: " << rc.width() << endl;
    */
}

KisVector2D KisCurveMagnetic::findNearestGradient (const KisVector2D& from, const Matrix& m)
{
    double mindist, tmpdist;
    KisVector2D vec(-1,-1), tmpvec;
    mindist = 10000; // Starting from a too big distance
    for (uint i = 0; i < (m.count()-1); i++) {   // Ignore last row
        for (uint j = 0; j < m[i].count(); j++) {
            if (j == from.x() && i == from.y())
                continue;
            if (m[i][j]) {
                tmpvec = KisVector2D(j,i);
                tmpdist = (tmpvec-from).length();
                if (tmpdist < mindist) {
                    mindist = tmpdist;
                    vec = tmpvec;
                }
            }
        }
    }
    return vec;
}

KisVector2D KisCurveMagnetic::findNearestGradient (const KisVector2D& from, const GMap& m)
{
    double mindist, tmpdist;
    KisVector2D vec(-1,-1);
    mindist = 10000;
    for (uint i = 0; i < m.count(); i++) {
        if (m[i] == from)
            continue;
        tmpdist = (m[i]-from).length();
        if (tmpdist < mindist) {
            mindist = tmpdist;
            vec = m[i];
        }
    }
    return vec;
}

GMap KisCurveMagnetic::convertMatrixToGMap (const Matrix& m)
{
    GMap gmap;
    Q_INT32 grad = m.last()[0];
    Q_INT32 tlx = m.last()[1];
    Q_INT32 tly = m.last()[2];
    for (uint i = 0; i < (m.count()-1); i++) {
        for (uint j = 0; j < m[j].count(); j++) {
            if ((m[i][j] != 0)) {
                //// THE ALGORITHM TO FIND USEFUL PIXELS GOES HERE ////
                if ((m[i][j] > (grad-10)) && (m[i][j] < (grad+10)))
                    gmap.append(KisVector2D(tlx+j,tly+i));
            }
        }
    }
    return gmap;
}

double tolerance (double dist)
{
    double tol = dist/3;
    if (tol < 1)  tol = 1;

    return tol;
}

KisVector2D KisCurveMagnetic::findNextGradient (const GMap& m, const KisVector2D& start, const KisVector2D& end)
{
    double mindist = 10000.0, tmpdist;
    double dist = (start-end).length();
    KisVector2D vec(-1,-1);
    for (uint i = 0; i < m.count(); i++) {
        if (m[i] == start || m[i] == end)
            continue;
        tmpdist = (start-m[i]).length();
        tmpdist += (m[i]-end).length();
        if (tmpdist < (dist+tolerance(dist)) && tmpdist < mindist) {
            mindist = tmpdist;
            vec = m[i];
        }
    }
    return vec;
}

Matrix KisCurveMagnetic::sobel (const QRect & rect, KisPaintDeviceSP src)
{
//     KisColorSpace *cs = src->colorSpace();
    Q_UINT32 pixelSize = src->pixelSize();
    int rectx = rect.topLeft().x();
    int recty = rect.topLeft().y();
    KisHLineIteratorPixel prevIt = src->createHLineIterator(rectx-1, recty-1, rect.width()+2, false);
    KisHLineIteratorPixel currIt = src->createHLineIterator(rectx-1, recty,   rect.width()+2, false);
    KisHLineIteratorPixel nextIt = src->createHLineIterator(rectx-1, recty+1, rect.width()+2, false);

    Q_INT32 ykernel[9] = { 1, 2, 1,
                           0, 0, 0,
                          -1,-2,-1 };
    
    Q_INT32 xkernel[9] = {-1, 0, 1,
                          -2, 0, 2,
                          -1, 0, 1 };

    Q_UINT8 **colors = new Q_UINT8*[9];

    for (int t=0;t<9;t++)
        colors[t] = new Q_UINT8[pixelSize];

    Q_INT32 *ygrad = new Q_INT32[pixelSize];
    Q_INT32 *xgrad = new Q_INT32[pixelSize];
    Q_INT32 *grad  = new Q_INT32[pixelSize];

    Q_INT32 edgegrad = 0;

    int row = 0, irow = 0;

    Matrix temp;
    temp.append(MatrixRow()); // Add first row
    while( row < rect.height() ) {
        if (currIt.isDone()) { // Reached the end of the row
            ++row; irow = 0; // Increment the row counter and reset the iteration counter for this row
            // New iterators. Should we use another way? (Rect iterators seems to make confusion with index)
            prevIt = src->createHLineIterator(rectx-1, recty+row-1, rect.width()+2, false);
            currIt = src->createHLineIterator(rectx-1, recty+row,   rect.width()+2, false);
            nextIt = src->createHLineIterator(rectx-1, recty+row+1, rect.width()+2, false);
            temp.append(MatrixRow()); // Add row
        }
        colors[irow] = prevIt.rawData();
        colors[irow+3] = currIt.rawData();
        colors[irow+6] = nextIt.rawData();
        if (irow == 2) { // Third iteration in this row, colors array is full.
            for (uint i = 0; i < (pixelSize-1); i++) {
                ygrad[i] = xgrad[i] = 0; // Reset the values.
                for (uint j = 0; j < 9; j++) {
                    if (ykernel[j] != 0)
                        ygrad[i] = ygrad[i]+colors[j][i]*ykernel[j];
                    if (xkernel[j] != 0)
                        xgrad[i] = xgrad[i]+colors[j][i]*xkernel[j];
                }
                grad[i] = (Q_INT32)(ROUND(RMS(ygrad[i],xgrad[i]))/5.66); // Taken from sobelfilter
            }
            if (grad[0] != 0 && grad[0] > edgegrad)
                edgegrad = grad[0];
            temp[row].append(grad[0]);
            // Swap pixels
            colors[0] = colors[1]; colors[1] = colors[2]; colors[2] = 0;
            colors[3] = colors[4]; colors[4] = colors[5]; colors[5] = 0;
            colors[6] = colors[7]; colors[7] = colors[8]; colors[8] = 0;
        } else
            ++irow;
        ++prevIt;
        ++currIt;
        ++nextIt;
    }

    delete [] colors;
    delete [] ygrad;
    delete [] xgrad;
    delete [] grad;

    temp.last().append(edgegrad); // Last row is empty, so I use it to store some information
    temp.last().append(rectx);
    temp.last().append(recty);
    return temp;
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
