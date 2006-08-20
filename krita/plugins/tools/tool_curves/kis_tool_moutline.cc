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
#include <set>

#include <qpainter.h>
#include <qlayout.h>
#include <qrect.h>
#include <qlabel.h>
#include <qpushbutton.h>

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

using namespace std;

#define RMS(a, b) (sqrt ((a) * (a) + (b) * (b)))
#define ROUND(x) ((int) ((x) + 0.5))

const int NOEDGE = 0x0000;
const int POSSIBLE_EDGE = 0x00f0;
const int EDGE = 400;

const int ORTHOGONAL_COST = 10; // 1*10
const int DIAGONAL_COST = 14;   // sqrt(2)*10
const int MALUS = 20;           // This applies to NOEDGE nodes

class Node {

    QPoint m_pos;
    int m_gCost;
    int m_hCost;
    int m_tCost;
    bool m_malus;
    QPoint m_parent;

public:

    Node()
    {
        m_pos = m_parent = QPoint(-1,-1);
        m_gCost = m_hCost = m_tCost = 0;
        m_malus = false;
    }

    Node(const Node& node)
    {
        m_pos = node.pos();
        m_gCost = node.gCost();
        m_hCost = node.hCost();
        m_tCost = node.tCost();
        m_malus = node.malus();
        m_parent = node.parent();
    }

    Node(const QPoint& parent, const QPoint& pos, int g, int h, bool malus)
        : m_pos(pos), m_hCost(h), m_malus(malus)
    {
        setGCost(g);
        m_parent = parent;
    }
    ~Node ()
    {
    }

    int gCost () const {return m_gCost;}
    int hCost () const {return m_hCost;}
    int tCost () const {return m_tCost;}
    bool malus () const {return m_malus;}
    QPoint pos () const {return m_pos;}
    int col () const {return m_pos.x();}
    int row () const {return m_pos.y();}
    QPoint parent () const {return m_parent;}

    void setGCost (int g)
    {
        m_gCost = g+(m_malus?MALUS:0);
        m_tCost = m_gCost+m_hCost;
    }
    void setHCost (int h)
    {
        m_hCost = h;
        m_tCost = m_gCost+m_hCost;
    }
    void setPos (const QPoint& pos)
    {
        m_pos = pos;
    }
    void setMalus (bool malus)
    {
        m_malus = malus;
    }
    void clear ()
    {
        m_pos = QPoint(-1,-1);
    }

    bool operator== (const Node& n2) const
    {
        return m_pos == n2.pos();
    }
    bool operator!= (const Node& n2) const
    {
        return m_pos != n2.pos();
    }
    bool operator== (const QPoint& n2) const
    {
        return m_pos == n2;
    }
    bool operator!= (const QPoint& n2) const
    {
        return m_pos != n2;
    }
    bool operator< (const Node& n2) const
    {
        return m_tCost < n2.tCost();
    }
    bool operator> (const Node& n2) const
    {
        return m_tCost > n2.tCost();
    }

    QValueList<Node> getNeighbor(const GrayMatrix& src, const Node& end)
    {
        QPoint tmpdist;
        QValueList<Node> temp;
        int dcol, drow;
        int g, h;
        bool malus;
        int x[8] = { 1, 1, 0,-1,-1,-1, 0, 1},
            y[8] = { 0,-1,-1,-1, 0, 1, 1, 1};
    
        for (int i = 0; i < 8; i++) {
            dcol = m_pos.x() + x[i];
            drow = m_pos.y() + y[i];
            tmpdist = QPoint(dcol,drow) - end.pos();
            // I use src[0] here because all cols have same number of rows
            if (dcol == (int)src.count() || dcol < 0 ||
                drow == (int)src[0].count() || drow < 0)
                continue;
            if (src[dcol][drow])
                malus = false;
            else
                malus = true;
            if (i%2)
                g = m_gCost + DIAGONAL_COST;
            else
                g = m_gCost + ORTHOGONAL_COST;
            h = ORTHOGONAL_COST * (abs(tmpdist.x()) + abs(tmpdist.y()));
            temp.append(Node(m_pos,QPoint(dcol,drow),g,h,malus));
        }
        return temp;
    }

};

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
    m_standardkeepselected = false;
}

KisCurveMagnetic::~KisCurveMagnetic ()
{

}

