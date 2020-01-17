/*
 *  Copyright (c) 2014 Victor Lafon metabolic.ewilan@hotmail.fr
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KORESOURCEBUNDLE_H
#define KORESOURCEBUNDLE_H

#include <QSet>
#include <QList>
#include <QSharedPointer>

#include <KoXmlWriter.h>

#include <KoResource.h>
#include "KoResourceBundleManifest.h"

#include "kritaresources_export.h"

#include <KisTag.h>

class KoStore;

/**
 * @brief A KoResourceBundle is a zip file that contains resources,
 * some metadata about the creator of the bundle and a manifest file
 * that lists the contained resources.
 */
class KRITARESOURCES_EXPORT KoResourceBundle
{

public:
    /**
     * @brief ResourceBundle : Ctor * @param bundlePath the path of the bundle
     */
    KoResourceBundle(QString const& fileName);

    /**
     * @brief ~ResourceBundle : Dtor
     */
    virtual ~KoResourceBundle();

    /**
     * @brief defaultFileExtension
     * @return the default file extension which should be when saving the resource
     */
    QString defaultFileExtension() const;

    /**
     * @brief load : Load this resource.
     * @return true if succeed, false otherwise.
     */
    bool load();
    bool loadFromDevice(QIODevice *dev);

    /**
     * @brief save : Save this resource.
     * @return true if succeed, false otherwise.
     */
    bool save();

    bool saveToDevice(QIODevice* dev) const;

    /**
     * @brief addMeta : Add a Metadata to the resource
     * @param type type of the metadata
     * @param value value of the metadata
     */
    void setMetaData(const QString &key, const QString &value);
    const QString metaData(const QString &key, const QString &defaultValue = QString()) const;

    /**
     * @brief addFile : Add a file to the bundle
     * @param fileType type of the resource file
     * @param filePath path of the resource file
     */
    void addResource(QString fileType, QString filePath, QVector<KisTagSP> fileTagList, const QByteArray md5sum);

    QList<QString> getTagsList();

    void setThumbnail(QString);

    /**
     * @brief saveMetadata: saves bundle metadata
     * @param store bundle where to save the metadata
     */
    void saveMetadata(QScopedPointer<KoStore> &store);

    /**
     * @brief saveManifest: saves bundle manifest
     * @param store bundle where to save the manifest
     */
    void saveManifest(QScopedPointer<KoStore> &store);

    QStringList resourceTypes() const;
    int resourceCount() const;

    KoResourceBundleManifest &manifest();

    KoResourceSP resource(const QString &resourceType, const QString &filepath);

    QImage image() const;

    QString filename() const;

private:

    void writeMeta(const QString &metaTag, KoXmlWriter *writer);
    void writeUserDefinedMeta(const QString &metaTag, KoXmlWriter *writer);
    bool readMetaData(KoStore *resourceStore);

private:
    QImage m_thumbnail;
    KoResourceBundleManifest m_manifest;
    QMap<QString, QString> m_metadata;
    QSet<QString> m_bundletags;
    QList<QByteArray> m_gradientsMd5Installed;
    QList<QByteArray> m_patternsMd5Installed;
    QList<QByteArray> m_brushesMd5Installed;
    QList<QByteArray> m_palettesMd5Installed;
    QList<QByteArray> m_workspacesMd5Installed;
    QList<QByteArray> m_presetsMd5Installed;
    QString m_filename;
    QString m_bundleVersion;

};

typedef QSharedPointer<KoResourceBundle> KoResourceBundleSP;

#endif // KORESOURCEBUNDLE_H
