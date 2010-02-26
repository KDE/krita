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

#include "reportentityfield.h"
#include "reportentities.h"
#include "KoReportDesigner.h"

#include <qdom.h>
#include <qpainter.h>
#include <kdebug.h>
#include <klocalizedstring.h>
#include <QGraphicsScene>
#include <koproperty/EditorView.h>
#include <QGraphicsSceneMouseEvent>

//
// class ReportEntityField
//

void ReportEntityField::init(QGraphicsScene * scene)
{
    if (scene)
        scene->addItem(this);

    ReportRectEntity::init(&m_pos, &m_size, m_set);
    
    connect(m_set, SIGNAL(propertyChanged(KoProperty::Set&, KoProperty::Property&)),
            this, SLOT(slotPropertyChanged(KoProperty::Set&, KoProperty::Property&)));

    setZValue(Z);
}

// methods (constructors)
ReportEntityField::ReportEntityField(KoReportDesigner * rw, QGraphicsScene * scene, const QPointF &pos)
        : ReportRectEntity(rw)
{
    init(scene);
    setSceneRect(getTextRect());
    m_pos.setScenePos(pos);
    m_name->setValue(m_reportDesigner->suggestEntityName("field"));
}

ReportEntityField::ReportEntityField(QDomNode & element, KoReportDesigner * d, QGraphicsScene * s)
        : ReportRectEntity(d), KRFieldData(element)
{
    init(s);
    setSceneRect(m_pos.toScene(), m_size.toScene());
}

ReportEntityField* ReportEntityField::clone()
{
    QDomDocument d;
    QDomElement e = d.createElement("clone");;
    QDomNode n;
    buildXML(d, e);
    n = e.firstChild();
    return new ReportEntityField(n, designer(), 0);
}

// methods (deconstructor)
ReportEntityField::~ReportEntityField()
{}

QRect ReportEntityField::getTextRect()
{
    return QFontMetrics(font()).boundingRect(int (x()), int (y()), 0, 0, textFlags(), dataSourceAndObjectTypeName(controlSource(), "field"));
}



void ReportEntityField::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // store any values we plan on changing so we can restore them
    QFont f = painter->font();
    QPen  p = painter->pen();

    painter->setFont(font());

    QColor bg = m_backgroundColor->value().value<QColor>();
    bg.setAlpha((m_backgroundOpacity->value().toInt() / 100) * 255);

    painter->setBackground(bg);
    painter->setPen(m_foregroundColor->value().value<QColor>());

    painter->fillRect(QGraphicsRectItem::rect(), bg);
    painter->drawText(rect(), textFlags(), dataSourceAndObjectTypeName(controlSource(), "field"));


    if ((Qt::PenStyle)m_lineStyle->value().toInt() == Qt::NoPen || m_lineWeight->value().toInt() <= 0) {
        painter->setPen(QPen(QColor(224, 224, 224)));
    } else {
        painter->setPen(QPen(m_lineColor->value().value<QColor>(), m_lineWeight->value().toInt(), (Qt::PenStyle)m_lineStyle->value().toInt()));
    }

    painter->drawRect(rect());

    painter->setBackgroundMode(Qt::TransparentMode);

    drawHandles(painter);

    // restore an values before we started just in case
    painter->setFont(f);
    painter->setPen(p);
}

void ReportEntityField::buildXML(QDomDocument & doc, QDomElement & parent)
{
    QDomElement entity = doc.createElement("report:field");

    // properties
    addPropertyAsAttribute(&entity, m_name);
    addPropertyAsAttribute(&entity, m_controlSource);
    addPropertyAsAttribute(&entity, m_verticalAlignment);
    addPropertyAsAttribute(&entity, m_horizontalAlignment);
    entity.setAttribute("report:z-index", zValue());

    // bounding rect
    buildXMLRect(doc, entity, &m_pos, &m_size);

    //text style info
    buildXMLTextStyle(doc, entity, textStyle());

    //Line Style
    buildXMLLineStyle(doc, entity, lineStyle());


#if 0
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

void ReportEntityField::slotPropertyChanged(KoProperty::Set &s, KoProperty::Property &p)
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

    ReportRectEntity::propertyChanged(s, p);
    if (m_reportDesigner)m_reportDesigner->setModified(true);
}

void ReportEntityField::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    m_controlSource->setListData(m_reportDesigner->fieldKeys(), m_reportDesigner->fieldNames());
    ReportRectEntity::mousePressEvent(event);
}
