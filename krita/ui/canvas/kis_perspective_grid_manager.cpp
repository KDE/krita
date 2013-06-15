/*
 * This file is part of Krita
 *
 *  Copyright (c) 2006,2008 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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

#include "canvas/kis_perspective_grid_manager.h"


#include <kaction.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>

#include <QTransform>

#include "canvas/kis_canvas2.h"
#include "kis_config.h"
#include "kis_image.h"
#include "kis_perspective_grid.h"
#include "canvas/kis_grid_painter_configuration.h"
#include "kis_view2.h"
#include "kis_canvas_resource_provider.h"
#include "kis_coordinates_converter.h"

/***************************************************************/
/*                 Auxiliary line structures                   */
/***************************************************************/

class KisPerspectiveGridManager::LineWrapper {
public:
    LineWrapper(const QPointF &p0, const QPointF &p1) {
        init(toKisVector2D(p0), toKisVector2D(p1));
    }

    LineWrapper(const KisVector2D &p0, const KisVector2D &p1) {
        init(p0, p1);
    }

    QPointF intersects(const LineWrapper &other)/* const */{
        KisVector2D n0 = m_lineEquation.normal();
        KisVector2D n1 = other.m_lineEquation.normal();

        // Ensure vectors have the same direction
        if((n0(0) > 0) != (n1(0) > 0)) {
            n1 = -n1;
        }

        if(qFuzzyCompare(n0(0), n1(0)) &&
           qFuzzyCompare(n0(1), n1(1))) {

            const KisVector2D nearPoint(0,0);
            const KisVector2D farPoint(1e10,1e10);

            KisVector2D otherPt = other.m_lineEquation.projection(nearPoint);
            KisVector2D otherPtProj = m_lineEquation.projection(otherPt);

            KisVector2D newOffset = otherPt + 0.5 * (otherPtProj - otherPt);
            LineEquation tempLine(n0, newOffset);
            // Just throw it somewhere towards infinity...
            return toQPointF(tempLine.projection(farPoint));
        }

        return toQPointF(m_lineEquation.intersection(other.m_lineEquation));
    }

    qreal distance(const QPointF &pt) const {
        return m_lineEquation.absDistance(toKisVector2D(pt));
    }

    QPointF p0() const {
        return m_p0;
    }

    QPointF p1() const {
        return m_p1;
    }

    bool contains(const QPointF &pt) const {
        bool coincide = pt == m_p0 || pt == m_p1;
        bool verticalSide = (pt.x() > m_p0.x()) == (pt.x() > m_p1.x());
        bool horizontalSide = (pt.y() > m_p0.y()) ==  (pt.y() > m_p1.y());

        return coincide || !(verticalSide && horizontalSide);
    }

    bool isNull(qreal precision) {
        qreal dx = m_p1.x() - m_p0.x();
        qreal dy = m_p1.y() - m_p0.y();
        return dx * dx + dy * dy <= precision * precision;
    }

private:
    void init(const KisVector2D &p0, const KisVector2D &p1) {
        m_lineEquation =
            LineEquation::Through(p0, p1);

        m_p0 = toQPointF(p0);
        m_p1 = toQPointF(p1);
    }

private:
    LineEquation m_lineEquation;
    QPointF m_p0;
    QPointF m_p1;
};

struct KisPerspectiveGridManager::SubdivisionLinesInfo {
    QPointF startPoint;
    QPointF shift;

    QPointF intersection;
    const LineWrapper *clipLine;

    int numSubdivisions;
};


/***************************************************************/
/*                 KisPerspectiveGridManager                   */
/***************************************************************/

KisPerspectiveGridManager::KisPerspectiveGridManager(KisView2 * parent)
        : KisCanvasDecoration("perspectiveGrid", i18n("Perspective grid"), parent)
        , m_view(parent)
{
}


KisPerspectiveGridManager::~KisPerspectiveGridManager()
{
}

void KisPerspectiveGridManager::updateGUI()
{
    KisImageWSP image = m_view->image();

    if (image) {
        KisPerspectiveGrid* pGrid = image->perspectiveGrid();
        m_toggleGrid->setEnabled(pGrid->hasSubGrids());
    }
}

void KisPerspectiveGridManager::setup(KActionCollection * collection)
{
    m_toggleGrid  = new KToggleAction(i18n("Show Perspective Grid"), this);
    collection->addAction("view_toggle_perspective_grid", m_toggleGrid);
    connect(m_toggleGrid, SIGNAL(triggered()), this, SLOT(toggleVisibility()));

    m_toggleGrid->setCheckedState(KGuiItem(i18n("Hide Perspective Grid")));
    m_toggleGrid->setChecked(false);
    m_gridClear  = new KAction(i18n("Clear Perspective Grid"), this);
    collection->addAction("view_clear_perspective_grid", m_gridClear);
    connect(m_gridClear, SIGNAL(triggered()), this, SLOT(clearPerspectiveGrid()));
}

