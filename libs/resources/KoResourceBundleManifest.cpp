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
#include <KoXmlReader.h>
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
        return false;
    }

    if (!errorMessage.isEmpty()) {
        warnKrita << "Error parsing manifest" << errorMessage << "line" << errorLine << "column" << errorColumn;
        return false;
    }

    // First find the manifest:manifest node.
    QDomNode n = manifestDocument.firstChild();
    for (; !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement()) {
            continue;
        }
        if (n.toElement().localName() == "manifest" && n.toElement().namespaceURI() == KoXmlNS::manifest) {
            break;
        }
    }

    if (n.isNull()) {
        // "Could not find manifest:manifest";
        return false;
    }

    // Now loop through the children of the manifest:manifest and
    // store all the manifest:file-entry elements.
    const QDomElement  manifestElement = n.toElement();
    for (n = manifestElement.firstChild(); !n.isNull(); n = n.nextSibling()) {

        if (!n.isElement())
            continue;

        QDomElement el = n.toElement();
        if (!(el.localName() == "file-entry" && el.namespaceURI() == KoXmlNS::manifest))
            continue;

        QString fullPath  = el.attributeNS(KoXmlNS::manifest, "full-path", QString());
        QString mediaType = el.attributeNS(KoXmlNS::manifest, "media-type", QString());
        QString md5sum = el.attributeNS(KoXmlNS::manifest, "md5sum", QString());
        QString version   = el.attributeNS(KoXmlNS::manifest, "version", QString());

        QStringList tagList;
        QDomNode tagNode = n.firstChildElement().firstChildElement();
        while (!tagNode.isNull()) {
            if (tagNode.firstChild().isText()) {
                tagList.append(tagNode.firstChild().toText().data());
            }
            tagNode = tagNode.nextSibling();
        }

        // Only if fullPath is valid, should we store this entry.
        // If not, we don't bother to find out exactly what is wrong, we just skip it.
        if (!fullPath.isNull() && !mediaType.isEmpty() && !md5sum.isEmpty()) {
            addResource(mediaType, fullPath, tagList, QByteArray::fromHex(md5sum.toLatin1()));
        }
    }

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

       Q_FOREACH (QString resourceType, m_resources.uniqueKeys()) {
           Q_FOREACH (const ResourceReference &resource, m_resources[resourceType].values()) {
               manifestWriter.startElement("manifest:file-entry");
               manifestWriter.addAttribute("manifest:media-type", resourceTypeToManifestType(resourceType));
               // we cannot just use QFileInfo(resource.resourcePath).fileName() because it would cut off the subfolder
               // but the resourcePath is already correct, so let's just add the resourceType
               manifestWriter.addAttribute("manifest:full-path", resourceTypeToManifestType(resourceType) + "/" + resource.resourcePath);
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

void KoResourceBundleManifest::addResource(const QString &fileTypeName, const QString &fileName, const QStringList &fileTagList, const QString &md5)
{
    ResourceReference ref(fileName, fileTagList, fileTypeName, md5);
    if (!m_resources.contains(fileTypeName)) {
        m_resources[fileTypeName] = QMap<QString, ResourceReference>();
    }
    m_resources[fileTypeName].insert(fileName, ref);
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
    // If no type is specified we return all the resources
    if(type.isEmpty()) {
        QList<ResourceReference> resources;
        QList<QMap<QString, ResourceReference> >::iterator i;
        QList<QMap<QString, ResourceReference> > values = m_resources.values();
        for(i = values.begin(); i != values.end(); ++i) {
            resources.append(i->values());
        }

        return resources;
    }
    else if (!m_resources.contains(type)) {
        return QList<KoResourceBundleManifest::ResourceReference>();
    }
    return m_resources[type].values();
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

