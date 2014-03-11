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

#ifndef KOXMLGENERATOR_H
#define KOXMLGENERATOR_H

#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>
#include <QtXml/QDomNode>
#include <QtXml/QDomNodeList>

class QIODevice;

/**
 * @brief The KoXmlGenerator class
 * @details Allow to manage Xml file content
 */
class KoXmlGenerator
{
public:

    /**
     * @brief KoXmlGenerator : Ctor
     * @param xmlFileName the name of the XML file to be created
     */
    KoXmlGenerator(QString xmlFileName);

    /**
     * @brief KoXmlGenerator : Ctor
     * @param data the QByteArray containing Xml data
     */
    KoXmlGenerator(QByteArray data);

    /**
     * @brief KoXmlGenerator : Ctor
     * @param device the device associated to Xml data
     */
    KoXmlGenerator(QIODevice *device);

    /**
     * @brief ~KoXmlGenerator : Dtor
     */
    virtual ~KoXmlGenerator();

    /**
     * @brief toByteArray
     * @return the data contained in the XML document
     */
    QByteArray toByteArray();

    /**
     * @brief getName
     * @return the name of the XML document
     */
    QString getName();

    /**
     * @brief checkSort : Check/sort the file to be easily comprehensible.
     */
    virtual void checkSort();

    /**
     * @brief addTag : Append a tag, with or without text node, to the root of the document
     * @param tagName the name of the tag to be added
     * @param textValue the text linked to the tag
     * @param emptyFile true if the file is empty
     * @return the element corresponding to the created tag.
     */
    virtual QDomElement addTag(QString tagName,QString textValue="",bool emptyFile=false);

    /**
     * @brief removeFirstTag : Remove the first tag having same name (and same textvalue)
     * @param tagName the name of the tag to be removed
     * @param textValue the text linked to the tag
     * @return true if succeed, false otherwise.
     */
    bool removeFirstTag(QString tagName,QString textValue="");

    /**
     * @brief removeFirstTag : Remove the first tag having same name and attribute
     * @param tagName the name of the tag to be removed.
     * @param attName the name of the attribute
     * @param attValue the value of the attribute
     * @return true if succeed, false otherwise.
     */
    bool removeFirstTag(QString tagName,QString attName,QString attValue);

    /**
     * @brief removeTag : Remove all the tags having the same name.
     * @param tagName the name of the tag to be removed.
     */
    void removeTag(QString tagName);

    /**
     * @brief searchValue : Search in the list the first tag having the right textvalue
     * @param tagList the list of the tags to be analyzed
     * @param textValue the text to be found in the tag
     * @return the first node found, null node otherwise.
     */
    QDomNode searchValue(QDomNodeList tagList,QString textValue);

    /**
     * @brief searchValue : Search in the list the first tag having the right attribute value
     * @param tagList the list of the tags to be analyzed
     * @param attName the name of the attribute
     * @param attValue the value of the attribute
     * @return the first node found, null node otherwise.
     */
    QDomNode searchValue(QDomNodeList tagList,QString attName,QString attValue);

    /**
     * @brief show : Show the XML data of the document in cout.
     */
    void show();

    /**
     * @brief toString
     * @return the string containing the whole XML data of the document.
     */
    QString toString();

    /**
     * @brief toFile : Print the XML data from virtual document to XML file on disk.
     * @return the name of the file generated
     */
    QString toFile();

protected:

    /**
     * @description The virtual XML file.
     */
    QDomDocument xmlDocument;

    /**
     * @description The element considered as root of the XML tree of the document
     */
    QDomElement root;

    /**
     * @description The device if initialized using second constructor
     */
    QIODevice *device;
};

#endif // KOXMLGENERATOR_H

