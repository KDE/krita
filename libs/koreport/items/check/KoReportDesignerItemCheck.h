/*
 * Kexi Report Plugin
 * Copyright (C) 2009-2010 by Adam Pigg (adam@piggz.co.uk)
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

#ifndef REPORTENTITYCHECK_H
#define REPORTENTITYCHECK_H

#include <KoReportDesignerItemRectBase.h>
#include <QGraphicsRectItem>
#include <koproperty/Property.h>
#include <koproperty/Set.h>
#include "KoReportItemCheck.h"

class KoReportDesignerItemCheck : public KoReportItemCheck, public KoReportDesignerItemRectBase
{
    Q_OBJECT
public:
    KoReportDesignerItemCheck(KoReportDesigner *, QGraphicsScene * scene, const QPointF &pos);
    KoReportDesignerItemCheck(QDomNode & element, KoReportDesigner *, QGraphicsScene * scene);

    virtual ~KoReportDesignerItemCheck();

    virtual void buildXML(QDomDocument & doc, QDomElement & parent);

    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);

    virtual KoReportDesignerItemCheck* clone();
private:
    void init(QGraphicsScene*);

private slots:
    void slotPropertyChanged(KoProperty::Set &, KoProperty::Property &);

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent * event);
};

#endif // REPORTENTITYCHECK_H
