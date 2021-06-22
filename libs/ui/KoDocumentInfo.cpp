/* This file is part of the KDE project
   SPDX-FileCopyrightText: 1998, 1999, 2000 Torben Weis <weis@kde.org>
   SPDX-FileCopyrightText: 2004 David Faure <faure@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "KoDocumentInfo.h"

#include "KisDocument.h"
#include "KoXmlNS.h"

#include <QDateTime>
#include <KoStoreDevice.h>
#include <KoXmlWriter.h>
#include <QDomDocument>
#include <KoXmlReader.h>
#include <QDir>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>
#include <kuser.h>
#include <kemailsettings.h>

#include <KritaVersionWrapper.h>


KoDocumentInfo::KoDocumentInfo(QObject *parent) : QObject(parent)
{
    m_aboutTags << "title" << "description" << "subject" << "abstract"
    << "keyword" << "initial-creator" << "editing-cycles" << "editing-time"
    << "date" << "creation-date" << "language" << "license";

    m_authorTags << "creator" << "creator-first-name" << "creator-last-name" << "initial" << "author-title" << "position" << "company";
    m_contactTags << "email" << "telephone" << "telephone-work" << "fax" << "country" << "postal-code" << "city" << "street";
    setAboutInfo("editing-cycles", "0");
    setAboutInfo("time-elapsed", "0");
    setAboutInfo("initial-creator", i18n("Unknown"));
    setAboutInfo("creation-date", QDateTime::currentDateTime()
                 .toString(Qt::ISODate));
}

KoDocumentInfo::KoDocumentInfo(const KoDocumentInfo &rhs, QObject *parent)
    : QObject(parent),
      m_aboutTags(rhs.m_aboutTags),
      m_authorTags(rhs.m_authorTags),
      m_contact(rhs.m_contact),
      m_authorInfo(rhs.m_authorInfo),
      m_authorInfoOverride(rhs.m_authorInfoOverride),
      m_aboutInfo(rhs.m_aboutInfo),
      m_generator(rhs.m_generator)

{
}

KoDocumentInfo::~KoDocumentInfo()
{
}

bool KoDocumentInfo::load(const QDomDocument &doc)
{
    m_authorInfo.clear();

    if (!loadAboutInfo(doc.documentElement()))
        return false;

    if (!loadAuthorInfo(doc.documentElement()))
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

void KoDocumentInfo::setAuthorInfo(const QString &info, const QString &data)
{
    if (!m_authorTags.contains(info) && !m_contactTags.contains(info) && !info.contains("contact-mode-")) {
        return;
    }

    m_authorInfoOverride.insert(info, data);
}

void KoDocumentInfo::setActiveAuthorInfo(const QString &info, const QString &data)
{
    if (!m_authorTags.contains(info) && !m_contactTags.contains(info) && !info.contains("contact-mode-")) {
        return;
    }
    if (m_contactTags.contains(info)) {
        m_contact.insert(data, info);
    } else {
        m_authorInfo.insert(info, data);
    }
    emit infoUpdated(info, data);
}

QString KoDocumentInfo::authorInfo(const QString &info) const
{
    if (!m_authorTags.contains(info)  && !m_contactTags.contains(info) && !info.contains("contact-mode-"))
        return QString();

    return m_authorInfo[ info ];
}

QStringList KoDocumentInfo::authorContactInfo() const
{
    return m_contact.keys();
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

bool KoDocumentInfo::loadOasisAuthorInfo(const QDomNode &metaDoc)
{
    QDomElement e = KoXml::namedItemNS(metaDoc, KoXmlNS::dc, "creator");
    if (!e.isNull() && !e.text().isEmpty())
        setActiveAuthorInfo("creator", e.text());

    QDomNode n = metaDoc.firstChild();
    for (; !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;

        QDomElement e = n.toElement();
        if (!(e.namespaceURI() == KoXmlNS::meta &&
                e.localName() == "user-defined" && !e.text().isEmpty()))
            continue;

        QString name = e.attributeNS(KoXmlNS::meta, "name", QString());
        setActiveAuthorInfo(name, e.text());
    }

    return true;
}

bool KoDocumentInfo::loadAuthorInfo(const QDomElement &e)
{
    m_contact.clear();
    QDomNode n = e.namedItem("author").firstChild();
    for (; !n.isNull(); n = n.nextSibling()) {
        QDomElement e = n.toElement();
        if (e.isNull())
            continue;

        if (e.tagName() == "full-name") {
            setActiveAuthorInfo("creator", e.text().trimmed());
        } else if (e.tagName() == "contact") {
            m_contact.insert(e.text(), e.attribute("type"));
        } else {
            setActiveAuthorInfo(e.tagName(), e.text().trimmed());
        }
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
    for (int i=0; i<m_contact.keys().size(); i++) {
        t = doc.createElement("contact");
        e.appendChild(t);
        QString key = m_contact.keys().at(i);
        t.setAttribute("type", m_contact[key]);
        t.appendChild(doc.createTextNode(key));
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

bool KoDocumentInfo::loadOasisAboutInfo(const QDomNode &metaDoc)
{
    QStringList keywords;
    QDomElement e;
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
            //this is the odf way but add meta:comment if it's already loaded
            QDomElement e  = KoXml::namedItemNS(metaDoc, KoXmlNS::dc, tag);
            if (!e.isNull() && !e.text().isEmpty())
                setAboutInfo("description", aboutInfo("description") + e.text().trimmed());
        } else if (tag == "abstract") {
            //this was the old way so add it to dc:description
            QDomElement e  = KoXml::namedItemNS(metaDoc, KoXmlNS::meta, tag);
            if (!e.isNull() && !e.text().isEmpty())
                setAboutInfo("description", aboutInfo("description") + e.text().trimmed());
        } else if (tag == "title"|| tag == "subject"
                   || tag == "date" || tag == "language") {
            QDomElement e  = KoXml::namedItemNS(metaDoc, KoXmlNS::dc, tag);
            if (!e.isNull() && !e.text().isEmpty())
                setAboutInfo(tag, e.text().trimmed());
        } else if (tag == "generator") {
            setOriginalGenerator(e.text().trimmed());
        } else {
            QDomElement e  = KoXml::namedItemNS(metaDoc, KoXmlNS::meta, tag);
            if (!e.isNull() && !e.text().isEmpty())
                setAboutInfo(tag, e.text().trimmed());
        }
    }

    if (keywords.count() > 0) {
        setAboutInfo("keyword", keywords.join(", "));
    }

    return true;
}

bool KoDocumentInfo::loadAboutInfo(const QDomElement &e)
{
    QDomNode n = e.namedItem("about").firstChild();
    QDomElement tmp;
    for (; !n.isNull(); n = n.nextSibling()) {
        tmp = n.toElement();
        if (tmp.isNull())
            continue;

        if (tmp.tagName() == "abstract")
            setAboutInfo("abstract", tmp.text());

        setAboutInfo(tmp.tagName(), tmp.text());
    }

    return true;
}

QDomElement KoDocumentInfo::saveAboutInfo(QDomDocument &doc)
{
    QDomElement e = doc.createElement("about");
    QDomElement t;

    Q_FOREACH (const QString &tag, m_aboutTags) {
        if (tag == "abstract") {
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
    KisDocument *doc = dynamic_cast< KisDocument *>(parent());
    if (doc && doc->isAutosaving()) {
        return;
    }

    setAboutInfo("editing-cycles", QString::number(aboutInfo("editing-cycles").toInt() + 1));
    setAboutInfo("date", QDateTime::currentDateTime().toString(Qt::ISODate));

    updateParameters();
}

void KoDocumentInfo::updateParameters()
{
    KisDocument *doc = dynamic_cast< KisDocument *>(parent());
    if (doc && (!doc->isModified())) {
        return;
    }

    KConfig config("kritarc");
    config.reparseConfiguration();
    KConfigGroup appAuthorGroup(&config, "Author");
    QString profile = appAuthorGroup.readEntry("active-profile", "");

    QString authorInfo = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/authorinfo/";
    QDir dir(authorInfo);
    QStringList filters = QStringList() << "*.authorinfo";

    //Anon case
    setActiveAuthorInfo("creator", QString());
    setActiveAuthorInfo("initial", "");
    setActiveAuthorInfo("author-title", "");
    setActiveAuthorInfo("position", "");
    setActiveAuthorInfo("company", "");
    if (dir.entryList(filters).contains(profile+".authorinfo")) {
        QFile file(dir.absoluteFilePath(profile+".authorinfo"));
        if (file.exists()) {
            file.open(QFile::ReadOnly);
            QByteArray ba = file.readAll();
            file.close();
            QDomDocument doc = QDomDocument();
            doc.setContent(ba);
            QDomElement root = doc.firstChildElement();

            QDomElement el = root.firstChildElement("nickname");
            if (!el.isNull()) {
                setActiveAuthorInfo("creator", el.text());
            }
            el = root.firstChildElement("givenname");
            if (!el.isNull()) {
                setActiveAuthorInfo("creator-first-name", el.text());
            }
            el = root.firstChildElement("middlename");
            if (!el.isNull()) {
                setActiveAuthorInfo("initial", el.text());
            }
            el = root.firstChildElement("familyname");
            if (!el.isNull()) {
               setActiveAuthorInfo("creator-last-name", el.text());
            }
            el = root.firstChildElement("title");
            if (!el.isNull()) {
                setActiveAuthorInfo("author-title", el.text());
            }
            el = root.firstChildElement("position");
            if (!el.isNull()) {
                setActiveAuthorInfo("position", el.text());
            }
            el = root.firstChildElement("company");
            if (!el.isNull()) {
                setActiveAuthorInfo("company", el.text());
            }

            m_contact.clear();
            el = root.firstChildElement("contact");
            while (!el.isNull()) {
                m_contact.insert(el.text(), el.attribute("type"));
                el = el.nextSiblingElement("contact");
            }
        }
    }

    //allow author info set programmatically to override info from author profile
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
    setAboutInfo("editing-time", QString::number(0));
}

QString KoDocumentInfo::originalGenerator() const
{
    return m_generator;
}

void KoDocumentInfo::setOriginalGenerator(const QString &generator)
{
    m_generator = generator;
}
