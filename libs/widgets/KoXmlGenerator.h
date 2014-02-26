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

class KoXmlGenerator
{
protected:

    ///@description The virtual XML file.
    QDomDocument xmlDocument;

    ///@description The element considered as root of the XML tree of the document
    QDomElement root;

    ///@description The device if initialized using second constructor
    QIODevice *device;

public:
    /**
     * Constructor
     * Create a virtual XML file.
     * @param xmlFileName the name of the XML file to be created
     */
    KoXmlGenerator(QString xmlFileName);

    /**
     * Constructor
     * Create a virtual XML file.
     * @param data the name of the XML file to be created
     * @param xmlFileName the name of the XML file to be created
     */

    KoXmlGenerator(QByteArray data,QString xmlFileName);

    /**
     * Constructor
     * Create a virtual XML file, extracting data from an existing device.
     * @param device the virtual device used as data for Xml file.
     * @param rootName the name of the root tag that should be contained in the file.
     */
    KoXmlGenerator(QIODevice *device,QString rootName="");

    ///Destructor
    virtual ~KoXmlGenerator();

    ///@return the data contained in the XML Document
    QByteArray toByteArray();

    ///@return the name of the XML document
    QString getName();

    ///Check/sort the file to be easily comprehensible.
    virtual void checkSort();

    /**
     * Append a tag, with or without text node, to the root of the document.
     * @param tagName the name of the tag to be added.
     * @param textValue the text linked to the tag.
     * @param emptyFile true if the file is empty
     * @return the element corresponding to the created tag.
     */
    virtual QDomElement addTag(QString tagName,QString textValue="",bool emptyFile=false);

    /**
     * Remove the first tag having the same name, and same text value if defined.
     * @param tagName the name of the tag to be removed.
     * @param textValue the text linked to the tag.
     * @return true if a tag has been removed, false otherwise.
     */
    bool removeFirstTag(QString tagName,QString textValue="");

    /**
     * Remove the first tag having the same name, and same text value if defined.
     * @param tagName the name of the tag to be removed.
     * @param textValue the text linked to the tag.
     * @return true if a tag has been removed, false otherwise.
     */
    bool removeFirstTag(QString tagName,QString attName,QString attValue);

    /**
     * Remove all the tags having the same name.
     * @param tagName the name of the tag to be removed.
     */
    void removeTag(QString);

    /**
     * Search in the list the first tag having the right text value.
     * @param tagList the list of the tags to be analyzed.
     * @param textValue the text to be found in the tag.
     * @return the first node found.
     */
    QDomNode searchValue(QDomNodeList tagList,QString);

    ///Show the XML data of the document in cout.
    void show();

    ///@return the string containing the whole XML data of the document.
    QString toString();

    /**
     * Print the XML data from virtual document to XML file on disk.
     * @return the name of the file generated
     */
    QString toFile();
};

#endif // KOXMLGENERATOR_H

