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
#include "KoReportItemweb.h"
#include <koproperty/Property.h>
#include <koproperty/Set.h>
#include <KoGlobal.h>
#include <kdebug.h>
#include <klocalizedstring.h>
#include <kglobalsettings.h>
#include <QBuffer>
#include <kcodecs.h>
#include <renderobjects.h>

KoReportItemChart::KoReportItemChart()
{
    //m_reportData = 0;
    createProperties();
}

KoReportItemweb::KoReportItemweb(QDomNode & element)
{
    createProperties();
    QDomNodeList nl = element.childNodes();
    QString n;
    QDomNode node;
 QDomElement e = element.toElement();
    m_url->setValue(e.attribute("report:address"));

    m_name->setValue(element.toElement().attribute("report:name"));
 //   m_controlSource->setValue(element.toElement().attribute("report:web-data-source"));
 //   m_resizeMode->setValue(element.toElement().attribute("report:resize-mode", "stretch"));
 //   Z = element.toElement().attribute("report:z-index").toDouble();

    parseReportRect(element.toElement(), &m_pos, &m_size);

   /* for (int i = 0; i < nl.count(); i++) {
        node = nl.item(i);
        n = node.nodeName();

        if (n == "report:web-data") {

            setInlineImageData(node.firstChild().nodeValue().toLatin1());
        } else {
            kDebug() << "while parsing image element encountered unknow element: " << n;
        }
    }*/

}

KoReportItemweb::~KoReportItemweb()
{
    delete m_set;
}

/*bool KoReportItemweb::isInline() const
{
    return !(inlineImageData().isEmpty());
}*/

QByteArray KoReportItemweb::inlineImageData() const
{
    QPixmap pixmap = m_staticImage->value().value<QPixmap>();
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::ReadWrite);
    pixmap.save(&buffer, "PNG");   // writes pixmap into ba in PNG format,
    //TODO should i remember the format used, or save as PNG as its lossless?

    QByteArray imageEncoded(KCodecs::base64Encode(buffer.buffer(), true));
    return imageEncoded;
}

void KoReportItemweb::setInlineImageData(QByteArray dat, const QString &fn)
{
    if (!fn.isEmpty()) {
        QPixmap pix(fn);
        if (!pix.isNull())
            m_staticImage->setValue(pix);
        else {
            QPixmap blank(1, 1);
            blank.fill();
            m_staticImage->setValue(blank);
        }
    } else {
        const QByteArray binaryStream(KCodecs::base64Decode(dat));
        const QPixmap pix(QPixmap::fromImage(QImage::fromData(binaryStream), Qt::ColorOnly));
        m_staticImage->setValue(pix);
    }

}

QString KoReportItemweb::mode() const
{
    return m_resizeMode->value().toString();
}

void KoReportItemweb::setMode(const QString &m)
{
    if (mode() != m) {
        m_resizeMode->setValue(m);
    }
}

void KoReportItemweb::createProperties()
{
    m_set = new KoProperty::Set(0, "Image");

    m_controlSource = new KoProperty::Property("item-data-source", QStringList(), QStringList(), QString(), i18n("Data Source"));

    QStringList keys, strings;
    keys << "clip" << "stretch";
    strings << i18n("Clip") << i18n("Stretch");
    m_resizeMode = new KoProperty::Property("resize-mode", keys, strings, "clip", i18n("Resize Mode"));

    m_staticImage = new KoProperty::Property("static-image", QPixmap(), i18n("Static Image"));

    addDefaultProperties();
    m_set->addProperty(m_controlSource);
    m_set->addProperty(m_resizeMode);
    m_set->addProperty(m_staticImage);
}


void KoReportItemweb::setColumn(const QString &c)
{
    m_controlSource->setValue(c);
}

QString KoReportItemweb::itemDataSource() const
{
    return m_controlSource->value().toString();
}

QString KoReportItemweb::typeName() const
{
    return "report:image";
}

int KoReportItemweb::render(OROPage* page, OROSection* section,  QPointF offset, QVariant data, KRScriptHandler *script)
{
    Q_UNUSED(script)

    QString uudata;
    QByteArray imgdata;
    if (!isInline()) {
        imgdata = data.toByteArray();
    } else {
        uudata = inlineImageData();
        imgdata = KCodecs::base64Decode(uudata.toLatin1());
    }

    QImage img;
    img.loadFromData(imgdata);
    OROImage * id = new OROImage();
    id->setImage(img);
    if (mode().toLower() == "stretch") {
        id->setScaled(true);
        id->setAspectRatioMode(Qt::KeepAspectRatio);
        id->setTransformationMode(Qt::SmoothTransformation);
    }

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



