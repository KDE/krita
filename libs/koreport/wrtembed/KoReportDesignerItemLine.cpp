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

#include "KoReportDesignerItemLine.h"
#include "KoReportDesignerItemBase.h"
#include "KoReportDesigner.h"

#include <QDomDocument>
#include <QPainter>
#include <kdebug.h>
#include <klocalizedstring.h>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

#include <koproperty/EditorView.h>
#include <KoGlobal.h>
#include "reportscene.h"
#include "krutils.h"

//
// class ReportEntityLine
//
void KoReportDesignerItemLine::init(QGraphicsScene* s, KoReportDesigner *r)
{
    m_reportDesigner = r;
    setPos(0, 0);
    setUnit(r->pageUnit());

#if QT_VERSION >= 0x040600
    setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges);
#else
    setFlags(ItemIsSelectable | ItemIsMovable);
#endif

    setPen(QPen(Qt::black, 5));
    setAcceptsHoverEvents(true);

    if (s)
        s->addItem(this);

    connect(m_set, SIGNAL(propertyChanged(KoProperty::Set&, KoProperty::Property&)),
            this, SLOT(slotPropertyChanged(KoProperty::Set&, KoProperty::Property&)));

    setZValue(Z);
}

KoReportDesignerItemLine::KoReportDesignerItemLine(KoReportDesigner * d, QGraphicsScene * scene, const QPointF &pos)
        : KoReportDesignerItemBase(d)
{
    init(scene, d);
    setLineScene(QLineF(pos, QPointF(20,20)+pos));

    m_name->setValue(m_reportDesigner->suggestEntityName("line"));
}

KoReportDesignerItemLine::KoReportDesignerItemLine(QDomNode & entity, KoReportDesigner * d, QGraphicsScene * scene)
        : KoReportItemLine(entity), KoReportDesignerItemBase(d)
{
    init(scene, d);
    setLine ( m_start.toScene().x(), m_start.toScene().y(), m_end.toScene().x(), m_end.toScene().y() );
}

KoReportDesignerItemLine* KoReportDesignerItemLine::clone()
{
    QDomDocument d;
    QDomElement e = d.createElement("clone");
    QDomNode n;
    buildXML(d, e);
    n = e.firstChild();
    return new KoReportDesignerItemLine(n, designer(), 0);
}

void KoReportDesignerItemLine::paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
                             QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing, true);
    QPen p = painter->pen();
    painter->setPen(QPen(m_lineColor->value().value<QColor>(), m_lineWeight->value().toInt(), (Qt::PenStyle)m_lineStyle->value().toInt()));
    painter->drawLine(line());
    if (isSelected()) {

        // draw a selected border for visual purposes
        painter->setPen(QPen(QColor(128, 128, 255), 0, Qt::DotLine));
        QPointF pt = line().p1();
        painter->fillRect(pt.x(), pt.y() - 2, 5, 5, QColor(128, 128, 255));
        pt = line().p2();
        painter->fillRect(pt.x() - 4, pt.y() - 2, 5, 5, QColor(128, 128, 255));

        painter->setPen(p);
    }
}

void KoReportDesignerItemLine::buildXML(QDomDocument & doc, QDomElement & parent)
{
    QDomElement entity = doc.createElement("report:line");

    // properties
    addPropertyAsAttribute(&entity, m_name);
    entity.setAttribute("report:z-index", zValue());
    KRUtils::setAttribute(entity, "svg:x1", m_start.toPoint().x());
    KRUtils::setAttribute(entity, "svg:y1", m_start.toPoint().y());
    KRUtils::setAttribute(entity, "svg:x2", m_end.toPoint().x());
    KRUtils::setAttribute(entity, "svg:y2", m_end.toPoint().y());

    buildXMLLineStyle(doc, entity, lineStyle());

    parent.appendChild(entity);
}

