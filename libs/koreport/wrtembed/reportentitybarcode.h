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


#ifndef __REPORTENTITYBARCODE_H__
#define __REPORTENTITYBARCODE_H__

#include <koproperty/Property.h>
#include <koproperty/Set.h>

#include "reportrectentity.h"
#include <krbarcodedata.h>

//
// ReportEntityBarcode
//
class ReportEntityBarcode : public QObject, public KRBarcodeData, public ReportRectEntity
{
    Q_OBJECT
public:
    ReportEntityBarcode(KoReportDesigner *, QGraphicsScene* scene, const QPointF &pos);
    ReportEntityBarcode(QDomNode & element, KoReportDesigner *, QGraphicsScene* scene);

    virtual ~ReportEntityBarcode();
    virtual void buildXML(QDomDocument & doc, QDomElement & parent);
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);

    virtual ReportEntityBarcode* clone();

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent * event);

private:
    void init(QGraphicsScene*);

    QRect getTextRect();

private slots:
    void slotPropertyChanged(KoProperty::Set &, KoProperty::Property &);
};

#endif
