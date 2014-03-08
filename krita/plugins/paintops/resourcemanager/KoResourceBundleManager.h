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

#ifndef KORESOURCEBUNDLEMANAGER_H
#define KORESOURCEBUNDLEMANAGER_H

#include "KoStore.h"
#include <krita_export.h> 

class KoXmlResourceBundleManifest;
class KoXmlResourceBundleMeta;

/**
 * @brief The KoResourceBundleManager class
 * @details Manage resource bundles structure
 */
class KRITAUI_EXPORT KoResourceBundleManager
{
public:

    /**
     * @brief KoResourceBundleManager : Ctor
     * @param kritaPath the path containing main Krita resources
     * @param packName the name of the package to be opened/created
     * @param mode the opening mode for the file
     */
    KoResourceBundleManager(QString kritaPath="",QString packName="",KoStore::Mode mode=KoStore::Write);

    /**
     * @brief setReadPack : Opens the store in Read mode
     * @param packName the name of the package to be opened
     */
    void setReadPack(QString packName);

    /**
     * @brief setWritePack : Opens the store in Write mode
     * @param packName the name of the package to be opened
     */
    void setWritePack(QString packName);

    /**
     * @brief setKritaPath : Set the path of Krita resources
     * @param kritaPath
     */
    void setKritaPath(QString kritaPath);

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
     * Add a Krita resource file to the store.
     * @param path the path containing the Krita resource File.
     * @return true if succeed, false otherwise.
     */

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
    void extractKFiles(QList<QString> pathList);

    /**
     * @brief createPack : Create a full resource package.
     * @param manifest the virtual generator of manifest file
     * @param meta the virtual generator of meta file
     */
    void createPack(KoXmlResourceBundleManifest* manifest, KoXmlResourceBundleMeta* meta);

    /**
     * @brief addManiMeta : Add manifest and meta Xml Files to the store
     * @param manifest the virtual generator of manifest file
     * @param meta the virtual generator of meta file
     */
    void addManiMeta(KoXmlResourceBundleManifest* manifest, KoXmlResourceBundleMeta* meta);

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

    ///File Method shortcuts

    bool atEnd() const;
    bool bad() const;
    bool close();
    bool hasFile(const QString &name) const;
    bool finalize();
    bool isOpen() const;
    bool open(const QString &name);
    QByteArray read(qint64 max);
    qint64 size() const;
    qint64 write(const QByteArray &_data);

    QString kritaPath;
    QString packName;

private:
    KoStore* resourcePack;
};

#endif // KORESOURCEBUNDLEMANAGER_H
