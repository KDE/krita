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
#include <QList>
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
    m_root=m_xmlDocument.createElement("package");
    m_xmlDocument.appendChild(m_root);
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
    QDomNode currentNode = m_root.firstChild();
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
                m_root.insertBefore(nextNode,previousNode);
            }
            else {
                m_root.insertAfter(nextNode,previousNode);
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
    QString attribut;
    QStringList tagList;

    while (!currentNode.isNull()) {
        attribut = currentNode.toElement().attribute("name");
        src.removeChild(currentNode);
        addManiTag(dest.toElement().tagName(),attribut,tagList);
        currentNode=src.firstChild();
    }

    m_root.removeChild(src);
}

//TODO A Revoir vu que c'était censé redéfinir le addTag de generator
QDomElement KoXmlResourceBundleManifest::addManiTag(QString fileTypeName,QString fileName,QStringList fileTagList,bool emptyFile)
{
    QDomNode currentNode;
    bool newNode=false;

    fileTypeName=fileTypeName.toLower();
    QDomNodeList fileTypeList=m_xmlDocument.elementsByTagName(fileTypeName);

    if (emptyFile || fileTypeList.isEmpty()) {
        currentNode=m_xmlDocument.createElement(fileTypeName);
        m_root.appendChild(currentNode);
        newNode=true;
    }
    else {
        currentNode=fileTypeList.item(0);
    }

    if (emptyFile || searchValue(m_xmlDocument.elementsByTagName("file"),"name",fileName).isNull()) {
        QDomElement result=addManiTag(currentNode.toElement(),"file","");
        result.setAttribute("name",fileName);

        if (!fileTagList.isEmpty()) {
            for (int i=0;i<fileTagList.size();i++) {
                addManiTag(result,"tag",fileTagList.at(i));
            }
        }

        return result;
    }
    else {
        if (newNode) {
            m_root.removeChild(currentNode);
        }
        return QDomElement();
    }
}

QDomElement KoXmlResourceBundleManifest::addManiTag(QDomElement parent,QString tagName,QString textValue)
{
    QDomElement currentElem;

    if (textValue!="") {
        QDomNodeList tagList=parent.elementsByTagName(tagName);
        for (int i=0;i<tagList.size();i++) {
            if (tagList.at(i).firstChild().toText().data()==textValue) {
                return tagList.at(i).toElement();
            }
        }
        currentElem=m_xmlDocument.createElement(tagName);
        currentElem.appendChild(m_xmlDocument.createTextNode(textValue));
        parent.appendChild(currentElem);
    }
    else {
        currentElem=m_xmlDocument.createElement(tagName);
        parent.appendChild(currentElem);
    }

    return currentElem;
}

