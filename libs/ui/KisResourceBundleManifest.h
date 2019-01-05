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

#include <QString>
#include <QPair>
#include <QMap>
#include <QMultiMap>

#include <kritaui_export.h>

class QIODevice;

class KRITAUI_EXPORT  KisResourceBundleManifest
{
public:

    struct ResourceReference {

        ResourceReference(const QString &_resourcePath, const QList<QString> &_tagList, const QString &_fileTypeName, const QByteArray &_md5) {
            resourcePath = _resourcePath;
            tagList = _tagList;
            fileTypeName = _fileTypeName;
            md5sum = _md5;
        }

        QString resourcePath;
        QList<QString> tagList;
        QString fileTypeName;
        QByteArray md5sum;
    };

    /**
     * @brief ResourceBundleManifest : Ctor
     */
    KisResourceBundleManifest();

    /**
     * @brief ~ResourceBundleManifest : Dtor
     */
    virtual ~KisResourceBundleManifest();


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
     * @param tagFileList list of the tags
     * @param md5 MD5 checksum
     * @return the element corresponding to the created tag.
     */
    void addResource(const QString &fileType, const QString &fileName, const QStringList &tagFileList, const QByteArray &md5);


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


#endif // KOXMLRESOURCEBUNDLEMANIFEST_H

