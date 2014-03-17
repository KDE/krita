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
#include "KoXmlResourceBundleManifest.h"
#include <QtCore/QList>
#include "KoPattern.h"
#include "KoAbstractGradient.h"
#include "KoResourceServerProvider.h"
#include "kis_brush_server.h"
#include "kis_resource_server_provider.h"
#include "kis_paintop_preset.h"
#include "kis_workspace_resource.h"

#include <iostream>
using namespace std;

KoXmlResourceBundleManifest::KoXmlResourceBundleManifest(QString xmlName):KoXmlGenerator(xmlName)
{
    root=xmlDocument.createElement("package");
    xmlDocument.appendChild(root);
}

KoXmlResourceBundleManifest::KoXmlResourceBundleManifest(QByteArray data):KoXmlGenerator(data)
{

}

KoXmlResourceBundleManifest::KoXmlResourceBundleManifest(QIODevice *device):KoXmlGenerator(device)
{

}

KoXmlResourceBundleManifest::~KoXmlResourceBundleManifest()
{

}

KoXmlResourceBundleManifest::TagEnum KoXmlResourceBundleManifest::getTagEnumValue(QString tagName)
{
    if (tagName=="brushes") {
        return Brush;
    }
    else if (tagName=="gradients") {
        return Gradient;
    }
    else if (tagName=="paintoppresets") {
        return Paintop;
    }
    else if (tagName=="palettes") {
        return Palette;
    }
    else if (tagName=="patterns") {
        return Pattern;
    }
    else if (tagName=="templates") {
        return Template;
    }
    else if (tagName=="workspaces") {
        return Workspace;
    }
    else if (tagName=="ref") {
        return Reference;
    }
    else {
        return Other;
    }
}

void KoXmlResourceBundleManifest::checkSort()
{
    QDomNode previousNode;
    QDomNode currentNode = root.firstChild();
    QDomNode nextNode = currentNode.nextSibling();

    TagEnum currentName;
    TagEnum lastOk=getTagEnumValue(currentNode.toElement().tagName());

    while (!nextNode.isNull()) {
        currentName=getTagEnumValue(nextNode.toElement().tagName());

        if (lastOk>currentName) {
            previousNode=currentNode.previousSibling();

            while (getTagEnumValue(previousNode.toElement().tagName())>currentName && !previousNode.isNull()) {
                  previousNode=previousNode.previousSibling();
            }

            if (getTagEnumValue(previousNode.toElement().tagName())==currentName) {
                merge(previousNode,nextNode);
            }
            else if (previousNode.isNull()){
                root.insertBefore(nextNode,previousNode);
            }
            else {
                root.insertAfter(nextNode,previousNode);
            }
        }
        else if (lastOk==currentName) {
            merge(currentNode,nextNode);
        }
        else {
            lastOk=currentName;
            currentNode=nextNode;
        }

        nextNode=currentNode.nextSibling();
    }
}

void KoXmlResourceBundleManifest::merge(QDomNode dest,QDomNode src)
{
    QDomNode currentNode=src.firstChild();

    while (!currentNode.isNull()) {
        addTag(dest.toElement().nodeName(),currentNode.firstChild().toText().data());
        src.removeChild(currentNode);
        currentNode=src.firstChild();
    }

    root.removeChild(src);
}

QDomElement KoXmlResourceBundleManifest::addTag(QString fileTypeName,QString fileName,bool emptyFile)
{
    QDomNode currentNode;
    bool newNode=false;

    fileTypeName=fileTypeName.toLower();
    QDomNodeList fileTypeList=xmlDocument.elementsByTagName(fileTypeName);

    if (emptyFile || fileTypeList.isEmpty()) {
        currentNode=xmlDocument.createElement(fileTypeName);
        root.appendChild(currentNode);
        newNode=true;
    }
    else {
        currentNode=fileTypeList.item(0);
    }

    if (emptyFile || searchValue(xmlDocument.elementsByTagName("file"),"name",fileName).isNull()) {
        QDomElement result=addTag(currentNode.toElement(),"file","");
        result.setAttribute("name",fileName);

        importFileTags(result,fileTypeName,fileName);

        return result;
    }
    else {
        if (newNode) {

            root.removeChild(currentNode);
        }
        return QDomElement();
    }
}

QDomElement KoXmlResourceBundleManifest::addTag(QDomElement parent,QString tagName,QString textValue)
{
    QDomElement currentElem;

    if (textValue!="") {
        QDomNodeList tagList=parent.elementsByTagName(tagName);
        for (int i=0;i<tagList.size();i++) {
            if (tagList.at(i).firstChild().toText().data()==textValue) {
                return tagList.at(i).toElement();
            }
        }
        currentElem=xmlDocument.createElement(tagName);
        currentElem.appendChild(xmlDocument.createTextNode(textValue));
        parent.appendChild(currentElem);
    }
    else {
        currentElem=xmlDocument.createElement(tagName);
        parent.appendChild(currentElem);
    }

    return currentElem;
}

