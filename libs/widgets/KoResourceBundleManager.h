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

#include <QString>
#include "KoXmlResourceBundleManifest.h"
#include "KoXmlResourceBundleMeta.h"
#include "KoStore.h"


class KoResourceBundleManager
{
    ///@description the virtual resource package
    KoStore* resourcePack;

public:
    ///@description the path containing main Krita resources
    QString kritaPath;
    QString packName;

    /**
     * Constructor
     * Creates a virtual XML file.
     * @param packName the name of the package to be opened/created
     * @param kritaPath the path containing main Krita resources
     * @param mode the opening mode for the file
     */
    KoResourceBundleManager(QString kritaPath="",QString packName="",KoStore::Mode mode=KoStore::Write);

    /**
     * Constructor
     * Creates a virtual XML file.
     * @param store the existing store to be used as resource pack
     * @param kritaPath the path containing main Krita resources
     */
    KoResourceBundleManager(KoStore *store,QString kritaPath="");

    /**
     * Opens the store in Read mode.
     * @param packName the name of the package to be opened
     */
    void setReadPack(QString packName);

    /**
     * Opens the store in Write mode.
     * @param packName the name of the package to be opened/created
     */
    void setWritePack(QString packName);

    /**
     * Update the path attribute.
     * @param kritaPath the path containing main Krita resources
     */
    void setKritaPath(QString kritaPath);

    ///@return true if the path is set (eq. !=""), false otherwise.
    bool isPathSet();

    /**
     * Set current directory to the root of the resource package.
     */
    void toRoot();

    /**
     * Add a Krita resource file to the store.
     * @param path the path containing the Krita resource File.
     * @return true if the file has been added, false otherwise.
     */
    bool addKFile(QString path);

    /**
     * Add several Krita resource files to the store.
     * @param pathList the list containing all the paths of the files to be added.
     */
    void addKFiles(QString *pathList);

    /**
     * Extract several Krita resource files from the store to Krita resource path.
     * @param packName the name of the package to be extracted
     */
    void extractKFiles(QString *pathList);


    /**
     * Extract a full resource package.
     * @param pathList the list containing all the paths of the files to be extracted.
     */
    void extractPack(QString packName);

    /**
     * Create a full resource package.
     * @param manifest the virtual generator of manifest file
     * @param manifest the virtual generator of meta file
     */
    void createPack(KoXmlResourceBundleManifest manifest, KoXmlResourceBundleMeta meta);

    /**
     * @return a QByteArray containing data of the file in the store
     * @param fileName the path of the file in the store
     */
    QByteArray getFileData(const QString &fileName);

    /**
     * @return a QIODevice containing the file in the store
     * @param fileName the path of the file in the store
     */
    QIODevice* getFile(const QString &fileName);

    ///File Method shortcuts

    bool hasFile(const QString &name) const;
    bool open(const QString &name);
    bool isOpen() const;
    bool close();
    QByteArray read(qint64 max);
    qint64 write(const QByteArray &_data);
    qint64 read(char *_buffer, qint64 _len);
    qint64 write(const char *_data, qint64 _len);
    qint64 size() const;
    bool atEnd() const;
    bool enterDirectory(const QString &directory);
    bool leaveDirectory();
    QIODevice* device();


    //TODO The following methods are not implemented for the moment.
    //Their specifications are given as potential but not final.

    //TODO QFile* extractManifest();
    //TODO void extractMeta();
    //TODO void extractThumbnail();

    //TODO addManifest(KoXmlManifest);
    //TODO addMeta(KoXmlMeta);
    //TODO addThumbnail();
};

#endif // KORESOURCEBUNDLEMANAGER_H
