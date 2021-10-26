/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2014 Victor Lafon <metabolic.ewilan@hotmail.fr>

   SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KORESOURCEBUNDLEMANIFEST_H
#define KORESOURCEBUNDLEMANIFEST_H

#include <QString>
#include <QPair>
#include <QMap>
#include <QMultiMap>

#include <kritaresources_export.h>

class QIODevice;

class KRITARESOURCES_EXPORT  KoResourceBundleManifest
{
public:

    struct ResourceReference {

        ResourceReference() {}

        ResourceReference(const QString &_resourcePath, const QList<QString> &_tagList, const QString &_fileTypeName,
                          const QString &_md5, const int _resourceId = -1, const QString _filenameInBundle = "") {
            resourcePath = _resourcePath;
            tagList = _tagList;
            fileTypeName = _fileTypeName;
            md5sum = _md5;
            resourceId = _resourceId;
            // only necessary to provide if filename in bundle is different from the filename on disk
            // for example for versioned resources
            filenameInBundle = _filenameInBundle.isEmpty() ? resourcePath : _filenameInBundle;
        }

        QString resourcePath;
        QList<QString> tagList;
        QString fileTypeName;
        QString md5sum;
        int resourceId;
        QString filenameInBundle;
    };

    /**
     * @brief ResourceBundleManifest : Ctor
     * @param xmlName the name of the XML file to be created
     */
    KoResourceBundleManifest();

    /**
     * @brief ~ResourceBundleManifest : Dtor
     */
    virtual ~KoResourceBundleManifest();


    /**
     * @brief load the ResourceBundleManifest from the given device
     */
    bool load(QIODevice *device);

    /**
     * @brief save the ResourceBundleManifest to the given device
     */
    bool save(QIODevice *device);

    /**
     * @brief addTag : Add a file tag as a child of the fileType tag.
     * @param fileType the type of the file to be added
     * @param fileName the name of the file to be added
     * @param emptyFile true if the file is empty
     * @return the element corresponding to the created tag.
     */
    void addResource(const QString &fileType, const QString &fileName, const QStringList &tagFileList, const QString &md5, const int resourceId = -1, const QString filenameInBundle = "");
    void removeResource(ResourceReference &resource);

    QStringList types() const;

    QStringList tags() const;

    QList<ResourceReference> files(const QString &type = QString()) const;

    /**
     * @brief removeFile : remove a file from the manifest
     * @param fileName : the name of the file to be removed
     * @return the list of resource tags to be removed from meta file.
     */
    void removeFile(QString fileName);

private:
    QMap<QString, QMap<QString, ResourceReference> > m_resources;
};


#endif 
