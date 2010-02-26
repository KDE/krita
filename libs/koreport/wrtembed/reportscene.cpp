/*
 * OpenRPT report writer and rendering engine
 * Copyright (C) 2001-2007 by OpenMFG, LLC (info@openmfg.com)
 * Copyright (C) 2007-2008 by Adam Pigg (adam@piggz.co.uk)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */


//
// ReportScene method implementations
//

#include "reportscene.h"
#include <reportpageoptions.h>
#include "reportrectentity.h"
#include "KoReportDesigner.h"

#include <labelsizeinfo.h>
#include <QPainter>
#include <KoDpi.h>
#include <kdebug.h>
#include <KoPageFormat.h>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>


ReportScene::ReportScene(qreal w, qreal h, KoReportDesigner *rd)
        : QGraphicsScene(0, 0, w, h)
{
    m_rd = rd;

    if (KoUnit::unitName(m_unit) != KoUnit::unitName(m_rd->pageUnit())) {
        m_unit = m_rd->pageUnit();
        if (KoUnit::unitName(m_unit) == "cc" || KoUnit::unitName(m_unit) == "pi" || KoUnit::unitName(m_unit) == "mm") {
            m_majorX = POINT_TO_INCH(m_unit.fromUserValue(10)) * KoDpi::dpiX();
            m_majorY = POINT_TO_INCH(m_unit.fromUserValue(10)) * KoDpi::dpiY();
        } else if (KoUnit::unitName(m_unit) == "pt") {
            m_majorX = POINT_TO_INCH(m_unit.fromUserValue(100)) * KoDpi::dpiX();
            m_majorY = POINT_TO_INCH(m_unit.fromUserValue(100)) * KoDpi::dpiY();
        } else {
            m_majorX = POINT_TO_INCH(m_unit.fromUserValue(1)) * KoDpi::dpiX();
            m_majorY = POINT_TO_INCH(m_unit.fromUserValue(1)) * KoDpi::dpiY();
        }
    }
}
ReportScene::~ReportScene()
{
    // Qt should be handling everything for us
}

void ReportScene::drawBackground(QPainter* painter, const QRectF & clip)
{
    //Draw the default background colour
    QGraphicsScene::drawBackground(painter, clip);
    painter->setRenderHint(QPainter::Antialiasing, false);

    if (m_rd->propertySet()->property("grid-visible").value().toBool()) {
        if (KoUnit::unitName(m_unit) != KoUnit::unitName(m_rd->pageUnit())) {
            m_unit = m_rd->pageUnit();
            if (KoUnit::unitName(m_unit) == "cc" || KoUnit::unitName(m_unit) == "pi" || KoUnit::unitName(m_unit) == "mm") {
                m_majorX = POINT_TO_INCH(m_unit.fromUserValue(10)) * KoDpi::dpiX();
                m_majorY = POINT_TO_INCH(m_unit.fromUserValue(10)) * KoDpi::dpiY();
            } else if (KoUnit::unitName(m_unit) == "pt") {
                m_majorX = POINT_TO_INCH(m_unit.fromUserValue(100)) * KoDpi::dpiX();
                m_majorY = POINT_TO_INCH(m_unit.fromUserValue(100)) * KoDpi::dpiY();
            } else {
                m_majorX = POINT_TO_INCH(m_unit.fromUserValue(1)) * KoDpi::dpiX();
                m_majorY = POINT_TO_INCH(m_unit.fromUserValue(1)) * KoDpi::dpiY();
            }

        }
        m_minorSteps = m_rd->propertySet()->property("grid-divisions").value().toInt();
        m_pixelIncrementX = (m_majorX / m_minorSteps);
        m_pixelIncrementY = (m_majorY / m_minorSteps);

        QPen pen = painter->pen();
        painter->setPen(QColor(212, 212, 212));

        //kDebug() << "dpix" << KoDpi::dpiX() << "dpiy" << KoDpi::dpiY() << "mayorx:" << majorx << "majory" << majory << "pix:" << pixel_incrementx << "piy:" << pixel_incrementy;

        QVector<QLine> lines;
        QVector<QPoint> points;

        if (m_pixelIncrementX > 2) { // do not bother painting points if increments are so small
            int wpoints = qRound(sceneRect().width() / m_pixelIncrementX);
            int hpoints = qRound(sceneRect().height() / m_pixelIncrementY);
            for (int i = 0; i < wpoints; ++i) {
                for (int j = 0; j < hpoints; ++j) {
                    //if (clip.contains(i * pixel_incrementx, j * pixel_incrementy)){
                    if (i % m_minorSteps == 0 && j % m_minorSteps == 0) {
                        lines << QLine(QPoint(i * m_pixelIncrementX, j * m_pixelIncrementY), QPoint(i * m_pixelIncrementX, j * m_pixelIncrementY  + m_majorX));
                        //painter->drawLine();
                        lines << QLine(QPoint(i * m_pixelIncrementX, j * m_pixelIncrementY), QPoint(i * m_pixelIncrementX + m_majorY, j * m_pixelIncrementY));
                        //painter->drawLine();
                    } else {
                        points << QPoint(i * m_pixelIncrementX, j * m_pixelIncrementY);
                        //painter->drawPoint();
                    }
                    //}
                }
            }
            painter->drawPoints(points);
            painter->drawLines(lines);

        }

        pen.setWidth(1);
        //update ( clip );
    }
}

