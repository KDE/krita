/* This file is part of the KDE project
   Copyright Shreya Pandit <shreya@shreyapandit.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoReportItemWeb.h"

#include <koproperty/Property.h>
#include <koproperty/Set.h>
#include <KoGlobal.h>

#include <kdebug.h>
#include <klocalizedstring.h>
#include <kglobalsettings.h>
#include <kcodecs.h>
#include <renderobjects.h>

#include <QGraphicsRectItem>
#include <QtWebKit>
#include <QtCore/QUrl>
#include <QWebPage>
#include <QtGui/QAction>
#include <QtGui/QWidget>
#include <QtGui/QApplication>

KoReportItemWeb::KoReportItemWeb(): m_loaded(false)
{
    createProperties();
    init();
}

KoReportItemWeb::KoReportItemWeb(QDomNode &element)
{
    createProperties();
    init();
    QDomNodeList nl = element.childNodes();
    QString n;
    QDomNode node;
    QDomElement e = element.toElement();
    url->setValue(e.attribute("report:url"));
    m_controlSource->setValue(element.toElement().attribute("report:item-data-source"));
    m_name->setValue(element.toElement().attribute("report:name"));
    Z = element.toElement().attribute("report:z-index").toDouble();
    parseReportRect(element.toElement(), &m_pos, &m_size);
    for (int i = 0; i < nl.count(); i++) {
        node = nl.item(i);
        n = node.nodeName();
    }
}

void KoReportItemWeb::init()
{
    m_webPage = new QWebPage();
    connect(m_webPage, SIGNAL(loadFinished(bool)),
            this, SLOT(loadFinished(bool)));
    //connect(m_webPage, SIGNAL(loadFinished(bool)),
    //        this, SLOT(render(OROPage*,OROSection*,QPointF,QVariant,KRScriptHandler)));
    //setUrl("http://www.kde.org");
    m_webImage = new QImage(m_size.toScene().toSize(), QImage::Format_ARGB32);
}

void KoReportItemWeb::createProperties()
{
    m_set = new KoProperty::Set(0, "web");
    url = new KoProperty::Property("url", QStringList(), QStringList(), QString(), i18n("Url"));
    m_controlSource = new KoProperty::Property("item-data-source", QStringList(), 
                                               QStringList(), QString(), i18n("Data Source"));
    m_set->addProperty(m_controlSource);
    addDefaultProperties();
    m_set->addProperty(m_controlSource);
    m_set->addProperty(url);

}

KoReportItemWeb::~KoReportItemWeb()
{
    delete m_set;
}
QString KoReportItemWeb::typeName() const
{
    return "web";
}

void KoReportItemWeb::setUrl(const QString &url)
{
    m_url = QUrl(url);
    m_webPage->mainFrame()->load(url);
}

void KoReportItemWeb::loadFinished(bool)
{
    m_loaded = true;
}

int KoReportItemWeb::render(OROPage *page, OROSection *section,  QPointF offset,
                            QVariant data, KRScriptHandler *script)
{
    Q_UNUSED(section);
    Q_UNUSED(data);
    Q_UNUSED(script);

    QPainter painter(m_webImage);
    m_webPage->mainFrame()->render(&painter);
    painter.end();
    OROImage *id = new OROImage();
    id->setImage(*m_webImage);
    id->setScaled(false);
    id->setPosition(m_pos.toScene() + offset);
    id->setSize(m_size.toScene());
    if (page) {
        page->addPrimitive(id);
    }


    if (!page) {
        delete id;
    }

    return 0; //Item doesnt stretch the section height
}

#include "KoReportItemWeb.moc"
