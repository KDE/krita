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

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include "KoXmlGenerator.h"
#include <cstdlib>
#include <iostream>
using namespace std;


KoXmlGenerator::KoXmlGenerator(QString xmlFileName):xmlDocument(xmlFileName)
{
    this->device=0;
    root=xmlDocument.documentElement();
}

KoXmlGenerator::KoXmlGenerator(QByteArray data)
{
    this->device=0;
    if (!xmlDocument.setContent(data)) {
        exit(1);
    }
    else {
        root=xmlDocument.documentElement();
    }
}

KoXmlGenerator::KoXmlGenerator(QIODevice *device)
{
    this->device=device;

    if (!device->isOpen() && !device->open(QIODevice::ReadOnly)) {
        exit(1);
    }

    if (!xmlDocument.setContent(device)) {
        device->close();
        exit(1);
    }
    else {
        device->close();
        root=xmlDocument.documentElement();
    }
}

KoXmlGenerator::~KoXmlGenerator()
{

}

QByteArray KoXmlGenerator::toByteArray()
{
    return xmlDocument.toByteArray();
}

QString KoXmlGenerator::getName()
{
	return xmlDocument.doctype().name();
}

void KoXmlGenerator::checkSort()
{

}

QDomElement KoXmlGenerator::addTag(QString tagName,QString textValue,bool emptyFile)
{
    Q_UNUSED(emptyFile);
    QDomElement child = xmlDocument.createElement(tagName);
    root.appendChild(child);

    if (textValue!="") {
        child.appendChild(xmlDocument.createTextNode(textValue));
    }

    return child;
}

bool KoXmlGenerator::removeFirstTag(QString tagName,QString textValue)
{
    QDomNodeList tagList=xmlDocument.elementsByTagName(tagName);

    if (tagList.isEmpty()) {
        return false;
    }
    else {
        if (textValue=="") {
            root.removeChild(tagList.item(0));
            return true;
        }
        else {
            QDomNode node=searchValue(tagList,textValue);
            if (node.isNull()) {
                return false;
            }
            else {
                root.removeChild(node);
                return true;
            }
        }
    }
}

bool KoXmlGenerator::removeFirstTag(QString tagName,QString attName,QString attValue)
{
    QDomNodeList tagList=xmlDocument.elementsByTagName(tagName);

    if (tagList.isEmpty() || attName=="" || attValue=="") {
        return false;
    }
    else {
        QDomNode node=searchValue(tagList,attName,attValue);
        if (node.isNull()) {
            return false;
        }
        else {
            node.parentNode().removeChild(node);
            return true;
        }
    }
}

void KoXmlGenerator::removeTag(QString tagName)
{
    QDomNodeList tagList=xmlDocument.elementsByTagName(tagName);

    if (tagList.isEmpty()) {
        return;
    }
    else {
        for (int i=0;i<tagList.size();i++) {
            root.removeChild(tagList.item(i));
        }
    }
}

QDomNode KoXmlGenerator::searchValue(QDomNodeList tagList,QString textValue)
{
    QDomNode node;

    for (int i=0;i<tagList.size();i++) {
        node=tagList.item(i);
        if (node.firstChild().toText().data()==textValue) {
            return node;
        }
    }

    return QDomNode();
}

QDomNode KoXmlGenerator::searchValue(QDomNodeList tagList,QString attName,QString attValue)
{
    QDomNode node;

    for (int i=0;i<tagList.size();i++) {
        node=tagList.item(i);
        if (node.toElement().attributeNode(attName).value()==attValue) {
            return node;
        }
    }
    return QDomNode();
}

void KoXmlGenerator::show()
{
    cout<<qPrintable(toString());
}

QString KoXmlGenerator::toString()
{
    return xmlDocument.toString();
}

QString KoXmlGenerator::toFile()
{
    QString xmlName=getName().append(".xml");

    if (device!=0) {
        xmlName=((QFile*)device)->fileName();
    }
    else {
        device=new QFile(xmlName);
    }

    if (!device->open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Truncate)) {
        return "";
    }
    else {
        QTextStream flux(device);

        flux.setCodec("UTF-8");
        flux<<toString();
        device->close();
        return xmlName;
    }
}

