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

#include <QFile>
#include <QTextStream>
#include "KoXmlGenerator.h"
#include <cstdlib>
#include <iostream>
using namespace std;


KoXmlGenerator::KoXmlGenerator(QString xmlFileName):m_xmlDocument(xmlFileName)
{
    this->m_device=0;
    m_root=m_xmlDocument.documentElement();
}

KoXmlGenerator::KoXmlGenerator(QByteArray data)
{
    this->m_device=0;
    if (!m_xmlDocument.setContent(data)) {
        exit(1);
    }
    else {
        m_root=m_xmlDocument.documentElement();
    }
}

KoXmlGenerator::KoXmlGenerator(QIODevice *device)
{
    this->m_device=device;

    if (!device->isOpen() && !device->open(QIODevice::ReadOnly)) {
        exit(1);
    }

    if (!m_xmlDocument.setContent(device)) {
        device->close();
        exit(1);
    }
    else {
        device->close();
        m_root=m_xmlDocument.documentElement();
    }
}

KoXmlGenerator::~KoXmlGenerator()
{

}

QByteArray KoXmlGenerator::toByteArray()
{
    return m_xmlDocument.toByteArray();
}

QString KoXmlGenerator::getName()
{
	return m_xmlDocument.doctype().name();
}

void KoXmlGenerator::checkSort()
{

}

QString KoXmlGenerator::getValue(QString tagName)
{
    QDomText currentTextValue;
    QDomNodeList list=m_xmlDocument.elementsByTagName(tagName);

    if (list.isEmpty() || (currentTextValue=list.at(0).firstChild().toText()).isNull()) {
        return QString();
    }
    else {
        return currentTextValue.data();
    }
}

QDomElement KoXmlGenerator::addTag(QString tagName,QString textValue,bool emptyFile)
{
    Q_UNUSED(emptyFile);
    QDomElement child = m_xmlDocument.createElement(tagName);
    m_root.appendChild(child);

    if (textValue!="") {
        child.appendChild(m_xmlDocument.createTextNode(textValue));
    }

    return child;
}

bool KoXmlGenerator::removeFirstTag(QString tagName,QString textValue)
{
    QDomNodeList tagList=m_xmlDocument.elementsByTagName(tagName);

    if (tagList.isEmpty()) {
        return false;
    }
    else {
        if (textValue=="") {
            m_root.removeChild(tagList.item(0));
            return true;
        }
        else {
            QDomNode currentNode=searchValue(tagList,textValue);
            if (currentNode.isNull()) {
                return false;
            }
            else {
                m_root.removeChild(currentNode);
                return true;
            }
        }
    }
}

bool KoXmlGenerator::removeFirstTag(QString tagName,QString attName,QString attValue)
{
    QDomNodeList tagList=m_xmlDocument.elementsByTagName(tagName);

    if (tagList.isEmpty() || attName=="" || attValue=="") {
        return false;
    }
    else {
        QDomNode currentNode=searchValue(tagList,attName,attValue);
        if (currentNode.isNull()) {
            return false;
        }
        else {
            currentNode.parentNode().removeChild(currentNode);
            return true;
        }
    }
}

void KoXmlGenerator::removeTag(QString tagName)
{
    QDomNodeList tagList=m_xmlDocument.elementsByTagName(tagName);

    if (tagList.isEmpty()) {
        return;
    }
    else {
        for (int i=0;i<tagList.size();i++) {
            m_root.removeChild(tagList.item(i));
        }
    }
}

QDomNode KoXmlGenerator::searchValue(QDomNodeList tagList,QString textValue)
{
    QDomNode currentNode;

    for (int i=0;i<tagList.size();i++) {
        currentNode=tagList.item(i);
        if (currentNode.firstChild().toText().data()==textValue) {
            return currentNode;
        }
    }

    return QDomNode();
}

QDomNode KoXmlGenerator::searchValue(QDomNodeList tagList,QString attName,QString attValue)
{
    QDomNode currentNode;

    for (int i=0;i<tagList.size();i++) {
        currentNode=tagList.item(i);
        if (currentNode.toElement().attributeNode(attName).value()==attValue) {
            return currentNode;
        }
    }
    return QDomNode();
}

void KoXmlGenerator::show()
{
    cout<<qPrintable(toString())<<endl;
}

QString KoXmlGenerator::toString()
{
    return m_xmlDocument.toString();
}

QString KoXmlGenerator::toFile()
{
    QString xmlName=getName().append(".xml");

    if (m_device!=0) {
        xmlName=((QFile*)m_device)->fileName();
    }
    else {
        m_device=new QFile(xmlName);
    }

    if (!m_device->open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Truncate)) {
        return "";
    }
    else {
        QTextStream flux(m_device);

        flux.setCodec("UTF-8");
        flux<<toString();
        m_device->close();
        return xmlName;
    }
}

