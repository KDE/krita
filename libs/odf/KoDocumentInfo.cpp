/* This file is part of the KDE project
   Copyright (C) 1998, 1999, 2000 Torben Weis <weis@kde.org>
   Copyright (C) 2004 David Faure <faure@kde.org>

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

#include "KoDocumentInfo.h"

#include "KoDocumentBase.h"
#include "KoOdfWriteStore.h"
#include "KoXmlNS.h"

#include <QDateTime>
#include <KoStoreDevice.h>
#include <KoXmlWriter.h>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <OdfDebug.h>
#include <klocalizedstring.h>
#include <kuser.h>
#include <kemailsettings.h>

#include <CalligraVersionWrapper.h>


KoDocumentInfo::KoDocumentInfo(QObject *parent) : QObject(parent)
{
    m_aboutTags << "title" << "description" << "subject" << "comments"
    << "keyword" << "initial-creator" << "editing-cycles"
    << "date" << "creation-date" << "language";

    m_authorTags << "creator" << "initial" << "author-title"
    << "email" << "telephone" << "telephone-work"
    << "fax" << "country" << "postal-code" << "city"
    << "street" << "position" << "company";

    setAboutInfo("editing-cycles", "0");
    setAboutInfo("initial-creator", i18n("Unknown"));
    setAboutInfo("creation-date", QDateTime::currentDateTime()
                 .toString(Qt::ISODate));
}

KoDocumentInfo::~KoDocumentInfo()
{
}

bool KoDocumentInfo::load(const KoXmlDocument &doc)
{
    m_authorInfo.clear();

    if (!loadAboutInfo(doc.documentElement()))
        return false;

    if (!loadAuthorInfo(doc.documentElement()))
        return false;

    return true;
}

bool KoDocumentInfo::loadOasis(const KoXmlDocument &metaDoc)
{
    m_authorInfo.clear();

    KoXmlNode t = KoXml::namedItemNS(metaDoc, KoXmlNS::office, "document-meta");
    KoXmlNode office = KoXml::namedItemNS(t, KoXmlNS::office, "meta");

    if (office.isNull())
        return false;

    if (!loadOasisAboutInfo(office))
        return false;

    if (!loadOasisAuthorInfo(office))
        return false;

    return true;
}

QDomDocument KoDocumentInfo::save(QDomDocument &doc)
{
    updateParametersAndBumpNumCycles();

    QDomElement s = saveAboutInfo(doc);
    if (!s.isNull())
        doc.documentElement().appendChild(s);

    s = saveAuthorInfo(doc);
    if (!s.isNull())
        doc.documentElement().appendChild(s);


    if (doc.documentElement().isNull())
        return QDomDocument();

    return doc;
}

bool KoDocumentInfo::saveOasis(KoStore *store)
{
    updateParametersAndBumpNumCycles();

    KoStoreDevice dev(store);
    KoXmlWriter* xmlWriter = KoOdfWriteStore::createOasisXmlWriter(&dev,
                             "office:document-meta");
    xmlWriter->startElement("office:meta");

    xmlWriter->startElement("meta:generator");
    xmlWriter->addTextNode(QString("Calligra/%1")
                           .arg(CalligraVersionWrapper::versionString()));
    xmlWriter->endElement();

    if (!saveOasisAboutInfo(*xmlWriter))
        return false;
    if (!saveOasisAuthorInfo(*xmlWriter))
        return false;

    xmlWriter->endElement();
    xmlWriter->endElement(); // root element
    xmlWriter->endDocument();
    delete xmlWriter;
    return true;
}

void KoDocumentInfo::setAuthorInfo(const QString &info, const QString &data)
{
    if (!m_authorTags.contains(info)) {
        return;
    }

    m_authorInfoOverride.insert(info, data);
}

void KoDocumentInfo::setActiveAuthorInfo(const QString &info, const QString &data)
{
    if (!m_authorTags.contains(info)) {
        return;
    }

    if (data.isEmpty()) {
        m_authorInfo.remove(info);
    } else {
        m_authorInfo.insert(info, data);
    }
    emit infoUpdated(info, data);
}

QString KoDocumentInfo::authorInfo(const QString &info) const
{
    if (!m_authorTags.contains(info))
        return QString();

    return m_authorInfo[ info ];
}

void KoDocumentInfo::setAboutInfo(const QString &info, const QString &data)
{
    if (!m_aboutTags.contains(info))
        return;

    m_aboutInfo.insert(info, data);
    emit infoUpdated(info, data);
}

QString KoDocumentInfo::aboutInfo(const QString &info) const
{
    if (!m_aboutTags.contains(info)) {
        return QString();
    }

    return m_aboutInfo[info];
}

bool KoDocumentInfo::saveOasisAuthorInfo(KoXmlWriter &xmlWriter)
{
    Q_FOREACH (const QString & tag, m_authorTags) {
        if (!authorInfo(tag).isEmpty() && tag == "creator") {
            xmlWriter.startElement("dc:creator");
            xmlWriter.addTextNode(authorInfo("creator"));
            xmlWriter.endElement();
        } else if (!authorInfo(tag).isEmpty()) {
            xmlWriter.startElement("meta:user-defined");
            xmlWriter.addAttribute("meta:name", tag);
            xmlWriter.addTextNode(authorInfo(tag));
            xmlWriter.endElement();
        }
    }

    return true;
}

bool KoDocumentInfo::loadOasisAuthorInfo(const KoXmlNode &metaDoc)
{
    KoXmlElement e = KoXml::namedItemNS(metaDoc, KoXmlNS::dc, "creator");
    if (!e.isNull() && !e.text().isEmpty())
        setActiveAuthorInfo("creator", e.text());

    KoXmlNode n = metaDoc.firstChild();
    for (; !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;

        KoXmlElement e = n.toElement();
        if (!(e.namespaceURI() == KoXmlNS::meta &&
                e.localName() == "user-defined" && !e.text().isEmpty()))
            continue;

        QString name = e.attributeNS(KoXmlNS::meta, "name", QString());
        setActiveAuthorInfo(name, e.text());
    }

    return true;
}

bool KoDocumentInfo::loadAuthorInfo(const KoXmlElement &e)
{
    KoXmlNode n = e.namedItem("author").firstChild();
    for (; !n.isNull(); n = n.nextSibling()) {
        KoXmlElement e = n.toElement();
        if (e.isNull())
            continue;

        if (e.tagName() == "full-name")
            setActiveAuthorInfo("creator", e.text().trimmed());
        else
            setActiveAuthorInfo(e.tagName(), e.text().trimmed());
    }

    return true;
}

QDomElement KoDocumentInfo::saveAuthorInfo(QDomDocument &doc)
{
    QDomElement e = doc.createElement("author");
    QDomElement t;

    Q_FOREACH (const QString &tag, m_authorTags) {
        if (tag == "creator")
            t = doc.createElement("full-name");
        else
            t = doc.createElement(tag);

        e.appendChild(t);
        t.appendChild(doc.createTextNode(authorInfo(tag)));
    }

    return e;
}

bool KoDocumentInfo::saveOasisAboutInfo(KoXmlWriter &xmlWriter)
{
    Q_FOREACH (const QString &tag, m_aboutTags) {
        if (!aboutInfo(tag).isEmpty() || tag == "title") {
            if (tag == "keyword") {
                Q_FOREACH (const QString & tmp, aboutInfo("keyword").split(';')) {
                    xmlWriter.startElement("meta:keyword");
                    xmlWriter.addTextNode(tmp);
                    xmlWriter.endElement();
                }
            } else if (tag == "title" || tag == "description" || tag == "subject" ||
                       tag == "date" || tag == "language") {
                QByteArray elementName(QString("dc:" + tag).toLatin1());
                xmlWriter.startElement(elementName.constData());
                xmlWriter.addTextNode(aboutInfo(tag));
                xmlWriter.endElement();
            } else {
                QByteArray elementName(QString("meta:" + tag).toLatin1());
                xmlWriter.startElement(elementName.constData());
                xmlWriter.addTextNode(aboutInfo(tag));
                xmlWriter.endElement();
            }
        }
    }

    return true;
}

bool KoDocumentInfo::loadOasisAboutInfo(const KoXmlNode &metaDoc)
{
    QStringList keywords;
    KoXmlElement e;
    forEachElement(e, metaDoc) {
        QString tag(e.localName());
        if (! m_aboutTags.contains(tag) && tag != "generator") {
            continue;
        }

        //debugOdf<<"localName="<<e.localName();
        if (tag == "keyword") {
            if (!e.text().isEmpty())
                keywords << e.text().trimmed();
        } else if (tag == "description") {
            //this is the odf way but add meta:comment if is already loaded
            KoXmlElement e  = KoXml::namedItemNS(metaDoc, KoXmlNS::dc, tag);
            if (!e.isNull() && !e.text().isEmpty())
                setAboutInfo("description", aboutInfo("description") + e.text().trimmed());
        } else if (tag == "comments") {
            //this was the old way so add it to dc:description
            KoXmlElement e  = KoXml::namedItemNS(metaDoc, KoXmlNS::meta, tag);
            if (!e.isNull() && !e.text().isEmpty())
                setAboutInfo("description", aboutInfo("description") + e.text().trimmed());
        } else if (tag == "title"|| tag == "subject"
                   || tag == "date" || tag == "language") {
            KoXmlElement e  = KoXml::namedItemNS(metaDoc, KoXmlNS::dc, tag);
            if (!e.isNull() && !e.text().isEmpty())
                setAboutInfo(tag, e.text().trimmed());
        } else if (tag == "generator") {
            setOriginalGenerator(e.text().trimmed());
        } else {
            KoXmlElement e  = KoXml::namedItemNS(metaDoc, KoXmlNS::meta, tag);
            if (!e.isNull() && !e.text().isEmpty())
                setAboutInfo(tag, e.text().trimmed());
        }
    }

    if (keywords.count() > 0) {
        setAboutInfo("keyword", keywords.join(", "));
    }

    return true;
}

bool KoDocumentInfo::loadAboutInfo(const KoXmlElement &e)
{
    KoXmlNode n = e.namedItem("about").firstChild();
    KoXmlElement tmp;
    for (; !n.isNull(); n = n.nextSibling()) {
        tmp = n.toElement();
        if (tmp.isNull())
            continue;

        if (tmp.tagName() == "abstract")
            setAboutInfo("comments", tmp.text());

        setAboutInfo(tmp.tagName(), tmp.text());
    }

    return true;
}

QDomElement KoDocumentInfo::saveAboutInfo(QDomDocument &doc)
{
    QDomElement e = doc.createElement("about");
    QDomElement t;

    Q_FOREACH (const QString &tag, m_aboutTags) {
        if (tag == "comments") {
            t = doc.createElement("abstract");
            e.appendChild(t);
            t.appendChild(doc.createCDATASection(aboutInfo(tag)));
        } else {
            t = doc.createElement(tag);
            e.appendChild(t);
            t.appendChild(doc.createTextNode(aboutInfo(tag)));
        }
    }

    return e;
}

void KoDocumentInfo::updateParametersAndBumpNumCycles()
{
    KoDocumentBase *doc = dynamic_cast< KoDocumentBase *>(parent());
    if (doc && doc->isAutosaving()) {
        return;
    }

    setAboutInfo("editing-cycles", QString::number(aboutInfo("editing-cycles").toInt() + 1));
    setAboutInfo("date", QDateTime::currentDateTime().toString(Qt::ISODate));

    updateParameters();
}

void KoDocumentInfo::updateParameters()
{
    KoDocumentBase *doc = dynamic_cast< KoDocumentBase *>(parent());
    if (doc && (!doc->isModified() && !doc->isEmpty())) {
        return;
    }

    KConfig config("kritarc");
    config.reparseConfiguration();
    KConfigGroup authorGroup(&config, "Author");
    QStringList profiles = authorGroup.readEntry("profile-names", QStringList());

    config.reparseConfiguration();
    KConfigGroup appAuthorGroup(&config, "Author");
    QString profile = appAuthorGroup.readEntry("active-profile", "");

    if (profiles.contains(profile)) {
        KConfigGroup cgs(&authorGroup, "Author-" + profile);
        setActiveAuthorInfo("creator", cgs.readEntry("creator"));
        setActiveAuthorInfo("initial", cgs.readEntry("initial"));
        setActiveAuthorInfo("author-title", cgs.readEntry("author-title"));
        setActiveAuthorInfo("email", cgs.readEntry("email"));
        setActiveAuthorInfo("telephone", cgs.readEntry("telephone"));
        setActiveAuthorInfo("telephone-work", cgs.readEntry("telephone-work"));
        setActiveAuthorInfo("fax", cgs.readEntry("fax"));
        setActiveAuthorInfo("country",cgs.readEntry("country"));
        setActiveAuthorInfo("postal-code",cgs.readEntry("postal-code"));
        setActiveAuthorInfo("city", cgs.readEntry("city"));
        setActiveAuthorInfo("street", cgs.readEntry("street"));
        setActiveAuthorInfo("position", cgs.readEntry("position"));
        setActiveAuthorInfo("company", cgs.readEntry("company"));
    } else {
        if (profile == "anonymous") {
            setActiveAuthorInfo("creator", QString());
            setActiveAuthorInfo("telephone", QString());
            setActiveAuthorInfo("telephone-work", QString());
            setActiveAuthorInfo("email", QString());
        } else {
            KUser user(KUser::UseRealUserID);
            setActiveAuthorInfo("creator", user.property(KUser::FullName).toString());
            setActiveAuthorInfo("telephone-work", user.property(KUser::WorkPhone).toString());
            setActiveAuthorInfo("telephone", user.property(KUser::HomePhone).toString());
            KEMailSettings eMailSettings;
            setActiveAuthorInfo("email", eMailSettings.getSetting(KEMailSettings::EmailAddress));
        }
        setActiveAuthorInfo("initial", "");
        setActiveAuthorInfo("author-title", "");
        setActiveAuthorInfo("fax", "");
        setActiveAuthorInfo("country", "");
        setActiveAuthorInfo("postal-code", "");
        setActiveAuthorInfo("city", "");
        setActiveAuthorInfo("street", "");
        setActiveAuthorInfo("position", "");
        setActiveAuthorInfo("company", "");
    }

    //alllow author info set programatically to override info from author profile
    Q_FOREACH (const QString &tag, m_authorTags) {
        if (m_authorInfoOverride.contains(tag)) {
            setActiveAuthorInfo(tag, m_authorInfoOverride.value(tag));
        }
    }
}

void KoDocumentInfo::resetMetaData()
{
    setAboutInfo("editing-cycles", QString::number(0));
    setAboutInfo("initial-creator", authorInfo("creator"));
    setAboutInfo("creation-date", QDateTime::currentDateTime().toString(Qt::ISODate));
}

QString KoDocumentInfo::originalGenerator() const
{
    return m_generator;
}

void KoDocumentInfo::setOriginalGenerator(const QString &generator)
{
    m_generator = generator;
}
