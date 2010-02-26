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

#include "reportentitycheck.h"
#include "reportentities.h"
#include "KoReportDesigner.h"

#include <qdom.h>
#include <qpainter.h>
#include <kdebug.h>
#include <klocalizedstring.h>
#include <koproperty/EditorView.h>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

//
// class ReportEntityCheck
//

void ReportEntityCheck::init(QGraphicsScene * scene)
{
    if (scene)
        scene->addItem(this);

    ReportRectEntity::init(&m_pos, &m_size, m_set);

    connect(properties(), SIGNAL(propertyChanged(KoProperty::Set&, KoProperty::Property&)),
            this, SLOT(slotPropertyChanged(KoProperty::Set&, KoProperty::Property&)));

    setZValue(Z);
}

// methods (constructors)
ReportEntityCheck::ReportEntityCheck(KoReportDesigner* d, QGraphicsScene * scene, const QPointF &pos)
        : ReportRectEntity(d)
{
    init(scene);
    setSceneRect(QPointF(0, 0), QSizeF(15, 15)); //default size
    m_pos.setScenePos(pos);
    m_name->setValue(m_reportDesigner->suggestEntityName("check"));

}

ReportEntityCheck::ReportEntityCheck(QDomNode & element, KoReportDesigner * d, QGraphicsScene * s)
        : ReportRectEntity(d), KRCheckData(element)
{
    init(s);
    setSceneRect(m_pos.toScene(), m_size.toScene());
}

ReportEntityCheck* ReportEntityCheck::clone()
{
    QDomDocument d;
    QDomElement e = d.createElement("clone");;
    QDomNode n;
    buildXML(d, e);
    n = e.firstChild();
    return new ReportEntityCheck(n, designer(), 0);
}

// methods (deconstructor)
ReportEntityCheck::~ReportEntityCheck()
{}

void ReportEntityCheck::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // store any values we plan on changing so we can restore them
    QFont f = painter->font();
    QPen  p = painter->pen();
    QBrush b = painter->brush();

    painter->setBackgroundMode(Qt::OpaqueMode);
    painter->setRenderHint(QPainter::Antialiasing);

    painter->setPen(m_foregroundColor->value().value<QColor>());

    if ((Qt::PenStyle)m_lineStyle->value().toInt() == Qt::NoPen || m_lineWeight->value().toInt() <= 0) {
        painter->setPen(QPen(QColor(224, 224, 224)));
    } else {
        painter->setPen(QPen(m_lineColor->value().value<QColor>(), m_lineWeight->value().toInt(), (Qt::PenStyle)m_lineStyle->value().toInt()));
    }

    qreal ox = m_size.toScene().width() / 5;
    qreal oy = m_size.toScene().height() / 5;

    //Checkbox Style
    if (m_checkStyle->value().toString() == "Cross") {
        painter->drawRoundedRect(QGraphicsRectItem::rect(), m_size.toScene().width() / 10 , m_size.toScene().height() / 10);

        QPen lp;
        lp.setColor(m_foregroundColor->value().value<QColor>());
        lp.setWidth(ox > oy ? oy : ox);
        painter->setPen(lp);
        painter->drawLine(ox, oy, m_size.toScene().width() - ox, m_size.toScene().height() - oy);
        painter->drawLine(ox, m_size.toScene().height() - oy, m_size.toScene().width() - ox, oy);
    } else if (m_checkStyle->value().toString() == "Dot") {
        //Radio Style
        painter->drawEllipse(QGraphicsRectItem::rect());

        QBrush lb(m_foregroundColor->value().value<QColor>());
        painter->setBrush(lb);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(rect().center(), m_size.toScene().width() / 2 - ox, m_size.toScene().height() / 2 - oy);
    } else {
        //Tickbox Style
        painter->drawRoundedRect(QGraphicsRectItem::rect(), m_size.toScene().width() / 10 , m_size.toScene().height() / 10);

        QPen lp;
        lp.setColor(m_foregroundColor->value().value<QColor>());
        lp.setWidth(ox > oy ? oy : ox);
        painter->setPen(lp);
        painter->drawLine(ox, m_size.toScene().height() / 2, m_size.toScene().width() / 2, m_size.toScene().height() - oy);
        painter->drawLine(m_size.toScene().width() / 2, m_size.toScene().height() - oy, m_size.toScene().width() - ox, oy);

    }

    painter->setBackgroundMode(Qt::TransparentMode);
    painter->setPen(m_foregroundColor->value().value<QColor>());

    // restore an values before we started just in case
    painter->setFont(f);
    painter->setPen(p);
    painter->setBrush(b);

    drawHandles(painter);
}

void ReportEntityCheck::buildXML(QDomDocument & doc, QDomElement & parent)
{
    kDebug() << endl;

    QDomElement entity = doc.createElement("report:check");

    //properties
    addPropertyAsAttribute(&entity, m_name);
    addPropertyAsAttribute(&entity, m_controlSource);
    entity.setAttribute("fo:foreground-color", m_foregroundColor->value().toString());
    addPropertyAsAttribute(&entity, m_checkStyle);

    // bounding rect
    buildXMLRect(doc, entity, &m_pos, &m_size);

    //Line Style
    buildXMLLineStyle(doc, entity, lineStyle());

    parent.appendChild(entity);
}

void ReportEntityCheck::slotPropertyChanged(KoProperty::Set &s, KoProperty::Property &p)
{
    Q_UNUSED(s)

    if (p.name() == "Name") {
        //For some reason p.oldValue returns an empty string
        if (!m_reportDesigner->isEntityNameUnique(p.value().toString(), this)) {
            p.setValue(m_oldName);
        } else {
            m_oldName = p.value().toString();
        }
    }

    ReportRectEntity::propertyChanged(s, p);
    if (m_reportDesigner) m_reportDesigner->setModified(true);
}

void ReportEntityCheck::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    m_controlSource->setListData(m_reportDesigner->fieldKeys(), m_reportDesigner->fieldNames());
    ReportRectEntity::mousePressEvent(event);
}
