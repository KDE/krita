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

#ifndef KOXMLMETA_H
#define KOXMLMETA_H

#include "KoXmlGenerator.h"

class KoXmlResourceBundleMeta: public KoXmlGenerator
{
private:
    /**
     * Contains all the types of metadata.
     * Other means all other values that are not listed before.
     * @description Allows to sort correctly the XML document.
     */
    enum TagEnum {Name=0, Author, Created, License, Modified, Description, Tag, Other};

public:
    /**
     * Constructor
     * Create a virtual XML file.
     * @param xmlName the name of the XML file to be created
     */
    KoXmlResourceBundleMeta(QString xmlName="meta");
    
    /**
     * Constructor
     * Create a virtual XML file, extracting data from an existing file.
     * @param file the virtual file used as data for Xml file.
     */
    KoXmlResourceBundleMeta(QIODevice *device);

    /**
     * Constructor
     * Create a virtual XML file, adding data from parameters.
     * @param name the name of the package
     * @param license the license of the package
     * @param xmlName the name of the XML file to be created
     */
    KoXmlResourceBundleMeta(QString name,QString license,QString fileName="meta");

    /**
     * Constructor
     * Create a virtual XML file, adding data from parameters.
     * @param resourceTagList the list of resource tags linked to the package
     * @param name the name of the package
     * @param license the license of the package
     * @param description the description of the content of the package
     * @param author the author of the package
     * @param created the creation date of the package
     * @param modified the last modification date of the package
     * @param xmlName the name of the XML file to be created
     */
    KoXmlResourceBundleMeta(QString *resourceTagList,QString name ,QString license,QString description="",
        QString author="",QString created="",QString modified="",QString xmlName="meta");

    ///Destructor
	~KoXmlResourceBundleMeta();

    /**
     * @param tagName the name of the tag
     * @return the value from TagEnum corresponding to the tag.
     */
    TagEnum getTagEnumValue(QString tagName);

    ///Check/sort the file to be easily comprehensible
    void checkSort();

    /**
     * Add a tag, with or without text node, as a child of the root of the document.
     * Replace the value is the tag already exist (tagNames other/tag excepted)
     * @param tagName the name of the tag to be added.
     * @param textValue the text linked to the tag.
     * @param emptyFile true if the file is empty
     * @return the element corresponding to the created tag.
     */
    QDomElement addTag(QString tagName,QString textValue="",bool empty=false);

    ///Add all the resource tags in the list if necessary
    void addTags(QList<QString> list);
};


#endif // KOXMLMETA_H

