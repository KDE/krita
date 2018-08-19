/* This file is part of the KDE project
   Copyright (C) 2014, Victor Lafon <metabolic.ewilan@hotmail.fr>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "KisResourceBundleManifest.h"

#include <QList>
#include <QString>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QDomNodeList>

#include <KoXmlNS.h>
#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <resources/KoPattern.h>
#include <resources/KoAbstractGradient.h>

#include "kis_brush_server.h"
#include "KisResourceServerProvider.h"
#include <brushengine/kis_paintop_preset.h>
#include "kis_workspace_resource.h"

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
    if (type == "patterns" || type == "gradients" || type == "palettes") {
        return "ko_" + type;
    }
    else {
        return "kis_" + type;
    }
}

KisResourceBundleManifest::KisResourceBundleManifest()
{
}

KisResourceBundleManifest::~KisResourceBundleManifest()
{
}

bool KisResourceBundleManifest::load(QIODevice *device)
{
    m_resources.clear();
    if (!device->isOpen()) {
        if (!device->open(QIODevice::ReadOnly)) {
            return false;
        }
    }

    KoXmlDocument manifestDocument;
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
    KoXmlNode n = manifestDocument.firstChild();
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
    const KoXmlElement  manifestElement = n.toElement();
    for (n = manifestElement.firstChild(); !n.isNull(); n = n.nextSibling()) {

        if (!n.isElement())
            continue;

        KoXmlElement el = n.toElement();
        if (!(el.localName() == "file-entry" && el.namespaceURI() == KoXmlNS::manifest))
            continue;

        QString fullPath  = el.attributeNS(KoXmlNS::manifest, "full-path", QString());
        QString mediaType = el.attributeNS(KoXmlNS::manifest, "media-type", QString());
        QString md5sum = el.attributeNS(KoXmlNS::manifest, "md5sum", QString());
        QString version   = el.attributeNS(KoXmlNS::manifest, "version", QString());

        QStringList tagList;
        KoXmlNode tagNode = n.firstChildElement().firstChildElement();
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

bool KisResourceBundleManifest::save(QIODevice *device)
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
               manifestWriter.addAttribute("manifest:full-path", resourceTypeToManifestType(resourceType) + "/" + QFileInfo(resource.resourcePath).fileName());
               manifestWriter.addAttribute("manifest:md5sum", QString(resource.md5sum.toHex()));
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

void KisResourceBundleManifest::addResource(const QString &fileTypeName, const QString &fileName, const QStringList &fileTagList, const QByteArray &md5)
{
    ResourceReference ref(fileName, fileTagList, fileTypeName, md5);
    if (!m_resources.contains(fileTypeName)) {
        m_resources[fileTypeName] = QMap<QString, ResourceReference>();
    }
    m_resources[fileTypeName].insert(fileName, ref);
}

QStringList KisResourceBundleManifest::types() const
{
    return m_resources.keys();
}

QStringList KisResourceBundleManifest::tags() const
{
    QSet<QString> tags;
    Q_FOREACH (const QString &type, m_resources.keys()) {
        Q_FOREACH (const ResourceReference &ref, m_resources[type].values()) {
            tags += ref.tagList.toSet();
        }
    }
    return QStringList::fromSet(tags);
}

QList<KisResourceBundleManifest::ResourceReference> KisResourceBundleManifest::files(const QString &type) const
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
        return QList<KisResourceBundleManifest::ResourceReference>();
    }
    return m_resources[type].values();
}

void KisResourceBundleManifest::removeFile(QString fileName)
{
    QList<QString> tags;
    Q_FOREACH (const QString &type, m_resources.keys()) {
        if (m_resources[type].contains(fileName)) {
            m_resources[type].remove(fileName);
        }
    }
}