KisCurve::iterator KisCurveMagnetic::addPivot (KisCurve::iterator it, const KisPoint& point)
{
    return iterator(*this,m_curve.insert(it.position(), CurvePoint(point,true,false,LINEHINT)));
}

KisCurve::iterator KisCurveMagnetic::pushPivot (const KisPoint& point)
{
    iterator it;

    it = pushPoint(point,true,false,LINEHINT);
//     if (count() == 1 && !m_parent->editingMode())
//         addPoint(it,point,true,false,LINEHINT);
    
    return selectPivot(it);
}

void KisCurveMagnetic::calculateCurve (KisCurve::iterator p1, KisCurve::iterator p2, KisCurve::iterator it)
{
    if (p1 == m_curve.end() || p2 == m_curve.end()) // It happens sometimes, for example on the first click
        return;
    if (m_parent->editingMode())
        return;
    QPoint start = (*p1).point().roundQPoint();
    QPoint end = (*p2).point().roundQPoint();
    QRect rc = QRect(start,end).normalize();
    rc.setTopLeft(rc.topLeft()+QPoint(-8,-8));         // Enlarge the view, so problems with gaussian blur can be removed
    rc.setBottomRight(rc.bottomRight()+QPoint(8,8));   // and we are able to find paths that go beyond the rect.

    KisPaintDeviceSP src = m_parent->m_currentImage->activeDevice();
    GrayMatrix       dst = GrayMatrix(rc.width(),GrayCol(rc.height()));

    detectEdges  (rc, src, dst);
    reduceMatrix (rc, dst, 3, 3, 3, 3);

    Node startNode, endNode;
    multiset<Node> openSet;
    NodeMatrix openMatrix = NodeMatrix(rc.width(),NodeCol(rc.height()));
    NodeMatrix closedMatrix = NodeMatrix(rc.width(),NodeCol(rc.height()));

    QPoint tl(rc.topLeft().x(),rc.topLeft().y());
    start -= tl;  // Relative to the matrix
    end -= tl;    // Relative to the matrix

    findEdge (start.x(), start.y(), dst, startNode);
    openMatrix[startNode.col()][startNode.row()] = *openSet.insert(startNode);
    endNode.setPos(end);

    while (!openSet.empty()) {
        Node current = *openSet.begin();
        
        openSet.erase(openSet.begin());
        openMatrix[current.col()][current.row()].clear();

        QValueList<Node> successors = current.getNeighbor(dst,endNode);
        for (QValueList<Node>::iterator i = successors.begin(); i != successors.end(); i++) {
            int col = (*i).col();
            int row = (*i).row();
            if ((*i) == endNode) {
                while (current.parent() != QPoint(-1,-1)) {
                    it = addPoint(it,tl+current.pos(),false,false,LINEHINT);
                    current = closedMatrix[current.parent().x()][current.parent().y()];
                }
                return;
            }
            Node *openNode = &openMatrix[col][row];
            if (*openNode != QPoint(-1,-1)) {
                if (*i > *openNode)
                    continue;
                else {
                    openSet.erase(qFind(openSet.begin(),openSet.end(),*openNode));
                    openNode->clear();      // Clear the Node
                }
            }
            Node *closedNode = &closedMatrix[col][row];
            if (*closedNode != QPoint(-1,-1)) {
                if ((*i) > (*closedNode))
                    continue;
                else {
                    openMatrix[col][row] = *openSet.insert(*closedNode);
                    closedNode->clear();    // Clear the Node
                    continue;
                }
            }
            openMatrix[col][row] = *openSet.insert(*i);
        }
        closedMatrix[current.col()][current.row()] = current;
    }
}

