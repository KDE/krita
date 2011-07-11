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

#include "KoReportDesignerItemweb.h"
#include <KoReportDesignerItemBase.h>
#include <KoReportDesigner.h>

#include <QImageWriter>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QBuffer>
#include <kcodecs.h>
#include <qdom.h>
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

void KoReportDesignerItemweb::init(QGraphicsScene * scene)			//done,compared,add function if necessary
{
    kDebug();
    if (scene)
        scene->addItem(this);

       connect(m_set, SIGNAL(propertyChanged(KoProperty::Set&, KoProperty::Property&)),
            this, SLOT(slotPropertyChanged(KoProperty::Set&, KoProperty::Property&)));
     KoReportDesignerItemRectBase::init(&m_pos, &m_size, m_set);	    
    
 //m_controlSource->setListData(m_reportDesigner->fieldKeys(), m_reportDesigner->fieldNames());
    setZValue(Z);
}

KoReportDesignerItemweb::KoReportDesignerItemweb(KoReportDesigner * rw, QGraphicsScene* scene, const QPointF &pos)    //done,compared
        : KoReportDesignerItemRectBase(rw)
{
  kDebug();
    init(scene);
    m_size.setSceneSize(QSizeF(100, 100));
    m_pos.setScenePos(pos);
    m_name->setValue(m_reportDesigner->suggestEntityName("webbrowser"));
}

KoReportDesignerItemweb::KoReportDesignerItemweb(QDomNode & element, KoReportDesigner * rw, QGraphicsScene* scene)    //done,compared
        : KoReportItemweb(element), KoReportDesignerItemRectBase(rw)
{
    init(scene);
    setSceneRect(m_pos.toScene(), m_size.toScene());
}

KoReportDesignerItemweb* KoReportDesignerItemweb::clone()								 //done,compared
{
    QDomDocument d;
    QDomElement e = d.createElement("clone");;
    QDomNode n;
    buildXML(d, e);
    n = e.firstChild();
    return new KoReportDesignerItemweb(n, designer(), 0);
}

KoReportDesignerItemweb::~KoReportDesignerItemweb()									 //done,compared
{
    // do we need to clean anything up?
}

//i do not need paint function as i am not displaying my image as a picture,it is a qwebview

/*void KoReportDesignerItemweb::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // store any values we plan on changing so we can restore them
    QPen  p = painter->pen();
  painter->fillRect(rect(), QColor(0xc2, 0xfc, 0xc7));//C2FCC7

    if (isInline()) {
        //QImage t_img = _image;
        QImage t_img = m_staticImage->value().value<QPixmap>().toImage();
        if (mode() == "stretch") {
            t_img = t_img.scaled(rect().width(), rect().height(), Qt::KeepAspectRatio);
        }
        painter->drawImage(rect().left(), rect().top(), t_img, 0, 0, rect().width(), rect().height());
    } else {
        painter->drawText(rect(), 0, dataSourceAndObjectTypeName(itemDataSource(), "image"));
    }

    //Draw a border so user knows the object edge
    painter->setPen(QPen(QColor(224, 224, 224)));
    painter->drawRect(rect());


    drawHandles(painter);

    // restore an values before we started just in case
    painter->setPen(p);
}*/

void KoReportDesignerItemweb::buildXML(QDomDocument & doc, QDomElement & parent)
{
    QDomElement entity = doc.createElement("report:image");

    // properties
    addPropertyAsAttribute(&entity, m_url);
    addPropertyAsAttribute(&entity, m_dataSource);
    entity.setAttribute("report:z-index", zValue());
    buildXMLRect(doc, entity, &m_pos, &m_size);


   /* if (isInline()) {
        QDomElement web = doc.createElement("report:inline-image-data");
        web.appendChild(doc.createTextNode(inlineImageData()));
        entity.appendChild(web);
    } else {
        addPropertyAsAttribute(&entity, m_controlSource);
    }*/

    parent.appendChild(entity);
}

void KoReportDesignerItemweb::slotPropertyChanged(KoProperty::Set &s, KoProperty::Property &p)
{
    if (p.name() == "Name") {
        //For some reason p.oldValue returns an empty string
        if (!m_reportDesigner->isEntityNameUnique(p.value().toString(), this)) {
            p.setValue(m_oldName);
        } else {
            m_oldName = p.value().toString();
        }
    }else if(p.name=="url"){

	setUrl(p.valur.toString());
} else if (p.name() == "data-source") {
    void setDataSource(const QString &ds);

    KoReportDesignerItemRectBase::propertyChanged(s, p);
    if (m_reportDesigner) m_reportDesigner->setModified(true);
}

void KoReportDesignerItemweb::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    m_controlSource->setListData(m_reportDesigner->fieldKeys(), m_reportDesigner->fieldNames());
    KoReportDesignerItemRectBase::mousePressEvent(event);
}
