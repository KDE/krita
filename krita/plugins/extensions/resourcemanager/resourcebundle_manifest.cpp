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
#include "resourcebundle_manifest.h"

#include <QList>
#include <QString>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QDomNodeList>

#include <KoXmlNS.h>
#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include "KoPattern.h"
#include "KoAbstractGradient.h"
#include "KoResourceServerProvider.h"

#include "kis_brush_server.h"
#include "kis_resource_server_provider.h"
#include "kis_paintop_preset.h"
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

ResourceBundleManifest::ResourceBundleManifest()
{
}

ResourceBundleManifest::~ResourceBundleManifest()
{
}

bool ResourceBundleManifest::load(QIODevice *device)
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
        qWarning() << "Error parsing manifest" << errorMessage << "line" << errorLine << "column" << errorColumn;
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
        QString mediaType = el.attributeNS(KoXmlNS::manifest, "media-type", QString(""));
        QString md5sum = el.attributeNS(KoXmlNS::manifest, "md5sum", QString(""));
        QString version   = el.attributeNS(KoXmlNS::manifest, "version", QString());

        QStringList tagList;
        KoXmlNode tagNode = n.firstChild();
        while (!tagNode.isNull()) {
            if (tagNode.isText()) {
                tagList.append(tagNode.toText().data());
            }
            tagNode = tagNode.nextSibling();
        }

        // Only if fullPath is valid, should we store this entry.
        // If not, we don't bother to find out exactly what is wrong, we just skip it.
        if (!fullPath.isNull() && !mediaType.isEmpty() && !md5sum.isEmpty()) {
            addResource(mediaType, fullPath, tagList, QByteArray::fromHex(md5sum.toAscii()));
        }
    }

    return true;
}