void KoXmlResourceBundleManifest::importFileTags(QDomElement parent,QString fileTypeName,QString fileName)
{
    QStringList resourceTagslist;
    KoResource *currentResource;

    fileName=fileName.section('/',fileName.count('/'));

    if (fileTypeName=="patterns") {
        KoResourceServer<KoPattern> *serv=KoResourceServerProvider::instance()->patternServer();
        currentResource=serv->resourceByName(fileName);
        if (!currentResource) {
            return;
        }
        resourceTagslist=serv->assignedTagsList(currentResource);
    }
    else if (fileTypeName=="gradients") {
        KoResourceServer<KoAbstractGradient> *serv=KoResourceServerProvider::instance()->gradientServer();
        currentResource=serv->resourceByName(fileName);
        if (!currentResource) {
            return;
        }
        resourceTagslist=serv->assignedTagsList(currentResource);
    }
    else if (fileTypeName=="brushes") {
        KoResourceServer<KisBrush> *serv=KisBrushServer::instance()->brushServer();
        currentResource=serv->resourceByName(fileName);
        if (!currentResource) {
            return;
        }
        resourceTagslist=serv->assignedTagsList(currentResource);
    }
    else if (fileTypeName=="workspaces") {
        KoResourceServer<KisWorkspaceResource> *serv=KisResourceServerProvider::instance()->workspaceServer();
        currentResource=serv->resourceByName(fileName);
        if (!currentResource) {
            return;
        }
        resourceTagslist=serv->assignedTagsList(currentResource);
    }
    else if (fileTypeName=="palettes") {
        KoResourceServer<KoColorSet> *serv=KoResourceServerProvider::instance()->paletteServer();
        currentResource=serv->resourceByName(fileName);
        if (!currentResource) {
            return;
        }
        resourceTagslist=serv->assignedTagsList(currentResource);
    }
    else if (fileTypeName=="paintoppresets") {
        KoResourceServer<KisPaintOpPreset> *serv=KisResourceServerProvider::instance()->paintOpPresetServer();
        currentResource=serv->resourceByName(fileName);
        if (!currentResource) {
            return;
        }
        resourceTagslist=serv->assignedTagsList(currentResource);
    }
    else return;

    for (int i=0;i<resourceTagslist.size();i++) {
        addTag(parent,"Tag",resourceTagslist.at(i));
    }
}

QList<QString> KoXmlResourceBundleManifest::removeFile(QString fileName)
{
    int i;

    QDomNode currentNode;
    QString currentTag;
    QDomAttr currentAtt;

    QList<QString> result;
    QDomNodeList tagList=xmlDocument.elementsByTagName("file");

    if (tagList.isEmpty()) {
        return result;
    }
    else {
        for (i=0;i<tagList.size();i++) {
            currentNode=tagList.at(i);
            currentAtt=currentNode.toElement().attributeNode("name");
            if (!currentAtt.isNull() && currentAtt.value()==fileName) {
                break;
            }
        }

        if (i==tagList.size() || currentNode.isNull()) {
            return result;
        }
        else {
            tagList=currentNode.toElement().elementsByTagName("tag");
            for (i=0;i<tagList.size();i++) {
                currentTag=tagList.at(i).firstChild().toText().data();
                if (!result.contains(currentTag)) {
                    result.push_front(currentTag);
                }
            }
        }
    }

    currentNode.parentNode().removeChild(currentNode);
    return result;
}

bool KoXmlResourceBundleManifest::isInstalled()
{
    return xmlDocument.elementsByTagName("installed").size()!=0;
}

void KoXmlResourceBundleManifest::install()
{
    if (xmlDocument.elementsByTagName("installed").isEmpty()) {
        root.appendChild(xmlDocument.createElement("installed"));
    }
}

void KoXmlResourceBundleManifest::uninstall()
{
    QDomNodeList instList=xmlDocument.elementsByTagName("installed");

    if (!instList.isEmpty()) {
        for (int i=0;i<instList.size();i++) {
            root.removeChild(instList.at(i));
        }
    }
}

QList<QString> KoXmlResourceBundleManifest::getTagList()
{
    QString currentTextValue;
    QList<QString> result;
    QDomNodeList tagList=xmlDocument.elementsByTagName("tag");

    for (int i=0;i<tagList.size();i++) {
        currentTextValue=tagList.at(i).firstChild().toText().data();
        if (!result.contains(currentTextValue)) {
            result.push_front(currentTextValue);
        }
    }
    return result;
}

QList<QString> KoXmlResourceBundleManifest::getFileList()
{
    QList<QString> result;
    QDomNodeList fileList=xmlDocument.elementsByTagName("file");

    for (int i=0;i<fileList.size();i++) {
        result.push_front(fileList.at(i).toElement().attributeNode("name").value());
    }

    return result;
}

QList<QString> KoXmlResourceBundleManifest::getFilesToExtract()
{
    QString currentTagName;
    QString currentFileName;

    QList<QString> result;
    QDomNodeList fileList=xmlDocument.elementsByTagName("file");

    for (int i=0;i<fileList.size();i++) {
        currentFileName = fileList.at(i).toElement().attributeNode("name").value();
        currentTagName=fileList.at(i).parentNode().toElement().tagName();
        currentTagName.append("/");
        currentTagName.append(currentFileName.section('/',currentFileName.count('/')));
        result.push_front(currentTagName);
    }

    return result;
}