void KoReportDesignerItemLine::slotPropertyChanged(KoProperty::Set &s, KoProperty::Property &p)
{
    Q_UNUSED(s);

    if (p.name() == "Start" || p.name() == "End") {
        if (p.name() == "Start")
            m_start.setUnitPos(p.value().toPointF(), KRPos::DontUpdateProperty);
        if (p.name() == "End")
            m_end.setUnitPos(p.value().toPointF(), KRPos::DontUpdateProperty);

        setLine(m_start.toScene().x(), m_start.toScene().y(), m_end.toScene().x(), m_end.toScene().y());
    }
    else if (p.name() == "Name") {
        //For some reason p.oldValue returns an empty string
        if (!m_reportDesigner->isEntityNameUnique(p.value().toString(), this)) {
            p.setValue(m_oldName);
        } else {
            m_oldName = p.value().toString();
        }
    }
    if (m_reportDesigner)
        m_reportDesigner->setModified(true);
}

void KoReportDesignerItemLine::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    m_reportDesigner->changeSet(m_set);
    setSelected(true);
    QGraphicsLineItem::mousePressEvent(event);
}

QVariant KoReportDesignerItemLine::itemChange(GraphicsItemChange change, const QVariant &value)
{
    kDebug();
    return QGraphicsItem::itemChange(change, value);
}

void KoReportDesignerItemLine::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
    QGraphicsLineItem::mouseReleaseEvent(event);
}

void KoReportDesignerItemLine::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    int x;
    int y;

    QPointF p = dynamic_cast<ReportScene*>(scene())->gridPoint(event->scenePos());

    kDebug() << p;
    
    x = p.x();
    y = p.y();

    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x > scene()->width()) x = scene()->width();
    if (y > scene()->height()) y = scene()->height();

    switch (m_grabAction) {
    case 1:
	m_start.setScenePos(QPointF(x,y));
        break;
    case 2:
	m_end.setScenePos(QPointF(x,y));
        break;
    default:
        QPointF d = mapToItem(this, dynamic_cast<ReportScene*>(scene())->gridPoint(event->scenePos())) - mapToItem(this, dynamic_cast<ReportScene*>(scene())->gridPoint(event->lastScenePos()));

        if (((line().p1() + d).x() >= 0) &&
                ((line().p2() + d).x() >= 0) &&
                ((line().p1() + d).y() >= 0) &&
                ((line().p2() + d).y() >= 0)  &&
                ((line().p1() + d).x() <= scene()->width()) &&
                ((line().p2() + d).x() <= scene()->width()) &&
                ((line().p1() + d).y() <= scene()->height()) &&
                ((line().p2() + d).y() <= scene()->height()))
            setLineScene(QLineF(line().p1() + d, line().p2() + d));
        break;
    }
}

int KoReportDesignerItemLine::grabHandle(QPointF pos)
{
    if (QRectF(line().p1().x(), line().p1().y() - 2, 5, 5).contains(pos)) {
        // we are over point 1
        return 1;
    } else if (QRectF(line().p2().x() - 4, line().p2().y() - 2, 5, 5).contains(pos)) {
        // we are over point 2
        return 2;
    } else {
        return 0;
    }

}

void KoReportDesignerItemLine::hoverMoveEvent(QGraphicsSceneHoverEvent * event)
{
    if (isSelected()) {
        m_grabAction = grabHandle(event->pos());
        switch (m_grabAction) {
        case 1: //Point 1
            setCursor(Qt::SizeAllCursor);
            break;
        case 2: //Point 2
            setCursor(Qt::SizeAllCursor);
            break;
        default:
            unsetCursor();
        }
    }
}

void KoReportDesignerItemLine::setLineScene(QLineF l)
{
    m_start.setScenePos(l.p1(), KRPos::DontUpdateProperty);
    m_end.setScenePos(l.p2());
}

void KoReportDesignerItemLine::move(const QPointF& m)
{
    QPointF original = m_pos.toScene();
    original += m;
    m_pos.setScenePos(original);
}
