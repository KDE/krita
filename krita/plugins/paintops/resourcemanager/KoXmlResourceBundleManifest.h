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

#include <QPair>
#include <QMap>
#include <QMultiMap>
#include "krita_export.h"

class KoResource;

class  KoXmlResourceBundleManifest
{
public:

    struct ResourceReference {

        ResourceReference(const QString &_resourcePath, const QList<QString> &_tagList, const QByteArray &_md5) {
            resourcePath = _resourcePath;
            tagList = _tagList;
            md5sum = _md5;
        }

        QString resourcePath;
        QList<QString> tagList;
        QByteArray md5sum;
    };

    /**
     * @brief KoXmlResourceBundleManifest : Ctor
     * @param xmlName the name of the XML file to be created
     */
    KoXmlResourceBundleManifest();

    /**
     * @brief ~KoXmlResourceBundleManifest : Dtor
     */
    virtual ~KoXmlResourceBundleManifest();


    /**
     * @brief load the KoXmlResourceBundleManifest from the given device
     */
    bool load(QIODevice *device);

    /**
     * @brief save the KoXmlResourceBundleManifest to the given device
     */
    bool save(QIODevice *device);

    /**
     * @brief addTag : Add a file tag as a child of the fileType tag.
     * @param fileType the type of the file to be added
     * @param fileName the name of the file to be added
     * @param emptyFile true if the file is empty
     * @return the element corresponding to the created tag.
     */
    void addResource(const QString &fileType, const QString &fileName, const QStringList &tagFileList, const QByteArray &md5);


    QStringList types() const;

    QStringList tags() const;

    QList<ResourceReference> files(const QString &type = QString()) const;

    /**
     * @brief getFileList
     * @return the list of the files enumerated in the XML
     */
    QList<QString> getFileList(QString kritaPath, bool firstBuild);

    /**
     * @brief getFilesToExtract
     * @return the list of the files to be extracted
     */
    QMap<QString, QString> getFilesToExtract();

    /**
     * @brief getDirList
     * @return the list of the directories containing the files of the Xml document
     */
    QList<QString> getDirList();
    /**
     * @brief removeFile : remove a file from the manifest
     * @param fileName : the name of the file to be removed
     * @return the list of resource tags to be removed from meta file.
     */
    void removeFile(QString fileName);

    /**
     * @brief exportTags : export file tags to the right Krita xml files
     */
    void exportTags();

    /**
     * @brief install : add "installed" tag in the XML file if necessary
     */
    void install();

    /**
     * @brief uninstall : remove "installed" tag in the XML file if necessary
     */
    void uninstall();

    /**
     * @brief isInstalled
     * @return true if the "installed" tag is in the XML file, false otherwise
     */
    bool isInstalled();

    void rename(QString newName);

    void updateFilePaths(QString kritaPath, QString bundleName);

private:
    QMap<QString, QMap<QString, ResourceReference> > m_resources;
};


#endif // KOXMLRESOURCEBUNDLEMANIFEST_H

