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
#include "KoXmlManifest.h"

KoXmlManifest::KoXmlManifest(QString xmlName):KoXmlGenerator(xmlName)
{
    root=xmlDocument.createElement("package");
    xmlDocument.appendChild(root);
}

KoXmlManifest::KoXmlManifest(QFile *file):KoXmlGenerator(file,"package")
{

}

KoXmlManifest::~KoXmlManifest()
{

}

KoXmlManifest::TagEnum KoXmlManifest::getTagEnumValue(QString tagName)
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

void KoXmlManifest::checkSort()
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

void KoXmlManifest::merge(QDomNode dest,QDomNode src)
{
    QDomNode node=src.firstChild();
    while (!node.isNull()) {
        addTag(dest.toElement().nodeName(),node.firstChild().toText().data());
        src.removeChild(node);
        node=src.firstChild();
    }
    root.removeChild(src);
}


QDomElement KoXmlManifest::addTag(QString fileTypeName,QString fileName,bool emptyFile)
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

    if (emptyFile || searchValue(xmlDocument.elementsByTagName("file"),fileName).isNull()) {

        QDomElement result=node.appendChild(xmlDocument.createElement("file")).toElement();

        result.appendChild(xmlDocument.createTextNode(fileName));
        return result;
    }
    else {
        if (newNode) {
            root.removeChild(node);
        }
        return QDomElement();
    }
}

