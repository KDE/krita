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
// Class ReportCanvas
//
//     Overrides the drawForeground() method to do the grid.
//

#ifndef __REPORTSCENE_H__
#define __REPORTSCENE_H__

#include <QGraphicsScene>
#include <KoUnit.h>
typedef QList<QGraphicsItem*> QGraphicsItemList;
class KoReportDesigner;
class QGraphicsSceneContextMenuEvent;
class QGraphicsSceneMouseEvent;

class ReportScene : public QGraphicsScene
{
    Q_OBJECT
public:
    ReportScene(qreal w, qreal h, KoReportDesigner*);
    virtual ~ReportScene();
    KoReportDesigner* document() {
        return m_rd;
    }
    QPointF gridPoint(const QPointF&);
    void raiseSelected();
    void lowerSelected();
    QGraphicsItemList itemsOrdered();
    qreal gridSize() {
        return m_pixelIncrementX;
    }

protected:
    virtual void drawBackground(QPainter* painter, const QRectF & clip);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent * e);
    virtual void focusOutEvent(QFocusEvent * focusEvent);
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent * contextMenuEvent);

signals:
    void clicked();
    void lostFocus();

private:
    qreal lowestZValue();
    qreal highestZValue();

    KoReportDesigner * m_rd;

    KoUnit m_unit;
    int m_minorSteps;
    qreal m_majorX;
    qreal m_majorY;
    qreal m_pixelIncrementX;
    qreal m_pixelIncrementY;

};

#endif
