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

#ifndef KOXMLRESOURCEBUNDLEMETA_H
#define KOXMLRESOURCEBUNDLEMETA_H

#include "KoXmlGenerator.h"

class KoXmlResourceBundleMeta: public KoXmlGenerator
{
private:
    /**
     * @brief The TagEnum enum : Contains all the metadata types.
     * @description Allows to sort correctly the XML document.
     * @details Other means all other values that are not listed before.
     */
    enum TagEnum {Name=0, Author, Created, License, Modified, Description, Tag, Other};

public:
    /**
     * @brief KoXmlResourceBundleMeta : Ctor
     * @param xmlName the name of the XML file to be created
     */
    KoXmlResourceBundleMeta(QString xmlName="meta");

    /**
     * @brief KoXmlResourceBundleMeta : Ctor
     * @param data the QByteArray containing Xml data
     */
    KoXmlResourceBundleMeta(QByteArray data);

    /**
     * @brief KoXmlResourceBundleMeta
     * @param device the device associated to Xml data
     */
    KoXmlResourceBundleMeta(QIODevice *device);

    /**
     * @brief KoXmlResourceBundleMeta : Ctor
     * @param name the name of the package
     * @param license the license of the package
     * @param fileName the name of the XML file to be created
     */
    KoXmlResourceBundleMeta(QString name,QString license,QString fileName="meta");

    /**
     * @brief KoXmlResourceBundleMeta : Ctor
     * @param resourceTagList the list of resource tags linked to the package
     * @param name name the name of the package
     * @param license license the license of the package
     * @param description the description of the content of the package
     * @param author author the author of the package
     * @param created the creation date of the package
     * @param modified the last modification date of the package
     * @param xmlName the name of the XML file to be created
     */
    KoXmlResourceBundleMeta(QString *resourceTagList,QString name ,QString license,QString description="",
        QString author="",QString created="",QString modified="",QString xmlName="meta");

    /**
     * @brief ~KoXmlResourceBundleMeta : Dtor
     */
    virtual ~KoXmlResourceBundleMeta();

    /**
     * @brief getTagEnumValue
     * @param tagName the name of the tag
     * @return the value from TagEnum corresponding to the tag.
     */
    TagEnum getTagEnumValue(QString tagName);

    /**
     * @brief checkSort : Check/sort the file to be easily comprehensible
     */
    void checkSort();

    /**
     * @brief addTag : Add a file tag as a child of the root of the document
     * @param tagName the name of the tag to be added
     * @param textValue the text linked to the tag
     * @param emptyFile true if the file is empty
     * @return the element corresponding to the created tag.
     */
    QDomElement addTag(QString tagName,QString textValue="",bool empty=false);

    /**
     * @brief addTags : Add all the resource tags in the list if necessary
     * @param list the list of resource tags to be added
     */
    void addTags(QList<QString> list);

    /**
     * @brief getPackName
     * @return the name of the resource pack
     */
    QString getPackName();

    /**
     * @brief getShortPackName
     * @return the short name of the resource pack
     */
    QString getShortPackName();
};


#endif // KOXMLRESOURCEBUNDLEMETA_H

