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
#ifndef KRIMAGEDATA_H
#define KRIMAGEDATA_H
#include <KoReportASyncItemBase.h>
#include <QRect>
#include <QPainter>
#include <QDomDocument>
#include "krpos.h"
#include "krsize.h"
#include <koproperty/Property.h>
#include <koproperty/Set.h>
#include <kdebug.h>
#include <klocalizedstring.h>
#include <marble/MarbleWidget.h>
#include <QMap>
#include "MapRenderer.h"

class OROImage;
class OROPicture;
class OROPage;
class OROSection;

namespace Scripting
{
class Maps;
}

/**
 @author
*/
class KoReportItemMaps : public KoReportASyncItemBase
{
    Q_OBJECT
public:
    KoReportItemMaps() {
        createProperties();
    }
    explicit KoReportItemMaps(QDomNode &element);
    virtual ~KoReportItemMaps();

    virtual QString typeName() const;
    virtual int renderSimpleData(OROPage *page, OROSection *section, const QPointF &offset, const QVariant &data, KRScriptHandler *script);

    virtual QString itemDataSource() const;
    void renderFinished();

    qreal longtitude() const;
    qreal latitude() const;
    int zoom() const;

    QSize size() const;
    OROPicture* oroImage();

protected:
    KoProperty::Property * m_controlSource;
    KoProperty::Property* m_resizeMode;
    KoProperty::Property* m_staticImage;
    KoProperty::Property* m_latitudeProperty;
    KoProperty::Property* m_longitudeProperty;
    KoProperty::Property* m_zoomProperty;
//     KoProperty::Property* m_mapThemeIdProperty;

    void setMode(const QString&);
    void setInlineImageData(QByteArray, const QString& = QString());
    void setColumn(const QString&);
    QString mode() const;
    bool isInline() const;
    QByteArray inlineImageData() const;

    qreal m_longtitude;
    qreal m_latitude;
    int m_zoom;
    OROPage *m_pageId;
    OROSection *m_sectionId;
    QPointF m_offset;
    OROPicture * m_oroPicture;
    MapRenderer m_mapRenderer;

private:
    virtual void createProperties();
    void deserializeData(const QVariant& serialized);

    friend class Scripting::Maps;
};

#endif
