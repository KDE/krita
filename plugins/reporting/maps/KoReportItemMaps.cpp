/*
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
#include "KoReportItemMaps.h"
#include <koproperty/Property.h>
#include <koproperty/Set.h>
#include <kdebug.h>
#include <klocalizedstring.h>
#include <QBuffer>
#include <kcodecs.h>
#include <renderobjects.h>
#include <QPixmap>
#include <sys/socket.h>
#include <QLabel>
#include <QStringList>



#define myDebug() if (0) kDebug(44021)


KoReportItemMaps::KoReportItemMaps(QDomNode & element)
    : m_longtitude(0)
    , m_latitude(0)
    , m_zoom(0)
    , m_pageId(0)
    , m_sectionId(0)
    , m_oroPicture(0)
{
    createProperties();
    QDomNodeList nl = element.childNodes();
    QString n;
    QDomNode node;

    m_name->setValue(element.toElement().attribute("report:name"));
    m_controlSource->setValue(element.toElement().attribute("report:item-data-source"));
    Z = element.toElement().attribute("report:z-index").toDouble();

    parseReportRect(element.toElement(), &m_pos, &m_size);
    for (int i = 0; i < nl.count(); i++) {
        node = nl.item(i);
        n = node.nodeName();
    }
}


KoReportItemMaps::~KoReportItemMaps()
{
    delete m_set;
}

void KoReportItemMaps::createProperties()
{
    m_set = new KoProperty::Set(0, "Maps");

    m_controlSource = new KoProperty::Property("item-data-source", QStringList(), QStringList(), QString(), i18n("Data Source"));

    addDefaultProperties();
    m_set->addProperty(m_controlSource);
}


void KoReportItemMaps::setColumn(const QString &c)
{
    m_controlSource->setValue(c);
}

QString KoReportItemMaps::itemDataSource() const
{
    kDebug() << m_controlSource->value().toString();
    return m_controlSource->value().toString();
}

QString KoReportItemMaps::typeName() const
{
    return "maps";
}

int KoReportItemMaps::renderSimpleData(OROPage *page, OROSection *section, const QPointF &offset,
                                       const QVariant &data, KRScriptHandler *script)
{
    Q_UNUSED(script)
    
    myDebug() << this << "data:" << data;
    deserializeData(data);
    m_pageId = page;
    m_sectionId = section;
    m_offset = offset;


    m_oroPicture = new OROPicture();
    m_oroPicture->setPosition(m_pos.toScene() + m_offset);
    m_oroPicture->setSize(m_size.toScene());

    if (m_pageId) {
        m_pageId->addPrimitive(m_oroPicture);
    }

    if (m_sectionId) {
        OROPicture *i2 = dynamic_cast<OROPicture*>(m_oroPicture->clone());
        i2->setPosition(m_pos.toPoint());
    }

    m_mapRenderer.renderJob(this);

    return 0; //Item doesn't stretch the section height
}


void KoReportItemMaps::deserializeData(const QVariant& serialized)
{
    QStringList dataList = serialized.toString().split(QLatin1Char(';'));
    m_latitude = dataList[0].toDouble();
    m_longtitude = dataList[1].toDouble();
    m_zoom = dataList[2].toInt();
}

void KoReportItemMaps::renderFinished()
{
    myDebug() << m_pageId << ", " << m_sectionId;
    emit finishedRendering();
}

OROPicture* KoReportItemMaps::oroImage()
{
    return m_oroPicture;
}



qreal KoReportItemMaps::longtitude() const
{
    return m_longtitude;
}

qreal KoReportItemMaps::latitude() const
{
    return m_latitude;
}

int KoReportItemMaps::zoom() const
{
    return m_zoom;
}

QSize KoReportItemMaps::size() const
{
    return m_size.toScene().toSize();
}