QList<QString> KoXmlResourceBundleManifest::getDirList()
{
    QList<QString> result;
    QDomElement currentElement = root.firstChildElement();

    while (!currentElement.isNull()) {
        if (!result.contains(currentElement.tagName())) {
            result.push_back(currentElement.tagName());
        }
        currentElement = currentElement.nextSiblingElement();
    }
    return result;
}

//TODO Lors de l'ajout d'un fichier, changer le nom de celui-ci une fois le paquet construit
//Si c'est la première fois qu'il est construit

void KoXmlResourceBundleManifest::exportTags()
{
    QString tagName;
    QString fileName;
    QDomElement fileElem;
    QDomElement tagElem;
    QDomElement typeElem=root.firstChildElement();

    while (!typeElem.isNull()) {
        tagName=typeElem.tagName();
        if (tagName=="patterns") {
            KoResourceServer<KoPattern> *serv=KoResourceServerProvider::instance()->patternServer();
            fileElem=typeElem.firstChildElement();
            while (!fileElem.isNull()) {
                fileName=fileElem.attributeNode("name").value();
                fileName=fileName.section('/',fileName.count('/'));
                tagElem=fileElem.firstChildElement();
                while (!tagElem.isNull()) {
                    serv->addTag(serv->resourceByName(fileName),tagElem.firstChild().toText().data());
                    tagElem=tagElem.nextSiblingElement();
                }
                fileElem=fileElem.nextSiblingElement();
            }
        }
        else if (tagName=="gradients") {
            KoResourceServer<KoAbstractGradient> *serv=KoResourceServerProvider::instance()->gradientServer();
            fileElem=typeElem.firstChildElement();
            while (!fileElem.isNull()) {
                fileName=fileElem.attributeNode("name").value();
                fileName=fileName.section('/',fileName.count('/'));
                tagElem=fileElem.firstChildElement();
                while (!tagElem.isNull()) {
                    serv->addTag(serv->resourceByName(fileName),tagElem.firstChild().toText().data());
                    tagElem=tagElem.nextSiblingElement();
                }
                fileElem=fileElem.nextSiblingElement();
            }
        }
        else if (tagName=="brushes") {
            KoResourceServer<KisBrush> *serv=KisBrushServer::instance()->brushServer();
            fileElem=typeElem.firstChildElement();
            while (!fileElem.isNull()) {
                fileName=fileElem.attributeNode("name").value();
                fileName=fileName.section('/',fileName.count('/'));
                tagElem=fileElem.firstChildElement();
                while (!tagElem.isNull()) {
                    serv->addTag(serv->resourceByName(fileName),tagElem.firstChild().toText().data());
                    tagElem=tagElem.nextSiblingElement();
                }
                fileElem=fileElem.nextSiblingElement();
            }
        }
        else if (tagName=="workspaces") {
            KoResourceServer<KisWorkspaceResource> *serv=KisResourceServerProvider::instance()->workspaceServer();
            fileElem=typeElem.firstChildElement();
            while (!fileElem.isNull()) {
                fileName=fileElem.attributeNode("name").value();
                fileName=fileName.section('/',fileName.count('/'));
                tagElem=fileElem.firstChildElement();
                while (!tagElem.isNull()) {
                    serv->addTag(serv->resourceByName(fileName),tagElem.firstChild().toText().data());
                    tagElem=tagElem.nextSiblingElement();
                }
                fileElem=fileElem.nextSiblingElement();
            }
        }
        else if (tagName=="palettes") {
            KoResourceServer<KoColorSet> *serv=KoResourceServerProvider::instance()->paletteServer();
            fileElem=typeElem.firstChildElement();
            while (!fileElem.isNull()) {
                fileName=fileElem.attributeNode("name").value();
                fileName=fileName.section('/',fileName.count('/'));
                tagElem=fileElem.firstChildElement();
                while (!tagElem.isNull()) {
                    serv->addTag(serv->resourceByName(fileName),tagElem.firstChild().toText().data());
                    tagElem=tagElem.nextSiblingElement();
                }
                fileElem=fileElem.nextSiblingElement();
            }
        }
        else if (tagName=="paintoppresets") {
            KoResourceServer<KisPaintOpPreset> *serv=KisResourceServerProvider::instance()->paintOpPresetServer();
            fileElem=typeElem.firstChildElement();
            while (!fileElem.isNull()) {
                fileName=fileElem.attributeNode("name").value();
                fileName=fileName.section('/',fileName.count('/'));
                tagElem=fileElem.firstChildElement();
                while (!tagElem.isNull()) {
                    serv->addTag(serv->resourceByName(fileName),tagElem.firstChild().toText().data());
                    tagElem=tagElem.nextSiblingElement();
                }
                fileElem=fileElem.nextSiblingElement();
            }
        }
        typeElem=typeElem.nextSiblingElement();
    }
}
