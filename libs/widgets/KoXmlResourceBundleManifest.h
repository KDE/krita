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

#ifndef KOXMLRESOURCEBUNDLEMANIFEST_H
#define KOXMLRESOURCEBUNDLEMANIFEST_H

#include "KoXmlGenerator.h"

class KoResource;

class KoXmlResourceBundleManifest: public KoXmlGenerator
{
private:
    /**
     * @brief The TagEnum enum : Contains all the resource types.
     * @description Allows to sort correctly the XML document.
     * @details Other means all other values that are not listed before.
     */
    enum TagEnum {Brush=0, Gradient, Paintop, Palette, Pattern,
                  Template, Workspace, Reference, Other};

public:
    /**
     * @brief KoXmlResourceBundleManifest : Ctor
     * @param xmlName the name of the XML file to be created
     */
    KoXmlResourceBundleManifest(QString xmlName="manifest");

    /**
     * @brief KoXmlResourceBundleManifest : Ctor
     * @param data the QByteArray containing Xml data
     */
    KoXmlResourceBundleManifest(QByteArray data);

    /**
     * @brief KoXmlResourceBundleManifest
     * @param device the device associated to Xml data
     */
    KoXmlResourceBundleManifest(QIODevice *device);
    
    /**
     * @brief ~KoXmlResourceBundleManifest : Dtor
     */
    virtual ~KoXmlResourceBundleManifest();

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
     * @brief merge : Merge the data contained in two tags in only one tag
     * @param dest the node that will contain the whole data
     * @param src the node that will be removed after merging
     */
    void merge(QDomNode dest,QDomNode src);

    /**
     * @brief addTag : Add a file tag as a child of the fileType tag.
     * @param fileType the type of the file to be added
     * @param fileName the name of the file to be added
     * @param emptyFile true if the file is empty
     * @return the element corresponding to the created tag.
     */
    QDomElement addTag(QString fileType,QString fileName,bool emptyFile=false);

    /**
     * @brief getFileList
     * @return the list of the files enumerared in the XML
     */
    QList<QString> getFileList();

    /**
     * @brief getFilesToExtract
     * @return the list of the files to be extracted
     */
    QList<QString> getFilesToExtract();

    /**
     * @brief getDirList
     * @return the list of the directories containing the files of the Xml document
     */
    QList<QString> getDirList();

    /**
     * @brief addTag : Add a tag as the child of the node
     * @param parent the parent node of the tag
     * @param tagName the name of the tag to be added
     * @param textValue the text value associated to the tag
     * @return the child as a QDomElement
     */
    QDomElement addTag(QDomElement parent,QString tagName,QString textValue);

    /**
     * @brief importFileTags : import the resource tags associated to the file to the manifest
     * @param parent the parent node of the tag
     * @param fileTypeName the type of the file
     * @param fileName the name of the file
     */
    void importFileTags(QDomElement parent,QString fileTypeName,QString fileName);

    /**
     * @brief getTagList
     * @return the list of all resource tags contained in the manifest
     */
    QList<QString> getTagList();

    /**
     * @brief removeFile : remove a file from the manifest
     * @param fileName : the name of the file to be removed
     * @return the list of resource tags to be removed from meta file.
     */
    QList<QString> removeFile(QString fileName);

    /**
     * @brief exportTags : export file tags to the right Krita xml files
     */
    void exportTags();
 };


#endif // KOXMLRESOURCEBUNDLEMANIFEST_H

