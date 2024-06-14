/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2014 Victor Lafon <metabolic.ewilan@hotmail.fr>

   SPDX-License-Identifier: LGPL-2.1-or-later
*/
#include "KoResourceBundleManifest.h"

#include <QList>
#include <QSet>
#include <QString>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QDomNodeList>
#include <QFileInfo>

#include <KoXmlNS.h>
#include <KoXmlWriter.h>

#include <kis_debug.h>

#include "KisResourceTypes.h"

QString resourceTypeToManifestType(const QString &type) {
    if (type.startsWith("ko_")) {
        return type.mid(3);
    }
    else if (type.startsWith("kis_")) {
        return type.mid(4);
    }
    else {
        return type;
    }
}

QString manifestTypeToResourceType(const QString &type) {
    if (type == ResourceType::Patterns || type == ResourceType::Gradients || type == ResourceType::Palettes) {
        return "ko_" + type;
    }
    else {
        return "kis_" + type;
    }
}

KoResourceBundleManifest::KoResourceBundleManifest()
{
}

KoResourceBundleManifest::~KoResourceBundleManifest()
{
}

bool KoResourceBundleManifest::load(QIODevice *device)
{
    m_resources.clear();
    if (!device->isOpen()) {
        if (!device->open(QIODevice::ReadOnly)) {
            return false;
        }
    }

    QDomDocument manifestDocument;
    QString errorMessage;
    int errorLine;
    int errorColumn;
    if (!manifestDocument.setContent(device, true, &errorMessage, &errorLine, &errorColumn)) {
        warnKrita << "Error parsing manifest" << errorMessage
                  << "line" << errorLine
                  << "column" << errorColumn;
        return false;
    }

    QDomElement root = manifestDocument.documentElement();
    if (root.localName() != "manifest" || root.namespaceURI() != KoXmlNS::manifest) {
        return false;
    }

    QDomElement e = root.firstChildElement("file-entry");
    for (; !e.isNull(); e = e.nextSiblingElement("file-entry")) {
        if (!parseFileEntry(e)) {
            warnKrita << "Skipping invalid manifest entry"
                      << "line" << e.lineNumber();
        }
    }

    return true;
}

bool KoResourceBundleManifest::parseFileEntry(const QDomElement &e)
{
    if (e.localName() != "file-entry" || e.namespaceURI() != KoXmlNS::manifest) {
        return false;
    }

    QString fullPath  = e.attributeNS(KoXmlNS::manifest, "full-path");
    QString mediaType = e.attributeNS(KoXmlNS::manifest, "media-type");
    QString md5sum    = e.attributeNS(KoXmlNS::manifest, "md5sum");
    QString version   = e.attributeNS(KoXmlNS::manifest, "version");

    if (fullPath == "/" && mediaType == "application/x-krita-resourcebundle") {
        // The manifest always contains an entry for the bundle root.
        // This is not a resource, so skip it without indicating failure.
        return true;
    } else if (fullPath.isNull() || mediaType.isNull() || md5sum.isNull()) {
        return false;
    }

    QStringList tagList;
    QDomElement t = e.firstChildElement("tags")
                     .firstChildElement("tag");
    for (; !t.isNull(); t = t.nextSiblingElement("tag")) {
        QString tag = t.text();
        if (!tag.isNull()) {
            tagList.append(tag);
        }
    }

    addResource(mediaType, fullPath, tagList, QByteArray::fromHex(md5sum.toLatin1()), -1);
    return true;
}

bool KoResourceBundleManifest::save(QIODevice *device)
{
       if (!device->isOpen()) {
           if (!device->open(QIODevice::WriteOnly)) {
               return false;
           }
       }
       KoXmlWriter manifestWriter(device);
       manifestWriter.startDocument("manifest:manifest");
       manifestWriter.startElement("manifest:manifest");
       manifestWriter.addAttribute("xmlns:manifest", KoXmlNS::manifest);
       manifestWriter.addAttribute("manifest:version", "1.2");
       manifestWriter.addManifestEntry("/", "application/x-krita-resourcebundle");

       Q_FOREACH (QString resourceType, m_resources.keys()) {
           Q_FOREACH (const ResourceReference &resource, m_resources[resourceType].values()) {
               manifestWriter.startElement("manifest:file-entry");
               manifestWriter.addAttribute("manifest:media-type", resourceTypeToManifestType(resourceType));
               // we cannot just use QFileInfo(resource.resourcePath).fileName() because it would cut off the subfolder
               // but the resourcePath is already correct, so let's just add the resourceType
               manifestWriter.addAttribute("manifest:full-path", resourceTypeToManifestType(resourceType) + "/" + resource.filenameInBundle);
               manifestWriter.addAttribute("manifest:md5sum", resource.md5sum);
               if (!resource.tagList.isEmpty()) {
                   manifestWriter.startElement("manifest:tags");
                   Q_FOREACH (const QString tag, resource.tagList) {
                       manifestWriter.startElement("manifest:tag");
                       manifestWriter.addTextNode(tag);
                       manifestWriter.endElement();
                   }
                   manifestWriter.endElement();
               }
               manifestWriter.endElement();
           }
       }

       manifestWriter.endElement();
       manifestWriter.endDocument();

       return true;
}

void KoResourceBundleManifest::addResource(const QString &fileTypeName, const QString &fileName, const QStringList &fileTagList, const QString &md5, const int resourceId, const QString filenameInBundle)
{
    ResourceReference ref(fileName, fileTagList, fileTypeName, md5, resourceId, filenameInBundle);
    if (!m_resources.contains(fileTypeName)) {
        m_resources[fileTypeName] = QMap<QString, ResourceReference>();
    }
    m_resources[fileTypeName].insert(fileName, ref);
}

void KoResourceBundleManifest::removeResource(KoResourceBundleManifest::ResourceReference &resource)
{
    if (m_resources.contains(resource.fileTypeName)) {
        if (m_resources[resource.fileTypeName].contains(resource.resourcePath)) {
            m_resources[resource.fileTypeName].take(resource.resourcePath);
        }
    }
}

QStringList KoResourceBundleManifest::types() const
{
    return m_resources.keys();
}

QStringList KoResourceBundleManifest::tags() const
{
    QSet<QString> tags;
    Q_FOREACH (const QString &type, m_resources.keys()) {
        Q_FOREACH (const ResourceReference &ref, m_resources[type].values()) {
#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
            tags += QSet<QString>(ref.tagList.begin(), ref.tagList.end());
#else
            tags += QSet<QString>::fromList(ref.tagList);
#endif

        }
    }
#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
    return QStringList(tags.begin(), tags.end());
#else
    return QStringList::fromSet(tags);
#endif
}

QList<KoResourceBundleManifest::ResourceReference> KoResourceBundleManifest::files(const QString &type) const
{
    QList<ResourceReference> resources;

    if (type.isEmpty()) {
        // If no type is specified we return all the resources.
        Q_FOREACH (const QString &type, m_resources.keys()) {
            resources += m_resources[type].values();
        }
    } else if (m_resources.contains(type)) {
        resources = m_resources[type].values();
    }

    return resources;
}

void KoResourceBundleManifest::removeFile(QString fileName)
{
    QList<QString> tags;
    Q_FOREACH (const QString &type, m_resources.keys()) {
        if (m_resources[type].contains(fileName)) {
            m_resources[type].remove(fileName);
        }
    }
}

