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

#ifndef __KOREPORTDESIGNERITEMLABEL_H__
#define __KOREPORTDESIGNERITEMLABEL_H__

#include <QGraphicsRectItem>
#include <koproperty/Property.h>
#include <koproperty/Set.h>
#include "KoReportItemLabel.h"
#include <KoReportDesignerItemRectBase.h>
#include <QGraphicsItem>
#include <QGraphicsItemGroup>
#include "BoundedTextItem.h"

//
// ReportEntityLabel
//
class KoReportDesignerItemLabel : public KoReportItemLabel, public KoReportDesignerItemRectBase
{
  Q_OBJECT
public:
    KoReportDesignerItemLabel(KoReportDesigner *, QGraphicsScene * scene, const QPointF &pos);
    KoReportDesignerItemLabel(QDomNode & element, KoReportDesigner *, QGraphicsScene * scene);
    virtual ~KoReportDesignerItemLabel();

    virtual void buildXML(QDomDocument & doc, QDomElement & parent);
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);
    virtual KoReportDesignerItemLabel* clone();
    
public slots:
    virtual void enterInlineEditingMode();
    virtual void exitInlineEditingMode();
    
protected:
    virtual void mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event );
    virtual void keyReleaseEvent ( QKeyEvent * event );
  
private:
    void init(QGraphicsScene*, KoReportDesigner*);
    QRectF getTextRect() const;
    BoundedTextItem *m_inlineEdit;

private slots:
    void slotPropertyChanged(KoProperty::Set &, KoProperty::Property &);

};

#endif
