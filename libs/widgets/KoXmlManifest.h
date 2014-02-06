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

#ifndef KOXMLMANIFEST_H
#define KOXMLMANIFEST_H

#include "KoXmlGenerator.h"

class KoXmlManifest: public KoXmlGenerator
{
private:
    /**
     * Contains all the resource types.
     * Other means all other values that are not listed before.
     * @description Allows to sort correctly the XML document.
     */
    enum TagEnum {Brush=0, Gradient, Paintop, Palette, Pattern,
                  Template, Workspace, Reference, Other};

public:
    /**
     * Constructor
     * Create a virtual XML file.
     * @param xmlName the name of the XML file to be created
     */
    KoXmlManifest(QString xmlName="manifest");
    
    /**
     * Constructor
     * Create a virtual XML file, extracting data from an existing file.
     * @param file the virtual file used as data for Xml file.
     */
    KoXmlManifest(QFile *file);
    
    ///Destructor
    ~KoXmlManifest();

    /**
     * @param tagName the name of the tag
     * @return the value from TagEnum corresponding to the tag.
     */
    TagEnum getTagEnumValue(QString tagName);

    ///Check/sort the file to be easily comprehensible
    void checkSort();

    /**
     * Merge the data contained in two tags in only one tag.
     * @param dest the node that will contain the whole data
     * @param src the node that will be removed after merging
     */
    void merge(QDomNode dest,QDomNode src);

    /**
     * Add a file tag as a child of the fileType tag.
     * (ex : <brushes><file>file_Name</file></brushes>)
     * @param fileType the type of the file to be added (eq. name of the parent folder it is in : Brushes, Patterns...)
     * @param fileName the name of the file to be added.
     * @param emptyFile true if the file is empty
     * @return the element corresponding to the created tag.
     */
    QDomElement addTag(QString fileType,QString fileName,bool emptyFile=false);
 };


#endif // KOXMLMANIFEST_H

