/*
 * Copyright (C) 2001-2007 by OpenMFG, LLC (info@openmfg.com)
 * Copyright (C) 2007-2008 by Adam Pigg (adam@piggz.co.uk)
 * Copyright (C) 2011-2015 by Radoslaw Wicik (radoslaw@wicik.pl)
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

#include "KoReportDesignerItemMaps.h"
#include <KoReportDesignerItemBase.h>
#include <KoReportDesigner.h>

#include <QImageWriter>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QBuffer>
#include <QDomDocument>
#include <QPainter>
#include <kdebug.h>
#include <klocalizedstring.h>

#include <koproperty/Property.h>
#include <koproperty/Set.h>
#include <koproperty/EditorView.h>

//
// ReportEntitiesImage
//
// contructors/deconstructors
//#define KDE_DEFAULT_DEBUG_AREA 44021
#define myDebug() kDebug(44021) << "\e[35m=="

void KoReportDesignerItemMaps::init(QGraphicsScene *scene, KoReportDesigner *d)
{
    if (scene)
        scene->addItem(this);

    KoReportDesignerItemRectBase::init(&m_pos, &m_size, m_set, d);

    connect(m_set, SIGNAL(propertyChanged(KoProperty::Set&,KoProperty::Property&)),
            this, SLOT(slotPropertyChanged(KoProperty::Set&,KoProperty::Property&)));
	    
    m_controlSource->setListData(m_reportDesigner->fieldKeys(), m_reportDesigner->fieldNames());
    setZValue(Z);
}

KoReportDesignerItemMaps::KoReportDesignerItemMaps(KoReportDesigner * rw, QGraphicsScene* scene, const QPointF &pos)
        : KoReportDesignerItemRectBase(rw)
{
    Q_UNUSED(pos);
    myDebug() << "\e[35m======KoReportDesigner\e[0m";
    init(scene, rw);
    setSceneRect(properRect(*rw, KOREPORT_ITEM_RECT_DEFAULT_WIDTH, KOREPORT_ITEM_RECT_DEFAULT_WIDTH));
    m_name->setValue(m_reportDesigner->suggestEntityName(typeName()));
}

KoReportDesignerItemMaps::KoReportDesignerItemMaps(QDomNode & element, KoReportDesigner * rw, QGraphicsScene* scene)
        : KoReportItemMaps(element), KoReportDesignerItemRectBase(rw)
{
    myDebug() << "\e[35m======QDomNode\e[0m";
    init(scene, rw);
    setSceneRect(m_pos.toScene(), m_size.toScene());
}

KoReportDesignerItemMaps* KoReportDesignerItemMaps::clone()
{
    QDomDocument d;
    QDomElement e = d.createElement("clone");;
    QDomNode n;
    buildXML(d, e);
    n = e.firstChild();
    return new KoReportDesignerItemMaps(n, designer(), 0);
}

KoReportDesignerItemMaps::~KoReportDesignerItemMaps()
{
    // do we need to clean anything up?
}

void KoReportDesignerItemMaps::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    myDebug() << "\e[35m======Paint\e[0m";
    // store any values we plan on changing so we can restore them
    QPen  p = painter->pen();
    painter->fillRect(rect(), QColor(0xc2, 0xfc, 0xc7));//C2FCC7
    
    //Draw a border so user knows the object edge
    painter->setPen(QPen(QColor(224, 224, 224)));
    painter->drawRect(rect());
    painter->setPen(Qt::black);
    painter->drawText(rect(), 0, dataSourceAndObjectTypeName(itemDataSource(), "map"));
    
    drawHandles(painter);

    // restore an values before we started just in case
    painter->setPen(p);
}

void KoReportDesignerItemMaps::buildXML(QDomDocument & doc, QDomElement & parent)
{
    myDebug() << "\e[35m====== BUILDING XML \e[0m";
    QDomElement entity = doc.createElement(QLatin1String("report:") + typeName());

    // properties
    addPropertyAsAttribute(&entity, m_name);
    addPropertyAsAttribute(&entity, m_controlSource);
    addPropertyAsAttribute(&entity, m_latitudeProperty);
    addPropertyAsAttribute(&entity, m_longitudeProperty);
    addPropertyAsAttribute(&entity, m_zoomProperty);
    //addPropertyAsAttribute(&entity, m_resizeMode);
    entity.setAttribute("report:z-index", zValue());
    buildXMLRect(doc, entity, &m_pos, &m_size);

    parent.appendChild(entity);
}

void KoReportDesignerItemMaps::slotPropertyChanged(KoProperty::Set &s, KoProperty::Property &p)
{
    myDebug() << p.name() << ":" << p.value();
    if (p.name() == "Name") {
        //For some reason p.oldValue returns an empty string
        if (!m_reportDesigner->isEntityNameUnique(p.value().toString(), this)) {
            p.setValue(m_oldName);
        } else {
            m_oldName = p.value().toString();
        }
    }

    KoReportDesignerItemRectBase::propertyChanged(s, p);
    if (m_reportDesigner) m_reportDesigner->setModified(true);
}

void KoReportDesignerItemMaps::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    m_controlSource->setListData(m_reportDesigner->fieldKeys(), m_reportDesigner->fieldNames());
    KoReportDesignerItemRectBase::mousePressEvent(event);
}
