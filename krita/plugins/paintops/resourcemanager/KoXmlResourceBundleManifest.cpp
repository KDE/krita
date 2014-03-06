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
    QDomNode prev;
    QDomNode current = root.firstChild();
    QDomNode next = current.nextSibling();

    TagEnum tagName;
    TagEnum lastOk=getTagEnumValue(current.toElement().tagName());

    while (!next.isNull()) {
        tagName=getTagEnumValue(next.toElement().tagName());

        if (lastOk>tagName) {
            prev=current.previousSibling();

            while (getTagEnumValue(prev.toElement().tagName())>tagName && !prev.isNull()) {
                  prev=prev.previousSibling();
            }

            if (getTagEnumValue(prev.toElement().tagName())==tagName) {
                merge(prev,next);
            }
            else if (prev.isNull()){
                root.insertBefore(next,prev);
            }
            else {
                root.insertAfter(next,prev);
            }
        }
        else if (lastOk==tagName) {
            merge(current,next);
        }
        else {
            lastOk=tagName;
            current=next;
        }

        next=current.nextSibling();
    }
}

void KoXmlResourceBundleManifest::merge(QDomNode dest,QDomNode src)
{
    QDomNode node=src.firstChild();
    while (!node.isNull()) {
        addTag(dest.toElement().nodeName(),node.firstChild().toText().data());
        src.removeChild(node);
        node=src.firstChild();
    }
    root.removeChild(src);
}

QDomElement KoXmlResourceBundleManifest::addTag(QString fileTypeName,QString fileName,bool emptyFile)
{
    fileTypeName=fileTypeName.toLower();

    QDomNode node;
    QDomNodeList fileTypeList=xmlDocument.elementsByTagName(fileTypeName);
    bool newNode=false;

    if (emptyFile || fileTypeList.isEmpty()) {
        node=xmlDocument.createElement(fileTypeName);
        root.appendChild(node);
        newNode=true;
    }
    else {
        node=fileTypeList.item(0);
    }

    if (emptyFile || searchValue(xmlDocument.elementsByTagName("file"),"name",fileName).isNull()) {
        QDomElement result=addTag(node.toElement(),"file","");
        result.setAttribute("name",fileName);

        importFileTags(result,fileTypeName,fileName);

        return result;
    }
    else {
        if (newNode) {

            root.removeChild(node);
        }
        return QDomElement();
    }
}

void KoXmlResourceBundleManifest::importFileTags(QDomElement parent,QString fileTypeName,QString fileName)
{
    QStringList list;
    fileName=fileName.section('/',fileName.count('/'));
    KoResource *res;

    if (fileTypeName=="patterns") {
        KoResourceServer<KoPattern> *serv=KoResourceServerProvider::instance()->patternServer();
        res=serv->resourceByName(fileName);
        if (!res) {
            return;
        }
        list=serv->assignedTagsList(res);
    }
    else if (fileTypeName=="gradients") {
        KoResourceServer<KoAbstractGradient> *serv=KoResourceServerProvider::instance()->gradientServer();
        res=serv->resourceByName(fileName);
        if (!res) {
            return;
        }
        list=serv->assignedTagsList(res);
    }
    else if (fileTypeName=="brushes") {
        KoResourceServer<KisBrush> *serv=KisBrushServer::instance()->brushServer();
        res=serv->resourceByName(fileName);
        if (!res) {
            return;
        }
        list=serv->assignedTagsList(res);
    }
    else if (fileTypeName=="workspaces") {
        KoResourceServer<KisWorkspaceResource> *serv=KisResourceServerProvider::instance()->workspaceServer();
        res=serv->resourceByName(fileName);
        if (!res) {
            return;
        }
        list=serv->assignedTagsList(res);
    }
    else if (fileTypeName=="palettes") {
        KoResourceServer<KoColorSet> *serv=KoResourceServerProvider::instance()->paletteServer();
        res=serv->resourceByName(fileName);
        if (!res) {
            return;
        }
        list=serv->assignedTagsList(res);
    }
    else if (fileTypeName=="paintoppresets") {
        KoResourceServer<KisPaintOpPreset> *serv=KisResourceServerProvider::instance()->paintOpPresetServer();
        res=serv->resourceByName(fileName);
        if (!res) {
            return;
        }
        list=serv->assignedTagsList(res);
    }
    else return;

    for (int i=0;i<list.size();i++) {
        addTag(parent,"Tag",list.at(i));
    }
}

QDomElement KoXmlResourceBundleManifest::addTag(QDomElement parent,QString tagName,QString textValue)
{
    QDomElement prov;
    if (textValue!="") {
        QDomNodeList list=parent.elementsByTagName(tagName);
        for (int i=0;i<list.size();i++) {
            if (list.at(i).firstChild().toText().data()==textValue) {
                return list.at(i).toElement();
            }
        }
        prov=xmlDocument.createElement(tagName);
        prov.appendChild(xmlDocument.createTextNode(textValue));
        parent.appendChild(prov);
    }
    else {
        prov=xmlDocument.createElement(tagName);
        parent.appendChild(prov);
    }
    return prov;
}

QList<QString> KoXmlResourceBundleManifest::removeFile(QString fileName)
{
    QList<QString> result;
    QDomNode prov;
    QDomNodeList tagList=xmlDocument.elementsByTagName("file");
    QString tagProv;
    QDomAttr att;
    int i;

    if (tagList.isEmpty()) {
        return result;
    }
    else {
        for (i=0;i<tagList.size();i++) {
            prov=tagList.at(i);
            att=prov.toElement().attributeNode("name");
            if (!att.isNull()) {
                if (att.value()==fileName) {
                    break;
                }
            }
        }
        if (i==tagList.size() || prov.isNull()) {
            return result;
        }
        else {
            tagList=prov.toElement().elementsByTagName("tag");
            for (i=0;i<tagList.size();i++) {
                tagProv=tagList.at(i).firstChild().toText().data();
                if (!result.contains(tagProv)) {
                    result.push_front(tagProv);
                }
            }
        }
    }
    prov.parentNode().removeChild(prov);
    return result;
}

QList<QString> KoXmlResourceBundleManifest::getTagList(){
    QDomNodeList liste=xmlDocument.elementsByTagName("tag");
    QList<QString> result;
    QString prov;
    for (int i=0;i<liste.size();i++) {
        prov=liste.at(i).firstChild().toText().data();
        if (!result.contains(prov)) {
            result.push_front(prov);
        }
    }
    return result;
}

QList<QString> KoXmlResourceBundleManifest::getFileList()
{
    QDomNodeList liste=xmlDocument.elementsByTagName("file");

    QList<QString> result;
    for (int i=0;i<liste.size();i++) {
        result.push_front(liste.at(i).toElement().attributeNode("name").value());
    }

    return result;
}

QList<QString> KoXmlResourceBundleManifest::getFilesToExtract()
{
    QList<QString> result;
    QString prov;
    QString fileName;
    QDomNodeList liste=xmlDocument.elementsByTagName("file");

    for (int i=0;i<liste.size();i++) {
        fileName = liste.at(i).toElement().attributeNode("name").value();
        prov=liste.at(i).parentNode().toElement().tagName();
        prov.append("/");
        prov.append(fileName.section('/',fileName.count('/')));
        result.push_front(prov);
    }

    return result;
}

QList<QString> KoXmlResourceBundleManifest::getDirList()
{
    QList<QString> result;
    QDomElement elt = root.firstChildElement();

    while (!elt.isNull()) {
        if (!result.contains(elt.tagName())) {
            result.push_back(elt.tagName());
        }
        elt = elt.nextSiblingElement();
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