void KisPerspectiveGridManager::clearPerspectiveGrid()
{
    KisImageWSP image = m_view->image();
    if (image) {
        image->perspectiveGrid()->clearSubGrids();
        m_view->canvas()->update();
        m_toggleGrid->setChecked(false);
        m_toggleGrid->setEnabled(false);
    }
}

void KisPerspectiveGridManager::startEdition()
{
    m_toggleGrid->setEnabled(false);
}

void KisPerspectiveGridManager::stopEdition()
{
    if (m_view->resourceProvider()->currentImage()->perspectiveGrid()->hasSubGrids()) {
        m_toggleGrid->setEnabled(true);
        m_toggleGrid->setChecked(true);
    } else {
        m_toggleGrid->setChecked(false);
    }
}

KisPerspectiveGridManager::SubdivisionLinesInfo
KisPerspectiveGridManager::getSubdivisionsInfo(const LineWrapper &l0,
                                               const LineWrapper &l1,
                                               const QPointF &focusPoint,
                                               int numSubdivisions)
{
    const LineWrapper *nearest;
    const LineWrapper *farthest;

    if(l0.distance(focusPoint) < l1.distance(focusPoint)) {
        nearest = &l0;
        farthest = &l1;
    } else {
        nearest = &l1;
        farthest = &l0;
    }

    SubdivisionLinesInfo info;
    info.startPoint = farthest->p0();
    info.shift = (farthest->p1() - farthest->p0()) / numSubdivisions;
    info.intersection = focusPoint;
    info.clipLine = nearest;
    info.numSubdivisions = numSubdivisions;

    return info;
}

void KisPerspectiveGridManager::drawSubdivisions(QPainter& gc, const SubdivisionLinesInfo &info)
{
    for(int i = info.numSubdivisions - 1; i > 0; i--) {
        QPointF start = info.startPoint + i*info.shift;
        QPointF end =
            LineWrapper(start, info.intersection).intersects(*info.clipLine);
        gc.drawLine(start, end);
    }
}

#define SMALLEST_LINE 1e-10

void KisPerspectiveGridManager::drawDecoration(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter *converter, KisCanvas2 *canvas)
{
    Q_UNUSED(updateRect);
    Q_UNUSED(canvas);

    KisImageWSP image = m_view->resourceProvider()->currentImage();
    Q_ASSERT(image);

    KisPerspectiveGrid* pGrid = image->perspectiveGrid();

    QPen mainPen = KisGridPainterConfiguration::mainPen();
    QPen subdivisionPen = KisGridPainterConfiguration::subdivisionPen();
    QPen errorPen = mainPen;
    errorPen.setColor(Qt::red);


    QTransform transform = converter->imageToWidgetTransform();
    gc.save();
    gc.setTransform(transform);

    for (QList<KisSubPerspectiveGrid*>::const_iterator it = pGrid->begin(); it != pGrid->end(); ++it) {
        const KisSubPerspectiveGrid* grid = *it;

        /**
         * Note that the notion of top-bottom-right-left
         * is purely theorical
         */
        LineWrapper lineTop(*grid->topLeft(), *grid->topRight());
        LineWrapper lineRight(*grid->topRight(), *grid->bottomRight());
        LineWrapper lineBottom(*grid->bottomRight(), *grid->bottomLeft());
        LineWrapper lineLeft(*grid->bottomLeft(), *grid->topLeft());

        QPointF horizIntersection;
        QPointF vertIntersection;

        bool linesNotNull = true;
        bool polygonIsConvex = true;

        if(lineTop.isNull(SMALLEST_LINE) ||
           lineBottom.isNull(SMALLEST_LINE) ||
           lineLeft.isNull(SMALLEST_LINE) ||
           lineRight.isNull(SMALLEST_LINE)) {

            linesNotNull = false;
        }

        if(linesNotNull) {
            horizIntersection = lineTop.intersects(lineBottom);
            vertIntersection = lineLeft.intersects(lineRight);

            if(lineTop.contains(horizIntersection) ||
               lineBottom.contains(horizIntersection) ||
               lineLeft.contains(vertIntersection) ||
               lineRight.contains(vertIntersection)) {

                polygonIsConvex = false;
            }
        }

        if(polygonIsConvex && linesNotNull) {
            gc.setPen(subdivisionPen);

            SubdivisionLinesInfo info;
            info = getSubdivisionsInfo(lineTop, lineBottom, vertIntersection,
                                       grid->subdivisions());
             drawSubdivisions(gc, info);

            info = getSubdivisionsInfo(lineLeft, lineRight, horizIntersection,
                                       grid->subdivisions());
            drawSubdivisions(gc, info);
        }

        gc.setPen(polygonIsConvex && linesNotNull ? mainPen : errorPen);
        gc.drawLine(*grid->topLeft(), *grid->topRight());
        gc.drawLine(*grid->topRight(), *grid->bottomRight());
        gc.drawLine(*grid->bottomRight(), *grid->bottomLeft());
        gc.drawLine(*grid->bottomLeft(), *grid->topLeft());
    }
    gc.restore();
}

#include "kis_perspective_grid_manager.moc"
