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

#include "KoXmlGenerator.h"
#include <QtXml/QDomElement>
#include <QFile>
#include <QTextStream>
#include <cstdlib>
#include <iostream>
using namespace std;


KoXmlGenerator::KoXmlGenerator(QString xmlFileName):xmlDocument(xmlFileName)
{
    root=xmlDocument.documentElement();
    this->device=0;
}

KoXmlGenerator::KoXmlGenerator(QIODevice *device,QString rootTag):xmlDocument(((QFile*)device)->fileName().section('.',0,0))
{
    this->device=device;

    if (!device->open(QIODevice::ReadOnly)) {
        exit(1);
    }

    if (!xmlDocument.setContent(device)) {
        device->close();
        exit(1);
    }
    else {
        device->close();
        if (rootTag!="") {
            QDomNodeList rootList=xmlDocument.elementsByTagName(rootTag);
            if (rootList.size()!=1) {
                exit(2);
            }
            else {
                root=rootList.item(0).toElement();
            }
        }
        else {
            root=xmlDocument.documentElement();	//TODO Check that it's the right root
        }
    }
}

KoXmlGenerator::~KoXmlGenerator()
{

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

