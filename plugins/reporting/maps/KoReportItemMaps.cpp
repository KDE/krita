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
    , m_zoom(1200)
    , m_pageId(0)
    , m_sectionId(0)
    , m_oroPicture(0)
{
    createProperties();

    m_name->setValue(element.toElement().attribute("report:name"));
    m_controlSource->setValue(element.toElement().attribute("report:item-data-source"));
    Z = element.toElement().attribute("report:z-index").toDouble();
    m_latitudeProperty->setValue(element.toElement().attribute("report:latitude").toDouble());
    m_longitudeProperty->setValue(element.toElement().attribute("report:longitude").toDouble());
    m_zoomProperty->setValue(element.toElement().attribute("report:zoom").toInt());
    QString themeId(element.toElement().attribute("report:theme"));
    themeId = themeId.isEmpty() ? m_themeManager.mapThemeIds()[0] : themeId;
    m_themeProperty->setValue(themeId);

    parseReportRect(element.toElement(), &m_pos, &m_size);
}


KoReportItemMaps::~KoReportItemMaps()
{
    delete m_set;
}

void KoReportItemMaps::createProperties()
{
    m_set = new KoProperty::Set(0, "Maps");

    m_controlSource = new KoProperty::Property("item-data-source", QStringList(), QStringList(), QString(), i18n("Data Source"));

    m_latitudeProperty = new KoProperty::Property("latitude", 0.0, i18n("Latitude"), i18n("Latitude"), KoProperty::Double);
    m_latitudeProperty->setOption("min", -90);
    m_latitudeProperty->setOption("max", 90);
    m_latitudeProperty->setOption("unit", QString::fromUtf8("°"));
    m_latitudeProperty->setOption("precision", 7);

    m_longitudeProperty = new KoProperty::Property("longitude", 0.0, i18n("Longitude"), i18n("Longitude"), KoProperty::Double);
    m_longitudeProperty->setOption("min", -180);
    m_longitudeProperty->setOption("max", 180);
    m_longitudeProperty->setOption("unit", QString::fromUtf8("°"));
    m_longitudeProperty->setOption("precision", 7);

    m_zoomProperty     = new KoProperty::Property("zoom", 1000, i18n("Zoom"), i18n("Zoom") );
    m_zoomProperty->setOption("min", 0);
    m_zoomProperty->setOption("max", 4000);
    m_zoomProperty->setOption("step", 100);
    m_zoomProperty->setOption("slider", true);

    QStringList mapThemIds(m_themeManager.mapThemeIds());
    m_themeProperty = new KoProperty::Property("theme",
                                                    mapThemIds,
                                                    mapThemIds,
                                                    mapThemIds[1]);

    if (mapThemIds.contains("earth/srtm/srtm.dgml")) {
        m_themeProperty->setValue("earth/srtm/srtm.dgml", false);
    }

    addDefaultProperties();
    m_set->addProperty(m_controlSource);
    m_set->addProperty(m_latitudeProperty);
    m_set->addProperty(m_longitudeProperty);
    m_set->addProperty(m_zoomProperty);
    m_set->addProperty(m_themeProperty);
}


void KoReportItemMaps::setColumn(const QString &c)
{
    m_controlSource->setValue(c);
}

QString KoReportItemMaps::itemDataSource() const
{
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
    if (dataList.size() == 3) {
        m_latitude = dataList[0].toDouble();
        m_longtitude = dataList[1].toDouble();
        m_zoom = dataList[2].toInt();
    } else {
        m_latitude = m_latitudeProperty->value().toReal();
        m_longtitude = m_longitudeProperty->value().toReal();
        m_zoom = m_zoomProperty->value().toInt();
    }
}

void KoReportItemMaps::renderFinished()
{
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

QString KoReportItemMaps::themeId() const
{
    return m_themeProperty->value().toString();
}