QList<QString> KoXmlResourceBundleManifest::removeFile(QString fileName)
{
    int i;

    QDomNode currentNode;
    QString currentTag;
    QDomAttr currentAtt;

    QList<QString> result;
    QDomNodeList tagList=m_xmlDocument.elementsByTagName("file");

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

QList<QString> KoXmlResourceBundleManifest::getTagsList()
{
    QString currentTextValue;
    QList<QString> result;
    QDomNodeList tagList=m_xmlDocument.elementsByTagName("tag");

    for (int i=0;i<tagList.size();i++) {
        currentTextValue=tagList.at(i).firstChild().toText().data();
        if (!result.contains(currentTextValue)) {
            result.push_front(currentTextValue);
        }
    }
    return result;
}

//TODO Résoudre le pb du src et le fait ke le fichier courant puisse etre différent du fichier qui était auparavant dans l'archive
QList<QString> KoXmlResourceBundleManifest::getFileList(QString kritaPath,bool firstBuild)
{
    QList<QString> result;
    QDomNodeList fileList=m_xmlDocument.elementsByTagName("file");

    if (firstBuild || isInstalled()) {
        for (int i=0;i<fileList.size();i++) {
            result.push_front(fileList.at(i).toElement().attributeNode("name").value());
        }
    }
    else {
        QDomElement currentElement;
        for (int i=0;i<fileList.size();i++) {
            currentElement = fileList.at(i).toElement();
            if(currentElement.attribute("src","")==QString("")) {
                result.push_front(currentElement.attributeNode("name").value());
            }
            else {
                QString currentFileName=currentElement.attributeNode("name").value();
                currentFileName=kritaPath+QString("temp/")+currentFileName.section('/',currentFileName.count('/')-2,currentFileName.count('/')-2)
                                    +QString("/")+currentFileName.section('/',currentFileName.count('/'));
                result.push_front(currentFileName);
            }
        }
    }
    return result;
}

QMap<QString,QString> KoXmlResourceBundleManifest::getFilesToExtract()
{
    QString currentFileName;
    QString srcFileName;
    QString targetFileName;

    QMap<QString,QString> result;
    QDomNodeList fileList=m_xmlDocument.elementsByTagName("file");

    for (int i=0;i<fileList.size();i++) {
        targetFileName = fileList.at(i).toElement().attribute("name");
        srcFileName = fileList.at(i).toElement().attribute("src");
        currentFileName=fileList.at(i).parentNode().toElement().tagName();
        currentFileName.append("/");
        currentFileName.append(srcFileName.section('/',srcFileName.count('/')));
        result.insert(currentFileName,targetFileName);
    }

    return result;
}

QList<QString> KoXmlResourceBundleManifest::getDirList()
{
    QList<QString> result;
    QDomElement currentElement = m_root.firstChildElement();

    while (!currentElement.isNull()) {
        if (!result.contains(currentElement.tagName()) && currentElement.tagName()!="installed") {
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
    QDomElement typeElem=m_root.firstChildElement();

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


void KoXmlResourceBundleManifest::install()
{ 
    if (m_xmlDocument.elementsByTagName("installed").isEmpty()) {
        m_root.appendChild(m_xmlDocument.createElement("installed"));
    }
}

void KoXmlResourceBundleManifest::updateFilePaths(QString kritaPath,QString bundleName)
{
    bundleName=bundleName.section('/',bundleName.count('/')).section('.',0,0);

    QDomNodeList fileList=m_xmlDocument.elementsByTagName("file");
    for (int i=0;i<fileList.size();i++) {
        QDomNode currentNode=fileList.at(i);
        if(!currentNode.attributes().contains("src")) {
            QString oldValue=currentNode.toElement().attribute("name");
            QString newValue=kritaPath+currentNode.parentNode().toElement().tagName()+"/"+bundleName+"/"
                    +oldValue.section('/',oldValue.count('/'));
            currentNode.toElement().setAttribute("name",newValue);
            currentNode.toElement().setAttribute("src",oldValue);
        }
    }
}

void KoXmlResourceBundleManifest::rename(QString newName)
{
    QDomNodeList fileList=m_xmlDocument.elementsByTagName("file");
    for (int i=0;i<fileList.size();i++) {
        QDomElement currentElement=fileList.at(i).toElement();
        QString oldValue=currentElement.attribute("name");
        QString newValue=oldValue.section('/',0,oldValue.count('/')-2)+QString("/")+newName
                +QString("/")+oldValue.section('/',oldValue.count('/'));
        currentElement.setAttribute("name",newValue);
    }
}


void KoXmlResourceBundleManifest::uninstall()
{
    QDomNodeList instList=m_xmlDocument.elementsByTagName("installed");

    if (!instList.isEmpty()) {
        for (int i=0;i<instList.size();i++) {
            m_root.removeChild(instList.at(i));
        }
    }
}

bool KoXmlResourceBundleManifest::isInstalled()
{
    return m_xmlDocument.elementsByTagName("installed").size()!=0;
}

QDomDocument KoXmlResourceBundleManifest::getXmlDocument()
{
    return this->m_xmlDocument;
}
