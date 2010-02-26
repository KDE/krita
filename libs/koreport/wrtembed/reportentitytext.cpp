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

#include "reportentitytext.h"
#include "reportentities.h"
#include "KoReportDesigner.h"

#include <qdom.h>
#include <qpainter.h>
#include <kdebug.h>
#include <klocalizedstring.h>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

#include <koproperty/Property.h>
#include <koproperty/Set.h>
#include <koproperty/EditorView.h>
//
// class ReportEntityText
//
// methods (constructors)

void ReportEntityText::init(QGraphicsScene * scene)
{
    //setFlags(ItemIsSelectable | ItemIsMovable);
    if (scene)
        scene->addItem(this);

    connect(properties(), SIGNAL(propertyChanged(KoProperty::Set&, KoProperty::Property&)),
            this, SLOT(slotPropertyChanged(KoProperty::Set&, KoProperty::Property&)));

    ReportRectEntity::init(&m_pos, &m_size, m_set);

    m_controlSource->setListData(m_reportDesigner->fieldKeys(), m_reportDesigner->fieldNames());
    setZValue(Z);
}

ReportEntityText::ReportEntityText(KoReportDesigner * rw, QGraphicsScene * scene, const QPointF &pos)
        : ReportRectEntity(rw)
{
    init(scene);
    setSceneRect(getTextRect());
    m_pos.setScenePos(pos);
    m_name->setValue(m_reportDesigner->suggestEntityName("text"));
}

ReportEntityText::ReportEntityText(QDomNode & element, KoReportDesigner * d, QGraphicsScene * s)
        : KRTextData(element), ReportRectEntity(d)
{
    init(s);
    setSceneRect(m_pos.toScene(), m_size.toScene());
}

ReportEntityText* ReportEntityText::clone()
{
    QDomDocument d;
    QDomElement e = d.createElement("clone");;
    QDomNode n;
    buildXML(d, e);
    n = e.firstChild();
    return new ReportEntityText(n, designer(), 0);
}

ReportEntityText::~ReportEntityText()
{}

QRect ReportEntityText::getTextRect()
{
    return QFontMetrics(font()).boundingRect(int (x()), int (y()), 0, 0, textFlags(), dataSourceAndObjectTypeName(controlSource(), "textarea"));
}

void ReportEntityText::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget)

    // store any values we plan on changing so we can restore them
    QFont f = painter->font();
    QPen  p = painter->pen();

    painter->setFont(font());

    QColor bg = m_backgroundColor->value().value<QColor>();
    bg.setAlpha((m_backgroundOpacity->value().toInt() / 100) * 255);

    painter->setBackground(bg);
    painter->setPen(m_foregroundColor->value().value<QColor>());

    painter->fillRect(rect(),  m_backgroundColor->value().value<QColor>());
    painter->drawText(rect(), textFlags(), dataSourceAndObjectTypeName(controlSource(), "textarea"));

    if ((Qt::PenStyle)m_lineStyle->value().toInt() == Qt::NoPen || m_lineWeight->value().toInt() <= 0) {
        painter->setPen(QPen(QColor(224, 224, 224)));
    } else {
        painter->setPen(QPen(m_lineColor->value().value<QColor>(), m_lineWeight->value().toInt(), (Qt::PenStyle)m_lineStyle->value().toInt()));
    }
    painter->drawRect(rect());

    painter->setBackgroundMode(Qt::TransparentMode);
    painter->setPen(m_foregroundColor->value().value<QColor>());

    drawHandles(painter);

    // restore an values before we started just in case
    painter->setFont(f);
    painter->setPen(p);
}

void ReportEntityText::buildXML(QDomDocument & doc, QDomElement & parent)
{
    //kdDebug() << "ReportEntityText::buildXML()");
    QDomElement entity = doc.createElement("report:text");

    // properties
    addPropertyAsAttribute(&entity, m_name);
    addPropertyAsAttribute(&entity, m_controlSource);
    addPropertyAsAttribute(&entity, m_verticalAlignment);
    addPropertyAsAttribute(&entity, m_horizontalAlignment);
    entity.setAttribute("report:bottom-padding", m_bottomPadding);
    entity.setAttribute("report:z-index", zValue());

    // bounding rect
    buildXMLRect(doc, entity, &m_pos, &m_size);

    //text style info
    buildXMLTextStyle(doc, entity, textStyle());

    //Line Style
    buildXMLLineStyle(doc, entity, lineStyle());

    parent.appendChild(entity);
}

void ReportEntityText::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    m_controlSource->setListData(m_reportDesigner->fieldKeys(), m_reportDesigner->fieldNames());
    ReportRectEntity::mousePressEvent(event);
}


void ReportEntityText::slotPropertyChanged(KoProperty::Set &s, KoProperty::Property &p)
{
    Q_UNUSED(s);

    if (p.name() == "Position") {
        m_pos.setUnitPos(p.value().toPointF(), KRPos::DontUpdateProperty);
    } else if (p.name() == "Size") {
        m_size.setUnitSize(p.value().toSizeF(), KRPos::DontUpdateProperty);
    } else if (p.name() == "Name") {
        //For some reason p.oldValue returns an empty string
        if (!m_reportDesigner->isEntityNameUnique(p.value().toString(), this)) {
            p.setValue(m_oldName);
        } else {
            m_oldName = p.value().toString();
        }
    }

    setSceneRect(m_pos.toScene(), m_size.toScene(), DontUpdateProperty);
    if (m_reportDesigner)
        m_reportDesigner->setModified(true);
    if (scene())
        scene()->update();
}
