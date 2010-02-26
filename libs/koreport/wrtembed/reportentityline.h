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


#ifndef __REPORTENTITYLINE_H__
#define __REPORTENTITYLINE_H__

#include <QGraphicsLineItem>
#include <koproperty/Property.h>
#include <koproperty/Set.h>
#include "reportentities.h"
#include <krlinedata.h>

class KoReportDesigner;

//
// ReportEntityLine
//
class ReportEntityLine : public QObject, public KRLineData, public QGraphicsLineItem, public ReportEntity
{
    Q_OBJECT
public:
    ReportEntityLine(KoReportDesigner *, QGraphicsScene * scene, const QPointF &pos);
    ReportEntityLine(QDomNode & element, KoReportDesigner *, QGraphicsScene * scene);

    virtual void buildXML(QDomDocument & doc, QDomElement & parent);
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget *widget = 0);
    virtual ReportEntityLine* clone();

    void setUnit(KoUnit u);
    void setLineScene(QLineF);
    
private:
    KoReportDesigner* m_rd;
    void init(QGraphicsScene*, KoReportDesigner *);
    int grabHandle(QPointF pos);

    int m_grabAction;

protected:
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent * event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent * event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private slots:
    void slotPropertyChanged(KoProperty::Set &, KoProperty::Property &);
};

#endif