void KisCurveMagnetic::findEdge (int col, int row, const GrayMatrix& src, Node& node)
{
    int x = -1;
    int y = -1;

    // tmpdist out of range
    KisVector2D mindist(5.0,5.0), tmpdist(1000.0,1000.0);
    for (int i = -5; i < 6; i++) {
        for (int j = -5; j < 6; j++) {
            if (src[col+i][row+j] != NOEDGE) {
                tmpdist = KisVector2D(i,j);
                if (tmpdist.length() < mindist.length())
                    mindist = tmpdist;
            }
        }
    }
    if (tmpdist.x() == 1000.0)
        mindist = KisVector2D(0.0,0.0);

    x = (int)(col + mindist.x());
    y = (int)(row + mindist.y());

    node.setPos(QPoint(x,y));
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
    GrayMatrix graysrc(rect.width(),GrayCol(rect.height()));
    GrayMatrix xdeltas(rect.width(),GrayCol(rect.height()));
    GrayMatrix ydeltas(rect.width(),GrayCol(rect.height()));
    GrayMatrix magnitude(rect.width(),GrayCol(rect.height()));
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
//     KisKernelSP kernel = createKernel( 2, 4, 5, 4, 2,
//                                        4, 9,12, 9, 4,
//                                        5,12,15,12, 5,
//                                        4, 9,12, 9, 4,
//                                        2, 4, 5, 4, 2, 115, 0 );
    KisKernelSP kernel = createKernel( 1, 1, 1, 1, 24, 1, 1, 1, 1, 32, 0);
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
            dst[col][row] = qGray(c.rgb());
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
                xdelta[col][row] = deri;
            } else
                xdelta[col][row] = 0;
            if (col >= start && col < yend) {
                deri = src[col+1][row] - src[col-1][row];
                ydelta[col][row] = deri;
            } else
                ydelta[col][row] = 0;
        }
    }
}

void KisCurveMagnetic::getMagnitude (const GrayMatrix& xdelta, const GrayMatrix& ydelta, GrayMatrix& gradient)
{
    for (uint col = 0; col < xdelta.count(); col++) {
        for (uint row = 0; row < xdelta[col].count(); row++)
            gradient[col][row] = (Q_INT16)(ROUND(RMS(xdelta[col][row],ydelta[col][row])));
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
                result = NOEDGE;
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
                    result = NOEDGE;
                } else {
                    if (rmag == mag) // If the external magnitude is equal to the current, suppress current.
                        result = NOEDGE;
                    else
                        result = (mag > 255) ? 255 : mag;
                }
            }
            nms[col][row] = result;
        }
    }
}

KisToolMagnetic::KisToolMagnetic ()
    : super("Magnetic Outline Tool")
{
    setName("tool_moutline");
    setCursor(KisCursor::load("tool_moutline_cursor.png", 6, 6));

    m_editingMode = false;
    m_editingCursor = m_draggingCursor = false;
    m_mode = 0;
    m_derived = 0;
    m_curve = 0;

    m_transactionMessage = i18n("Magnetic Outline Selection");
}

KisToolMagnetic::~KisToolMagnetic ()
{
    m_curve = 0;
    delete m_derived;
}

void KisToolMagnetic::update (KisCanvasSubject *subject)
{
    super::update(subject);
}

void KisToolMagnetic::activate ()
{
    super::activate();
    if (!m_derived) {
        m_derived = new KisCurveMagnetic(this);
        m_curve = m_derived;
    }
}

void KisToolMagnetic::deactivate ()
{
    m_curve->endActionOptions();
    m_actionOptions = NOOPTIONS;
    m_dragging = false;
    m_drawPivots = true;
}

void KisToolMagnetic::keyPress(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Control) {
        draw(false);
        if (m_editingMode) {
            m_editingMode = false;
            m_curve->selectPivot(m_current,false);
            m_mode->setText(i18n("Standard Mode"));
        } else {
            m_editingMode = true;
            m_mode->setText(i18n("Editing Mode"));
        }
        draw(false);
    } else if (event->key() == Qt::Key_Delete) {
        draw(false);
        m_dragging = false;
        if (m_curve->pivots().count() == 2)
            m_curve->clear();
        else {
            if ((*m_current) == m_curve->last() && !(m_editingMode)) {
                m_curve->deletePivot(m_current.previousPivot());
                m_previous = m_current.previousPivot();
            } else {
                m_editingMode = false;
                m_curve->deletePivot(m_current);
                m_editingMode = true;
            }
            draw(false);
        }
    } else
        super::keyPress(event);
}

void KisToolMagnetic::buttonRelease(KisButtonReleaseEvent *event)
{
    if (m_editingMode) {
        draw(m_current);
        m_editingMode = false;
        if (!m_curve->isEmpty())
            m_curve->movePivot(m_current, m_currentPoint);
        m_editingMode = true;
        draw(m_current);
    }
    super::buttonRelease(event);
}

