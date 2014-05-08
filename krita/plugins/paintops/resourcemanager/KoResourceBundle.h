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

#include "KoResource.h"

class KoXmlResourceBundleManifest;
class KoXmlResourceBundleMeta;
class KoStore;

/**
 * @brief The KoResourceBundle class
 * @details Describe the resource bundles as KoResources
 */
class KoResourceBundle : public KoResource
{

public:
    /**
     * @brief KoResourceBundle : Ctor
     * @param bundlePath the path of the bundle
     */
    KoResourceBundle(QString const& fileName);

    /**
     * @brief ~KoResourceBundle : Dtor
     */
    virtual ~KoResourceBundle();

    /**
     * @brief image
     * @return a QImage representing this resource.
     */
    QImage image() const;

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
    void addMeta(QString type, QString value);

    /**
     * @brief addMeta : Add a Metadata to the resource
     * @param type type of the metadata
     * @param value value of the metadata
     */
    void setMeta(KoXmlResourceBundleMeta* newMeta);

    /**
     * @brief addFile : Add a file to the bundle
     * @param fileType type of the resource file
     * @param filePath path of the resource file
     */
    void addFile(QString fileType, QString filePath, QStringList fileTagList);

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
     * @brief getAuthor
     * @return the metadata associated to the field "author" or QString() if it doesn't exist
     */
    QString getAuthor();

    /**
     * @brief getLicense
     * @return the metadata associated to the field "license" or QString() if it doesn't exist
     */
    QString getLicense();

    /**
     * @brief getWebSite
     * @return the metadata associated to the field "website" or QString() if it doesn't exist
     */
    QString getWebSite();

    /**
     * @brief getCreated
     * @return the metadata associated to the field "created" or QString() if it doesn't exist
     */
    QString getCreated();

    /**
     * @brief getUpdated
     * @return the metadata associated to the field "updated" or QString() if it doesn't exist
     */
    QString getUpdated();

    /**
     * @brief isInstalled
     * @return true if the bundle is installed, false otherwise.
     */
    bool isInstalled();

    void setThumbnail(QString);
    void removeTag(QString tagName);

protected:

    virtual QByteArray generateMD5() const;

private:

    /**
     * @brief setReadPack : Opens the store in Read mode
     * @param packName the name of the package to be opened
     */
    void setReadPack(QString m_packName);

    /**
     * @brief setWritePack : Opens the store in Write mode
     * @param packName the name of the package to be opened
     */
    void setWritePack(QString m_packName);

    /**
     * @brief setKritaPath : Set the path of Krita resources
     * @param kritaPath
     */
    void setKritaPath(QString m_kritaPath);

    /**
     * @brief isPathSet : Check if the path of Krita resources is set
     * @return true if succeed, false otherwise.
     */
    bool isPathSet();

    /**
     * @brief toRoot : Go to the root of the store
     */
    void toRoot();

    /**
     * @brief addKFile : Add a Krita resource file to the store.
     * @param path the path containing the Krita resource File.
     * @return true if succeed, false otherwise.
     */
    bool addKFile(QString path);

    /**
     * @brief addKFileBundle : Add a Krita resource file from a bundle folder to the store.
     * @param path the path containing the Krita resource File.
     * @return true if succeed, false otherwise.
     */
    bool addKFileBundle(QString path);

    /**
     * @brief addKFiles : Add several Krita resource files to the store.
     * @param pathList the list containing all the paths of the files to be added.
     */
    void addKFiles(QList<QString> pathList);

    /**
     * @brief extractKFiles : Extract several Krita resource files from the store to Krita path.
     * @param pathList the list containing all the paths of the files to be extracted.
     */
    void extractKFiles(QMap<QString, QString> pathList);

    /**
     * @brief createPack : Create a full resource package.
     * @param manifest the virtual generator of manifest file
     * @param meta the virtual generator of meta file
     */
    void createPack(KoXmlResourceBundleManifest* manifest, KoXmlResourceBundleMeta* meta, QImage thumbnail, bool firstBuild = false);

    /**
     * @brief addManiMeta : Add manifest and meta Xml Files to the store
     * @param manifest the virtual generator of manifest file
     * @param meta the virtual generator of meta file
     */
    void addManiMeta(KoXmlResourceBundleManifest* manifest, KoXmlResourceBundleMeta* meta);

    void addThumbnail(QImage thumbnail);

    /**
     * @brief getFileData
     * @param fileName the path of the file in the store
     * @return a QByteArray containing data of the file in the store
     */
    QByteArray getFileData(const QString &fileName);

    /**
     * @brief getFile
     * @param fileName the path of the file in the store
     * @return a QIODevice containing the file in the store
     */
    QIODevice* getFile(const QString &fileName);

    /**
     * @brief removeDir : Remove the chosen directory
     * @param dirName the name of the directory to be removed
     * @return true if succeed, false otherwise.
     */
    static bool removeDir(const QString & dirName);

    void extractTempFiles(QList<QString> pathList);


private:
    QString m_kritaPath;
    QString m_packName;
    KoStore* m_resourceStore;
    QImage m_thumbnail;
    KoXmlResourceBundleManifest* m_manifest;
    KoXmlResourceBundleMeta* m_meta;

    bool m_installed;
};

#endif // KORESOURCEBUNDLE_H
