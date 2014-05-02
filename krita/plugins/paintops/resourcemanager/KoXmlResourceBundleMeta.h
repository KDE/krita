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
#include "krita_export.h"

class KRITAUI_EXPORT KoXmlResourceBundleMeta: public KoXmlGenerator
{
private:
    /**
     * @brief The TagEnum enum : Contains all the metadata types.
     * @description Allows to sort correctly the XML document.
     * @details Other means all other values that are not listed before.
     */
    enum TagEnum {Name=0, Filename, Author, Created, License, Updated, Description, Website, Tag, Other};

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
     * @brief ~KoXmlResourceBundleMeta : Dtor
     */
    virtual ~KoXmlResourceBundleMeta();

    /**
     * @brief getTagEnumValue
     * @param tagName the name of the tag
     * @return the value from TagEnum corresponding to the tag.
     */
    static TagEnum getTagEnumValue(QString tagName);

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

    QList<QString> getTagsList();

    /**
     * @brief getPackName
     * @return the name of the resource pack
     */
    QString getPackName();

    QString getPackFileName();

    void setMeta(QString name="", QString author="", QString license="", QString website="", QString description="");
};


#endif // KOXMLRESOURCEBUNDLEMETA_H

