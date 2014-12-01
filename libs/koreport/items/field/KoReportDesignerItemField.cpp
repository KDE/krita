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

#include "KoReportDesignerItemField.h"
#include "KoReportItemField.h"
#include "KoReportDesigner.h"

#include <QDomDocument>
#include <QPainter>
#include <kdebug.h>
#include <klocalizedstring.h>
#include <QGraphicsScene>
#include <koproperty/EditorView.h>
#include <QGraphicsSceneMouseEvent>

//
// class ReportEntityField
//

void KoReportDesignerItemField::init(QGraphicsScene * scene, KoReportDesigner * d)
{
    if (scene)
        scene->addItem(this);

    KoReportDesignerItemRectBase::init(&m_pos, &m_size, m_set, d);
    
    connect(m_set, SIGNAL(propertyChanged(KoProperty::Set&,KoProperty::Property&)),
            this, SLOT(slotPropertyChanged(KoProperty::Set&,KoProperty::Property&)));

    setZValue(Z);
}

// methods (constructors)
KoReportDesignerItemField::KoReportDesignerItemField(KoReportDesigner * rw, QGraphicsScene * scene, const QPointF &pos)
        : KoReportDesignerItemRectBase(rw)
{
    Q_UNUSED(pos);
    init(scene, rw);
    setSceneRect(properRect(*rw, getTextRect().width(), getTextRect().height()));
    m_name->setValue(m_reportDesigner->suggestEntityName(typeName()));
}

KoReportDesignerItemField::KoReportDesignerItemField(QDomNode & element, KoReportDesigner * d, QGraphicsScene * s)
        : KoReportItemField(element), KoReportDesignerItemRectBase(d)
{
    init(s, d);
    setSceneRect(m_pos.toScene(), m_size.toScene());
}

KoReportDesignerItemField* KoReportDesignerItemField::clone()
{
    QDomDocument d;
    QDomElement e = d.createElement("clone");;
    QDomNode n;
    buildXML(d, e);
    n = e.firstChild();
    return new KoReportDesignerItemField(n, designer(), 0);
}

// methods (deconstructor)
KoReportDesignerItemField::~KoReportDesignerItemField()
{}

QRect KoReportDesignerItemField::getTextRect() const
{
    return QFontMetrics(font()).boundingRect(x(), y(), 0, 0, textFlags(), dataSourceAndObjectTypeName(itemDataSource(), "field"));
}



void KoReportDesignerItemField::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // store any values we plan on changing so we can restore them
    QFont f = painter->font();
    QPen  p = painter->pen();

    painter->setFont(font());
    painter->setBackgroundMode(Qt::TransparentMode);

    QColor bg = m_backgroundColor->value().value<QColor>();
    bg.setAlphaF(m_backgroundOpacity->value().toReal() *0.01);

    painter->setPen(m_foregroundColor->value().value<QColor>());

    painter->fillRect(QGraphicsRectItem::rect(), bg);
    painter->drawText(rect(), textFlags(), dataSourceAndObjectTypeName(itemDataSource(), "field"));


    if ((Qt::PenStyle)m_lineStyle->value().toInt() == Qt::NoPen || m_lineWeight->value().toInt() <= 0) {
        painter->setPen(QPen(Qt::lightGray));
    } else {
        painter->setPen(QPen(m_lineColor->value().value<QColor>(), m_lineWeight->value().toInt(), (Qt::PenStyle)m_lineStyle->value().toInt()));
    }

    painter->drawRect(rect());


    drawHandles(painter);

    // restore an values before we started just in case
    painter->setFont(f);
    painter->setPen(p);
}

void KoReportDesignerItemField::buildXML(QDomDocument & doc, QDomElement & parent)
{
    QDomElement entity = doc.createElement(QLatin1String("report:") + typeName());

    // properties
    addPropertyAsAttribute(&entity, m_name);
    addPropertyAsAttribute(&entity, m_controlSource);
    addPropertyAsAttribute(&entity, m_verticalAlignment);
    addPropertyAsAttribute(&entity, m_horizontalAlignment);
    addPropertyAsAttribute(&entity, m_wordWrap);
    addPropertyAsAttribute(&entity, m_canGrow);
    entity.setAttribute("report:z-index", zValue());

    // bounding rect
    buildXMLRect(doc, entity, &m_pos, &m_size);

    //text style info
    buildXMLTextStyle(doc, entity, textStyle());

    //Line Style
    buildXMLLineStyle(doc, entity, lineStyle());


#if 0 //Field Totals
    if (m_trackTotal) {
        QDomElement tracktotal = doc.createElement("tracktotal");
        if (m_trackBuiltinFormat)
            tracktotal.setAttribute("builtin", "true");
        if (_useSubTotal)
            tracktotal.setAttribute("subtotal", "true");
        tracktotal.appendChild(doc.createTextNode(_trackTotalFormat->value().toString()));
        entity.appendChild(tracktotal);
    }
#endif

    parent.appendChild(entity);
}

void KoReportDesignerItemField::slotPropertyChanged(KoProperty::Set &s, KoProperty::Property &p)
{
    Q_UNUSED(s);

    if (p.name() == "Name") {
        //For some reason p.oldValue returns an empty string
        if (!m_reportDesigner->isEntityNameUnique(p.value().toString(), this)) {
            p.setValue(m_oldName);
        } else {
            m_oldName = p.value().toString();
        }
    }

    KoReportDesignerItemRectBase::propertyChanged(s, p);
    if (m_reportDesigner)m_reportDesigner->setModified(true);
}

void KoReportDesignerItemField::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    m_controlSource->setListData(m_reportDesigner->fieldKeys(), m_reportDesigner->fieldNames());
    KoReportDesignerItemRectBase::mousePressEvent(event);
}
