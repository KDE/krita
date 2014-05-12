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

#include <QList>
#include "KoXmlResourceBundleMeta.h"

KoXmlResourceBundleMeta::KoXmlResourceBundleMeta(QString xmlName)
    : KoXmlGenerator(xmlName)
{
    m_root = m_xmlDocument.createElement("package");
    m_xmlDocument.appendChild(m_root);
}

KoXmlResourceBundleMeta::KoXmlResourceBundleMeta(QIODevice *device): KoXmlGenerator(device)
{

}

KoXmlResourceBundleMeta::~KoXmlResourceBundleMeta()
{

}

void KoXmlResourceBundleMeta::setMeta(QString name, QString author, QString license, QString website , QString description)
{
    addTag("name", name, true);
    addTag("author", author, true);
    addTag("license", license, true);
    addTag("website", website, true);
    addTag("description", description, true);
}

KoXmlResourceBundleMeta::TagEnum KoXmlResourceBundleMeta::getTagEnumValue(QString tagName)
{
    if (tagName == "name") {
        return Name;
    } else if (tagName == "filename") {
        return Filename;
    } else if (tagName == "author") {
        return Author;
    } else if (tagName == "email") {
        return Email;
    } else if (tagName == "created") {
        return Created;
    } else if (tagName == "license") {
        return License;
    } else if (tagName == "updated") {
        return Updated;
    } else if (tagName == "description") {
        return Description;
    } else if (tagName == "website") {
        return Website;
    } else if (tagName == "tag") {
        return Tag;
    } else {
        return Other;
    }
}

void KoXmlResourceBundleMeta::checkSort()
{
    QDomNode prevNode;
    QDomNode currentNode = m_root.firstChild();
    QDomNode nextNode = currentNode.nextSibling();

    TagEnum currentName;
    TagEnum lastOk = getTagEnumValue(currentNode.toElement().tagName());

    while (!nextNode.isNull()) {
        currentName = getTagEnumValue(nextNode.toElement().tagName());

        if (lastOk > currentName) {
            prevNode = currentNode.previousSibling();
            while (getTagEnumValue(prevNode.toElement().tagName()) > currentName && !prevNode.isNull()) {
                prevNode = prevNode.previousSibling();
            }

            if (currentName != Tag && currentName != Other && getTagEnumValue(prevNode.toElement().tagName()) == currentName) {
                m_root.removeChild(nextNode);
            } else if (prevNode.isNull()) {
                m_root.insertBefore(nextNode, prevNode);
            } else {
                m_root.insertAfter(nextNode, prevNode);
            }
        } else if (lastOk == currentName && currentName != Tag && currentName != Other) {
            m_root.removeChild(nextNode);
        } else {
            lastOk = currentName;
            currentNode = nextNode;
        }

        nextNode = currentNode.nextSibling();
    }
}

//TODO Vérifier si on peut pas simplifier cette méthode
QDomElement KoXmlResourceBundleMeta::addTag(QString tagName, QString textValue, bool emptyFile)
{
    tagName = tagName.toLower();

    int tagEnumValue = getTagEnumValue(tagName);

    if (tagEnumValue == Other && textValue == "") {
        return QDomElement();
    } else {
        QDomNodeList tagList = m_xmlDocument.elementsByTagName(tagName);
        QDomNode currentNode = tagList.item(0);

        if (emptyFile || tagEnumValue == Other || currentNode.isNull() || (tagEnumValue == Tag &&
                searchValue(tagList, textValue).isNull())) {
            if (textValue != "") {
                QDomElement child = m_xmlDocument.createElement(tagName);
                m_root.appendChild(child);
                child.appendChild(m_xmlDocument.createTextNode(textValue));
                return child;
            } else {
                return QDomElement();
            }
        } else if (tagEnumValue != Tag) {
            if (textValue == "") {
                m_root.removeChild(currentNode);
            } else {
                currentNode.firstChild().setNodeValue(textValue);
            }
            return currentNode.toElement();
        } else {
            return QDomElement();
        }
    }
}

void KoXmlResourceBundleMeta::addTags(QList<QString> list)
{
    QString currentTag;
    QDomNodeList tagList = m_xmlDocument.elementsByTagName("tag");

    for (int i = 0; i < list.size(); i++) {
        currentTag = list.at(i);
        if (currentTag != "" && currentTag.remove(" ") != "" && searchValue(tagList, currentTag).isNull()) {
            addTag("tag", list.at(i));
        }
    }
}

QString KoXmlResourceBundleMeta::getPackName()
{
    QDomNodeList tagList = m_xmlDocument.elementsByTagName("name");

    if (tagList.size() == 1) {
        return tagList.at(0).firstChild().toText().data();
    } else {
        return "";
    }
}

QString KoXmlResourceBundleMeta::getPackFileName()
{
    QDomNodeList tagList = m_xmlDocument.elementsByTagName("filename");

    if (tagList.size() == 1) {
        return tagList.at(0).firstChild().toText().data();
    } else {
        return "";
    }
}

QList<QString> KoXmlResourceBundleMeta::getTagsList()
{
    QString currentTextValue;
    QList<QString> result;
    QDomNodeList tagList = m_xmlDocument.elementsByTagName("tag");

    for (int i = 0; i < tagList.size(); i++) {
        currentTextValue = tagList.at(i).firstChild().toText().data();
        if (!result.contains(currentTextValue)) {
            result.push_front(currentTextValue);
        }
    }
    return result;
}