void ReportScene::mousePressEvent(QGraphicsSceneMouseEvent * e)
{
    // clear the selection if Shift Key has not been pressed and it's a left mouse button   and
    // if the right mouse button has been pressed over an item which is not part of selected items
    if (((e->modifiers() & Qt::ShiftModifier) == 0 && e->button() == Qt::LeftButton)   ||
            (!(selectedItems().contains(itemAt(e->scenePos()))) && e->button() == Qt::RightButton))
        clearSelection();

    //This will be caught by the section to display its properties, if an item is under the cursor then they will display their properties
    emit(clicked());

    QGraphicsScene::mousePressEvent(e);
}

void ReportScene::contextMenuEvent(QGraphicsSceneContextMenuEvent * e)
{
    m_rd->sectionContextMenuEvent(this, e);
}

QPointF ReportScene::gridPoint(const QPointF& p)
{
    if (!m_rd->propertySet()->property("grid-snap").value().toBool()) {
        return p;
    }

    if (KoUnit::unitName(m_unit) != KoUnit::unitName(m_rd->pageUnit())) {
        m_unit = m_rd->pageUnit();
        if (KoUnit::unitName(m_unit) != KoUnit::unitName(m_rd->pageUnit())) {
            m_unit = m_rd->pageUnit();
            if (KoUnit::unitName(m_unit) == "cc" || KoUnit::unitName(m_unit) == "pi" || KoUnit::unitName(m_unit) == "mm") {
                m_majorX = POINT_TO_INCH(m_unit.fromUserValue(10)) * KoDpi::dpiX();
                m_majorY = POINT_TO_INCH(m_unit.fromUserValue(10)) * KoDpi::dpiY();
            } else if (KoUnit::unitName(m_unit) == "pt") {
                m_majorX = POINT_TO_INCH(m_unit.fromUserValue(100)) * KoDpi::dpiX();
                m_majorY = POINT_TO_INCH(m_unit.fromUserValue(100)) * KoDpi::dpiY();
            } else {
                m_majorX = POINT_TO_INCH(m_unit.fromUserValue(1)) * KoDpi::dpiX();
                m_majorY = POINT_TO_INCH(m_unit.fromUserValue(1)) * KoDpi::dpiY();
            }

        }
    }
    m_minorSteps = m_rd->propertySet()->property("grid-divisions").value().toInt();
    m_pixelIncrementX = (m_majorX / m_minorSteps);
    m_pixelIncrementY = (m_majorY / m_minorSteps);

    return QPointF(qRound((p.x() / m_pixelIncrementX)) * m_pixelIncrementX, qRound((p.y() / m_pixelIncrementY)) * m_pixelIncrementY);
}

void ReportScene::focusOutEvent(QFocusEvent * focusEvent)
{
    emit(lostFocus());
    QGraphicsScene::focusOutEvent(focusEvent);
}

qreal ReportScene::lowestZValue()
{
    qreal z;
    qreal zz;
    z = 0;
    QGraphicsItemList list = items();
    for (QGraphicsItemList::iterator it = list.begin(); it != list.end(); ++it) {
        zz = (*it)->zValue();
        if (zz < z) {
            z = zz;
        }

    }
    return z;
}

qreal ReportScene::highestZValue()
{
    qreal z;
    qreal zz;
    z = 0;
    QGraphicsItemList list = items();
    for (QGraphicsItemList::iterator it = list.begin(); it != list.end(); ++it) {
        zz = (*it)->zValue();
        if (zz > z) {
            z = zz;
        }

    }
    return z;
}

void ReportScene::lowerSelected()
{
    QGraphicsItemList list = selectedItems();
    for (QGraphicsItemList::iterator it = list.begin();
            it != list.end(); ++it) {
        (*it)->setZValue(lowestZValue() - 1);
    }
}

void ReportScene::raiseSelected()
{
    QGraphicsItemList list = selectedItems();
    for (QGraphicsItemList::iterator it = list.begin();
            it != list.end(); ++it) {
        (*it)->setZValue(highestZValue() + 1);
    }
}

QGraphicsItemList ReportScene::itemsOrdered()
{
    QGraphicsItemList r;
    QGraphicsItemList list = items();
    for (QGraphicsItemList::iterator it = list.begin(); it != list.end(); ++it) {
        for (QGraphicsItemList::iterator rit = r.begin(); rit != r.end(); ++rit) {


        }

    }


    return r;
}
