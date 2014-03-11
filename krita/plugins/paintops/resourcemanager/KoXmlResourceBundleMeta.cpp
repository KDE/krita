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

#include <QtCore/QList>
#include "KoXmlResourceBundleMeta.h"
#include <iostream>
using namespace std;

KoXmlResourceBundleMeta::KoXmlResourceBundleMeta(QString xmlName):KoXmlGenerator(xmlName)
{
    root=xmlDocument.createElement("package");
    xmlDocument.appendChild(root);
}

KoXmlResourceBundleMeta::KoXmlResourceBundleMeta(QIODevice *device):KoXmlGenerator(device)
{

}

KoXmlResourceBundleMeta::KoXmlResourceBundleMeta(QByteArray data):KoXmlGenerator(data)
{

}

KoXmlResourceBundleMeta::KoXmlResourceBundleMeta(QString name,QString license,QString xmlName):KoXmlGenerator(xmlName)
{
    root=xmlDocument.createElement("package");
    xmlDocument.appendChild(root);

    addTag("name",name,true);
    addTag("license",license,true);
}

KoXmlResourceBundleMeta::KoXmlResourceBundleMeta(QString* resourceTagList,QString name,
                   QString license,QString description,QString author,QString created,
                   QString modified,QString xmlName):KoXmlGenerator(xmlName)
{
    root=xmlDocument.createElement("package");
    xmlDocument.appendChild(root);

    addTag("name",name,true);
    addTag("author",author,true);
    addTag("created",created,true);
    addTag("license",license,true);
    addTag("modified",modified,true);
    addTag("description",description,true);

    for (int i=0;i<resourceTagList->length();i++) {
        addTag("tag",resourceTagList[i],true);
    }
}

KoXmlResourceBundleMeta::~KoXmlResourceBundleMeta()
{

}

KoXmlResourceBundleMeta::TagEnum KoXmlResourceBundleMeta::getTagEnumValue(QString tagName)
{
    if (tagName=="name") {
            return Name;
    }
    else if (tagName=="author") {
        return Author;
    }
    else if (tagName=="created") {
        return Created;
    }
    else if (tagName=="license") {
        return License;
    }
    else if (tagName=="last-modified") {
        return Modified;
    }
    else if (tagName=="description") {
        return Description;
    }
    else if (tagName=="tag") {
        return Tag;
    }
    else {
        return Other;
    }
}

void KoXmlResourceBundleMeta::checkSort()
{
    QDomNode prev;
    QDomNode current = root.firstChild();
    QDomNode next = current.nextSibling();

    TagEnum name;
    TagEnum lastOk=getTagEnumValue(current.toElement().tagName());

    while (!next.isNull()) {
        name=getTagEnumValue(next.toElement().tagName());

        if (lastOk>name) {
            prev=current.previousSibling();
            while (getTagEnumValue(prev.toElement().tagName())>name && !prev.isNull()) {
                  prev=prev.previousSibling();
            }

            if (name!=Tag && name!=Other && getTagEnumValue(prev.toElement().tagName())==name) {
                root.removeChild(next);
            }
            else if (prev.isNull()){
                root.insertBefore(next,prev);
            }
            else {
                root.insertAfter(next,prev);
            }
        }
        else if (lastOk==name && name!=Tag && name!=Other) {
            root.removeChild(next);
        }
        else {
            lastOk=name;
            current=next;
        }

        next=current.nextSibling();
    }
}

//TODO Vérifier si on peut pas simplifier cette méthode
QDomElement KoXmlResourceBundleMeta::addTag(QString tagName,QString textValue,bool emptyFile)
{
    tagName=tagName.toLower();

    int tagEnumValue=getTagEnumValue(tagName);

    if (tagEnumValue==Other && textValue=="") {
        return QDomElement();
    }
    else {
        QDomNodeList tagList=xmlDocument.elementsByTagName(tagName);
        QDomNode node=tagList.item(0);

        if (emptyFile || tagEnumValue==Other || node.isNull() || (tagEnumValue==Tag &&
             searchValue(tagList,textValue).isNull())) {
            if (textValue!="") {
                QDomElement child = xmlDocument.createElement(tagName);
                root.appendChild(child);
                child.appendChild(xmlDocument.createTextNode(textValue));
                return child;
            }
            else {
                return QDomElement();
            }
        }
        else if (tagEnumValue!=Tag) {
            if (textValue=="") {
                root.removeChild(node);
            }
            else {
                node.firstChild().setNodeValue(textValue);
            }
            return node.toElement();
        }
        else {
            return QDomElement();
        }
    }
}

void KoXmlResourceBundleMeta::addTags(QList<QString> list)
{
    QDomNodeList tagList=xmlDocument.elementsByTagName("tag");
    QString prov;

    for (int i=0;i<list.size();i++) {
        prov=list.at(i);
        if (prov != "" && searchValue(tagList,prov).isNull()) {
            root.appendChild(xmlDocument.createElement("tag").appendChild(xmlDocument.createTextNode(prov)));
        }
    }
}

QString KoXmlResourceBundleMeta::getPackName(){
    QDomNodeList tagList=xmlDocument.elementsByTagName("name");
    if (tagList.size() == 1) {
        return tagList.at(0).firstChild().toText().data();
    }
    else {
        return "";
    }
}

QString KoXmlResourceBundleMeta::getShortPackName(){
    return getPackName().section('/',getPackName().count('/')).remove(".zip");
}
