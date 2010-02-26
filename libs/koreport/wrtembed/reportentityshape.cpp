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

#include "reportentityshape.h"
#include "reportentities.h"
#include "KoReportDesigner.h"

#include <qdom.h>
#include <QPainter>
#include <kdebug.h>
#include <klocalizedstring.h>
#include <koproperty/EditorView.h>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <KoShapeRegistry.h>


//
// class ReportEntityLabel
//

void ReportEntityShape::init(QGraphicsScene * scene)
{
    if (scene)
        scene->addItem(this);

    ReportRectEntity::init(&m_pos, &m_size, m_set);

    connect(properties(), SIGNAL(propertyChanged(KoProperty::Set&, KoProperty::Property&)),
            this, SLOT(slotPropertyChanged(KoProperty::Set&, KoProperty::Property&)));

    setZValue(Z);
}

// methods (constructors)
ReportEntityShape::ReportEntityShape(KoReportDesigner* d, QGraphicsScene * scene, const QPointF &pos)
        : ReportRectEntity(d)
{
    init(scene);
    setSceneRect(QPointF(0, 0), QSizeF(100, 100));
    m_pos.setScenePos(pos);
    m_name->setValue(m_reportDesigner->suggestEntityName("shape"));

}

ReportEntityShape::ReportEntityShape(QDomNode & element, KoReportDesigner * d, QGraphicsScene * s)
        : ReportRectEntity(d), KRShapeData(element)
{
    init(s);
    setSceneRect(m_pos.toScene(), m_size.toScene());
}

ReportEntityShape* ReportEntityShape::clone()
{
    QDomDocument d;
    QDomElement e = d.createElement("clone");;
    QDomNode n;
    buildXML(d, e);
    n = e.firstChild();
    return new ReportEntityShape(n, designer(), 0);
}

// methods (deconstructor)
ReportEntityShape::~ReportEntityShape()
{}

void ReportEntityShape::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QList<KoShape*> shapes;
    painter->setRenderHint(QPainter::Antialiasing);

    m_shape = (KoShapeRegistry::instance()->value(m_shapeType->value().toString()))->createDefaultShape();
    m_shape->setSize(m_size.toPoint() - QSizeF(4,4));
    shapes << m_shape;

    m_shapePainter.setShapes(shapes);

    m_shapePainter.paint(*painter, m_zoomHandle);
    //mShape->paint(*painter, z);

    drawHandles(painter);
}

void ReportEntityShape::buildXML(QDomDocument & doc, QDomElement & parent)
{
    kDebug();
    //kdDebug() << "ReportEntityLabel::buildXML()");
    QDomElement entity = doc.createElement("shape");

    // bounding rect
    buildXMLRect(doc, entity, &m_pos, &m_size);

    // name
    QDomElement n = doc.createElement("name");
    n.appendChild(doc.createTextNode(entityName()));
    entity.appendChild(n);

    // z
    QDomElement z = doc.createElement("zvalue");
    z.appendChild(doc.createTextNode(QString::number(zValue())));
    entity.appendChild(z);

    parent.appendChild(entity);
}

void ReportEntityShape::slotPropertyChanged(KoProperty::Set &s, KoProperty::Property &p)
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
