/*
 * Kexi Report Plugin
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

#ifndef __REPORTENTITYSHAPE_H__
#define __REPORTENTITYSHAPE_H__

#include <QGraphicsRectItem>
#include <koproperty/Property.h>
#include <koproperty/Set.h>
#include <krshapedata.h>
#include "reportrectentity.h"
#include <KoShape.h>
#include <KoShapePainter.h>
#include <KoZoomHandler.h>

//
// ReportEntityLabel
//
class ReportEntityShape : public QObject, public ReportRectEntity, public KRShapeData
{
    Q_OBJECT
public:
    ReportEntityShape(KoReportDesigner *, QGraphicsScene * scene, const QPointF &pos);
    ReportEntityShape(QDomNode & element, KoReportDesigner *, QGraphicsScene * scene);
    virtual ~ReportEntityShape();

    virtual void buildXML(QDomDocument & doc, QDomElement & parent);
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);
    virtual ReportEntityShape* clone();

private:
    void init(QGraphicsScene*);
    KoShape* m_shape;
    KoShapePainter m_shapePainter;
    KoZoomHandler m_zoomHandle;

private slots:
    void slotPropertyChanged(KoProperty::Set &, KoProperty::Property &);
};

#endif
