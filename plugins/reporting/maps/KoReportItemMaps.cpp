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
#include "KoReportItemMaps.h"
#include <koproperty/Property.h>
#include <koproperty/Set.h>
#include <KoGlobal.h>
#include <kdebug.h>
#include <klocalizedstring.h>
#include <kglobalsettings.h>
#include <QBuffer>
#include <kcodecs.h>
#include <renderobjects.h>
#include <MarbleWidget.h>
#include <MarbleModel.h>
#include <QImage>
#include <QPixmap>

#define myDebug() kDebug(44021) << "\e[35m=="

KoReportItemMaps::KoReportItemMaps(QDomNode & element)
{
    myDebug() << "\e[35m======";
    createProperties();
    QDomNodeList nl = element.childNodes();
    QString n;
    QDomNode node;

    m_name->setValue(element.toElement().attribute("report:name"));
    //m_controlSource->setValue(element.toElement().attribute("report:Maps-data-source"));
    //m_resizeMode->setValue(element.toElement().attribute("report:resize-mode", "stretch"));
    Z = element.toElement().attribute("report:z-index").toDouble();

    parseReportRect(element.toElement(), &m_pos, &m_size);
    myDebug() << "\e[35m====== childgren:";
    for (int i = 0; i < nl.count(); i++) {
        node = nl.item(i);
        n = node.nodeName();

//         if (n == "report:Maps-data") {
// 
//             setInlineImageData(node.firstChild().nodeValue().toLatin1());
//         } else {
            myDebug() << "\e[35m====== while parsing image element encountered unknow element: " << n;
//         }
    }
    
    initMarble();
    
}

void KoReportItemMaps::initMarble()
{
    m_marble = new Marble::MarbleWidget();
    m_marble->setMapThemeId("earth/srtm/srtm.dgml");
    m_marble->centerOn(52,20.8, false);
    m_marble->zoomView(1800);
    m_marble->setShowOverviewMap(false);
    m_marble->setFixedSize(m_size.toScene().toSize());
    m_mapImage = new QImage(m_size.toScene().toSize(), QImage::Format_ARGB32);
    m_mapImage->fill(QColor(200, 150, 5).rgb());
    
//     connect(m_marble->model()->d)
}


KoReportItemMaps::~KoReportItemMaps()
{
    delete m_set;
    delete m_marble;
}

// bool KoReportItemMaps::isInline() const
// {
//     return !(inlineImageData().isEmpty());
// }

// QByteArray KoReportItemMaps::inlineImageData() const
// {
//     QPixmap pixmap = m_staticImage->value().value<QPixmap>();
//     QByteArray ba;
//     QBuffer buffer(&ba);
//     buffer.open(QIODevice::ReadWrite);
//     pixmap.save(&buffer, "PNG");   // writes pixmap into ba in PNG format,
//     //TODO should i remember the format used, or save as PNG as its lossless?
// 
//     QByteArray imageEncoded(KCodecs::base64Encode(buffer.buffer(), true));
//     return imageEncoded;
// }

// void KoReportItemMaps::setInlineImageData(QByteArray dat, const QString &fn)
// {
//     //oryginal image function
//     if (!fn.isEmpty()) {
//         QPixmap pix(fn);
//         if (!pix.isNull())
//             m_staticImage->setValue(pix);
//         else {
//             QPixmap blank(1, 1);
//             blank.fill();
//             m_staticImage->setValue(blank);
//         }
//     } else {
//         const QByteArray binaryStream(KCodecs::base64Decode(dat));
//         const QPixmap pix(QPixmap::fromImage(QImage::fromData(binaryStream), Qt::ColorOnly));
//         m_staticImage->setValue(pix);
//     }
// 
// }

// QString KoReportItemMaps::mode() const
// {
//     return m_resizeMode->value().toString();
// }

// void KoReportItemMaps::setMode(const QString &m)
// {
//     if (mode() != m) {
//         m_resizeMode->setValue(m);
//     }
// }

void KoReportItemMaps::createProperties()
{
    m_set = new KoProperty::Set(0, "Maps");

    m_controlSource = new KoProperty::Property("item-data-source", QStringList(), QStringList(), QString(), i18n("Data Source"));

    //QStringList keys, strings;
    //keys << "clip" << "stretch";
    //strings << i18n("Clip") << i18n("Stretch");
    //m_resizeMode = new KoProperty::Property("resize-mode", keys, strings, "clip", i18n("Resize Mode"));

    //m_staticImage = new KoProperty::Property("static-image", QPixmap(), i18n("Static Image"));

    addDefaultProperties();
    m_set->addProperty(m_controlSource);
    //m_set->addProperty(m_resizeMode);
    //m_set->addProperty(m_staticImage);
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
    return "report:maps";
}

int KoReportItemMaps::render(OROPage* page, OROSection* section,  QPointF offset, QVariant data, KRScriptHandler *script)
{
    Q_UNUSED(script)
    
    ///@todo Remove this in real code
    Q_UNUSED(data) 
    
    myDebug() << "Render";
    
    //QPainter painter(m_mapImage);
    m_marble->render(m_mapImage);

    /*QString uudata;
    QByteArray imgdata;
    if (!isInline()) {
        imgdata = data.toByteArray();
    } else {
        uudata = inlineImageData();
        imgdata = KCodecs::base64Decode(uudata.toLatin1());
    }*/

    //QImage img;
    //img.loadFromData(imgdata);
    
    OROImage * id = new OROImage();
    id->setImage(*m_mapImage);
    //if (mode().toLower() == "stretch") {
        id->setScaled(false);
        //id->setAspectRatioMode(Qt::KeepAspectRatio);
        //id->setTransformationMode(Qt::SmoothTransformation);
    //}

    id->setPosition(m_pos.toScene() + offset);
    id->setSize(m_size.toScene());
    if (page) {
        page->addPrimitive(id);
    }
    
    if (section) {
        OROImage *i2 = dynamic_cast<OROImage*>(id->clone());
        i2->setPosition(m_pos.toPoint());
        section->addPrimitive(i2);
    }
    
    if (!page) {
        delete id;
    }
    
    return 0; //Item doesnt stretch the section height
}


