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

#include <KoXmlWriter.h>

#include "KoResource.h"
#include "KoXmlResourceBundleManifest.h"

/**
 * @brief The KoResourceBundle class
 * @details Describe the resource bundles as KoResources
 */
class KoResourceBundle : public KoResource
{

public:
    /**
     * @brief KoResourceBundle : Ctor * @param bundlePath the path of the bundle
     */
    KoResourceBundle(QString const& fileName);

    /**
     * @brief ~KoResourceBundle : Dtor
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

    /**
     * @brief save : Save this resource.
     * @return true if succeed, false otherwise.
     */
    bool save();

    virtual bool saveToDevice(QIODevice* dev) const;

    /**
     * @brief install : Install the resource bundle.
     */
    void install();

    /**
     * @brief uninstall : Uninstall the resource bundle.
     */
    void uninstall();

    /**
     * @brief addMeta : Add a Metadata to the resource
     * @param type type of the metadata
     * @param value value of the metadata
     */
    void addMeta(const QString &type, const QString &value);
    const QString getMeta(const QString &type, const QString &defaultValue = QString()) const;

    /**
     * @brief addFile : Add a file to the bundle
     * @param fileType type of the resource file
     * @param filePath path of the resource file
     */
    void addResource(QString fileType, QString filePath, QStringList fileTagList, const QByteArray md5sum);

    QList<QString> getTagsList();

    /**
     * @brief removeFile : Remove a file from the bundle
     * @param fileName name of the resource file
     */
    void removeFile(QString fileName);

    /**
     * @brief addResourceDirs : Link the directories containing the resources of the bundle to the resource types
     */
    void addResourceDirs();

    /**
     * @brief rename : Rename the bundle
     */
    void rename(QString, QString);

    /**
     * @brief isInstalled
     * @return true if the bundle is installed, false otherwise.
     */
    bool isInstalled();

    void setThumbnail(QString);

    void addTag(const QString &tagName);
    void removeTag(QString tagName);

protected:

    virtual QByteArray generateMD5() const;

private:

    void writeMeta(const char *metaTag, const QString &metaKey, KoXmlWriter *writer);
    void writeUserDefinedMeta(const QString &metaKey, KoXmlWriter *writer);

private:
    QImage m_thumbnail;
    KoXmlResourceBundleManifest m_manifest;
    QMap<QString, QString> m_metadata;
    QSet<QString> m_bundletags;
    bool m_installed;
};

#endif // KORESOURCEBUNDLE_H