bool ResourceBundleManifest::save(QIODevice *device)
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

       foreach(QString resourceType, m_resources.uniqueKeys()) {
           foreach(const ResourceReference &resource, m_resources[resourceType].values()) {
               manifestWriter.startElement("manifest:file-entry");
               manifestWriter.addAttribute("manifest:media-type", resourceTypeToManifestType(resourceType));
               manifestWriter.addAttribute("manifest:full-path", resourceTypeToManifestType(resourceType) + "/" + QFileInfo(resource.resourcePath).completeBaseName());
               manifestWriter.addAttribute("manifest:md5sum", QString(resource.md5sum.toHex()));
               if (!resource.tagList.isEmpty()) {
                   manifestWriter.startElement("manifest:tags");
                   foreach(const QString tag, resource.tagList) {
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

void ResourceBundleManifest::addResource(const QString &fileTypeName, const QString &fileName, const QStringList &fileTagList, const QByteArray &md5)
{
    ResourceReference ref(fileName, fileTagList, md5);
    if (!m_resources.contains(fileTypeName)) {
        m_resources[fileTypeName] = QMap<QString, ResourceReference>();
    }
    m_resources[fileTypeName].insert(fileName, ref);
}

QStringList ResourceBundleManifest::types() const
{
    return m_resources.keys();
}

QStringList ResourceBundleManifest::tags() const
{
    QSet<QString> tags;
    foreach(const QString &type, m_resources.keys()) {
        foreach(const ResourceReference &ref, m_resources[type].values()) {
            tags += ref.tagList.toSet();
        }
    }
    return QStringList::fromSet(tags);
}

QList<ResourceBundleManifest::ResourceReference> ResourceBundleManifest::files(const QString &type) const
{
    if (!m_resources.contains(type)) {
        return QList<ResourceBundleManifest::ResourceReference>();
    }
    return m_resources[type].values();
}

void ResourceBundleManifest::removeFile(QString fileName)
{
    QList<QString> tags;
    foreach(const QString &type, m_resources.keys()) {
        if (m_resources[type].contains(fileName)) {
            m_resources[type].remove(fileName);
        }
    }
}

QList<QString> ResourceBundleManifest::getFileList(QString /*kritaPath*/, bool /*firstBuild*/)
{
    QList<QString> result;
//    QDomNodeList fileList = m_xmlDocument.elementsByTagName("file");

//    if (firstBuild || isInstalled()) {
//        for (int i = 0; i < fileList.size(); i++) {
//            result.push_front(fileList.at(i).toElement().attributeNode("name").value());
//        }
//    } else {
//        QDomElement currentElement;
//        for (int i = 0; i < fileList.size(); i++) {
//            currentElement = fileList.at(i).toElement();
//            if (currentElement.attribute("src", "") == QString("")) {
//                result.push_front(currentElement.attributeNode("name").value());
//            } else {
//                QString currentFileName = currentElement.attributeNode("name").value();
//                currentFileName = kritaPath + QString("temp/") + currentFileName.section('/', currentFileName.count('/') - 2, currentFileName.count('/') - 2)
//                                  + QString("/") + currentFileName.section('/', currentFileName.count('/'));
//                result.push_front(currentFileName);
//            }
//        }
//    }
    return result;
}

QMap<QString, QString> ResourceBundleManifest::getFilesToExtract()
{
    QString currentFileName;
    QString srcFileName;
    QString targetFileName;

    QMap<QString, QString> result;
//    QDomNodeList fileList = m_xmlDocument.elementsByTagName("file");

//    for (int i = 0; i < fileList.size(); i++) {
//        targetFileName = fileList.at(i).toElement().attribute("name");
//        srcFileName = fileList.at(i).toElement().attribute("src");
//        currentFileName = fileList.at(i).parentNode().toElement().tagName();
//        currentFileName.append("/");
//        currentFileName.append(srcFileName.section('/', srcFileName.count('/')));
//        result.insert(currentFileName, targetFileName);
//    }

    return result;
}

QList<QString> ResourceBundleManifest::getDirList()
{
    QList<QString> result;
//    QDomElement currentElement = m_root.firstChildElement();

//    while (!currentElement.isNull()) {
//        if (!result.contains(currentElement.tagName()) && currentElement.tagName() != "installed") {
//            result.push_back(currentElement.tagName());
//        }
//        currentElement = currentElement.nextSiblingElement();
//    }
    return result;
}


void ResourceBundleManifest::exportTags()
{
//    QString tagName;
//    QString fileName;
//    QDomElement fileElem;
//    QDomElement tagElem;
//    QDomElement typeElem = m_root.firstChildElement();

//    while (!typeElem.isNull()) {
//        tagName = typeElem.tagName();
//        if (tagName == "patterns") {
//            KoResourceServer<KoPattern> *serv = KoResourceServerProvider::instance()->patternServer();
//            fileElem = typeElem.firstChildElement();
//            while (!fileElem.isNull()) {
//                fileName = fileElem.attributeNode("name").value();
//                fileName = fileName.section('/', fileName.count('/'));
//                tagElem = fileElem.firstChildElement();
//                while (!tagElem.isNull()) {
//                    serv->addTag(serv->resourceByName(fileName), tagElem.firstChild().toText().data());
//                    tagElem = tagElem.nextSiblingElement();
//                }
//                fileElem = fileElem.nextSiblingElement();
//            }
//        } else if (tagName == "gradients") {
//            KoResourceServer<KoAbstractGradient> *serv = KoResourceServerProvider::instance()->gradientServer();
//            fileElem = typeElem.firstChildElement();
//            while (!fileElem.isNull()) {
//                fileName = fileElem.attributeNode("name").value();
//                fileName = fileName.section('/', fileName.count('/'));
//                tagElem = fileElem.firstChildElement();
//                while (!tagElem.isNull()) {
//                    serv->addTag(serv->resourceByName(fileName), tagElem.firstChild().toText().data());
//                    tagElem = tagElem.nextSiblingElement();
//                }
//                fileElem = fileElem.nextSiblingElement();
//            }
//        } else if (tagName == "brushes") {
//            KoResourceServer<KisBrush> *serv = KisBrushServer::instance()->brushServer();
//            fileElem = typeElem.firstChildElement();
//            while (!fileElem.isNull()) {
//                fileName = fileElem.attributeNode("name").value();
//                fileName = fileName.section('/', fileName.count('/'));
//                tagElem = fileElem.firstChildElement();
//                while (!tagElem.isNull()) {
//                    serv->addTag(serv->resourceByName(fileName), tagElem.firstChild().toText().data());
//                    tagElem = tagElem.nextSiblingElement();
//                }
//                fileElem = fileElem.nextSiblingElement();
//            }
//        } else if (tagName == "workspaces") {
//            KoResourceServer<KisWorkspaceResource> *serv = KisResourceServerProvider::instance()->workspaceServer();
//            fileElem = typeElem.firstChildElement();
//            while (!fileElem.isNull()) {
//                fileName = fileElem.attributeNode("name").value();
//                fileName = fileName.section('/', fileName.count('/'));
//                tagElem = fileElem.firstChildElement();
//                while (!tagElem.isNull()) {
//                    serv->addTag(serv->resourceByName(fileName), tagElem.firstChild().toText().data());
//                    tagElem = tagElem.nextSiblingElement();
//                }
//                fileElem = fileElem.nextSiblingElement();
//            }
//        } else if (tagName == "palettes") {
//            KoResourceServer<KoColorSet> *serv = KoResourceServerProvider::instance()->paletteServer();
//            fileElem = typeElem.firstChildElement();
//            while (!fileElem.isNull()) {
//                fileName = fileElem.attributeNode("name").value();
//                fileName = fileName.section('/', fileName.count('/'));
//                tagElem = fileElem.firstChildElement();
//                while (!tagElem.isNull()) {
//                    serv->addTag(serv->resourceByName(fileName), tagElem.firstChild().toText().data());
//                    tagElem = tagElem.nextSiblingElement();
//                }
//                fileElem = fileElem.nextSiblingElement();
//            }
//        } else if (tagName == "paintoppresets") {
//            KoResourceServer<KisPaintOpPreset> *serv = KisResourceServerProvider::instance()->paintOpPresetServer();
//            fileElem = typeElem.firstChildElement();
//            while (!fileElem.isNull()) {
//                fileName = fileElem.attributeNode("name").value();
//                fileName = fileName.section('/', fileName.count('/'));
//                tagElem = fileElem.firstChildElement();
//                while (!tagElem.isNull()) {
//                    serv->addTag(serv->resourceByName(fileName), tagElem.firstChild().toText().data());
//                    tagElem = tagElem.nextSiblingElement();
//                }
//                fileElem = fileElem.nextSiblingElement();
//            }
//        }
//        typeElem = typeElem.nextSiblingElement();
//    }
}


void ResourceBundleManifest::install()
{
//    if (m_xmlDocument.elementsByTagName("installed").isEmpty()) {
//        m_root.appendChild(m_xmlDocument.createElement("installed"));
//    }
}

void ResourceBundleManifest::updateFilePaths(QString /*kritaPath*/, QString /*bundleName*/)
{
//    bundleName = bundleName.section('/', bundleName.count('/')).section('.', 0, 0);

//    QDomNodeList fileList = m_xmlDocument.elementsByTagName("file");
//    for (int i = 0; i < fileList.size(); i++) {
//        QDomNode currentNode = fileList.at(i);
//        if (!currentNode.attributes().contains("src")) {
//            QString oldValue = currentNode.toElement().attribute("name");
//            QString newValue = kritaPath + currentNode.parentNode().toElement().tagName() + "/" + bundleName + "/"
//                               + oldValue.section('/', oldValue.count('/'));
//            currentNode.toElement().setAttribute("name", newValue);
//            currentNode.toElement().setAttribute("src", oldValue);
//        }
//    }
}

void ResourceBundleManifest::rename(QString newName)
{
//    QDomNodeList fileList = m_xmlDocument.elementsByTagName("file");
//    for (int i = 0; i < fileList.size(); i++) {
//        QDomElement currentElement = fileList.at(i).toElement();
//        QString oldValue = currentElement.attribute("name");
//        QString newValue = oldValue.section('/', 0, oldValue.count('/') - 2) + QString("/") + newName
//                           + QString("/") + oldValue.section('/', oldValue.count('/'));
//        currentElement.setAttribute("name", newValue);
//    }
}


void ResourceBundleManifest::uninstall()
{
//    QDomNodeList instList = m_xmlDocument.elementsByTagName("installed");

//    if (!instList.isEmpty()) {
//        for (int i = 0; i < instList.size(); i++) {
//            m_root.removeChild(instList.at(i));
//        }
//    }
}

bool ResourceBundleManifest::isInstalled()
{
    return false;
}