void KisToolMagnetic::buttonPress(KisButtonPressEvent *event)
{
    updateOptions(event->state());
    if (!m_currentImage)
        return;
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_currentPoint = event->pos();
        PointPair temp(m_curve->end(),false);
        if (m_editingMode)
            temp = pointUnderMouse (m_subject->canvasController()->windowToView(event->pos().toQPoint()));
        if (temp.first == m_curve->end() && !(m_actionOptions)) {
            draw(m_curve->end());
            if (!m_curve->isEmpty()) {
                m_previous = m_current;
                m_current = m_curve->pushPivot(event->pos());
            } else {
                m_previous = m_current = m_curve->pushPivot(event->pos());
            }
            if (m_curve->pivots().count() > 1)
                m_curve->calculateCurve(m_previous,m_current,m_current);
            if (m_editingMode)
                draw();
            else {
                if ((*m_previous).point() == (*m_current).point())
                    draw(m_curve->end());
                else
                    draw();
            }
        } else if (temp.first != m_curve->end() && m_editingMode) {
            if (temp.second) {
                draw(true, true);
                m_current = m_curve->selectPivot(temp.first);
                draw(true, true);
            } else {
                draw(false);
                m_current = selectByMouse(temp.first);
                draw(false);
            }
            if (!(*m_current).isSelected())
                m_dragging = false;
        }
    }
}

void KisToolMagnetic::move(KisMoveEvent *event)
{
    updateOptions(event->state());
    if (m_curve->selectedPivots().isEmpty())
        return;
    if (m_currentPoint == event->pos().floorQPoint())
        return;
    if (m_editingMode) {
        PointPair temp = pointUnderMouse(m_subject->canvasController()->windowToView(event->pos().toQPoint()));
        if (temp.first == m_curve->end() && !m_dragging) {
            if (m_editingCursor || m_draggingCursor) {
                setCursor(KisCursor::load("tool_moutline_cursor.png", 6, 6));
                m_editingCursor = m_draggingCursor = false;
            }
        } else {
            if (!m_draggingCursor && temp.second) {
                setCursor(KisCursor::load("tool_moutline_dragging.png", 6, 6));
                m_editingCursor = false;
                m_draggingCursor = true;
            }
            if (!m_editingCursor && !temp.second) {
                setCursor(KisCursor::load("tool_moutline_editing.png", 6, 6));
                m_editingCursor = true;
                m_draggingCursor = false;
            }
        }
        if (!m_dragging)
            return;
    }
    KisPoint trans = event->pos() - m_currentPoint;
    KisPoint dist;
    dist = (*m_current).point() - (*m_current.previousPivot()).point();
    if (((fabs(dist.x()) + fabs(dist.y())) > 50 && !(m_editingMode)) || m_curve->pivots().count() == 1) {
        draw(m_curve->end());
        m_previous = m_current;
        m_current = m_curve->pushPivot(event->pos());
    } else if ((*m_previous).point() == (*m_current).point() && (*m_previous).point() == m_curve->last().point())
        draw(m_curve->end());
    else
        draw(m_current);
    m_curve->movePivot(m_current,event->pos());
    m_currentPoint = event->pos().floorQPoint();
    draw(m_current);
}

KisCurve::iterator KisToolMagnetic::selectByMouse(KisCurve::iterator it)
{
    KisCurve::iterator currPivot = m_curve->selectPivot(m_curve->addPivot(it, KisPoint(0,0)));
    m_curve->movePivot(currPivot,(*it).point());

    return currPivot;
}

void KisToolMagnetic::slotCommitCurve ()
{
    if (!m_curve->isEmpty())
        commitCurve();
}

QWidget* KisToolMagnetic::createOptionWidget(QWidget* parent)
{
    m_optWidget = super::createOptionWidget(parent);
    QVBoxLayout * l = dynamic_cast<QVBoxLayout*>(m_optWidget->layout());
    QHBoxLayout * hbox = new QHBoxLayout(l);
    Q_CHECK_PTR(hbox);

    hbox->setSpacing(6);

    m_mode = new QLabel(i18n("Standard mode"), m_optWidget);
    hbox->addWidget(m_mode);

    QPushButton *finish = new QPushButton(i18n("Finish selection"), m_optWidget);
    hbox->addWidget(finish);
    connect(finish, SIGNAL(clicked()), this, SLOT(slotCommitCurve()));

    return m_optWidget;
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

        m_action->setToolTip(i18n("Magnetic Selection: move around an edge to select it. Hit Ctrl to enter/quit manual editing mode, and double click to finish."));
        m_action->setExclusiveGroup("tools");
        m_ownAction = true;
    }
}

#include "kis_tool_moutline.moc"
