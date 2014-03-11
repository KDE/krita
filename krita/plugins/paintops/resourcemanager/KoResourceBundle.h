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
#include "krita_export.h"

class KoXmlResourceBundleManifest;
class KoXmlResourceBundleMeta;
class KoResourceBundleManager;

/**
 * @brief The KoResourceBundle class
 * @details Describe the resource bundles as KoResources
 */
class KRITAUI_EXPORT KoResourceBundle : public KoResource
{

public:
    /**
     * @brief KoResourceBundle : Ctor
     * @param bundlePath the path of the bundle
     */
    KoResourceBundle(QString const& bundlePath,QString kritaPath="/home/metabolic/kde4/src/calligra/krita/data");

    /**
     * @brief ~KoResourceBundle : Dtor
     */
    virtual ~KoResourceBundle();

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
     * @brief addMeta : Add a Metadata to the resource
     * @param type type of the metadata
     * @param value value of the metadata
     */
    void addMeta(QString type,QString value);

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
     * @brief addFile : Add a file to the bundle
     * @param fileType type of the resource file
     * @param filePath path of the resource file
     */
    void addFile(QString fileType,QString filePath);

    /**
     * @brief removeFile : Remove a file from the bundle
     * @param fileName name of the resource file
     */
    void removeFile(QString fileName);

    /**
     * @brief install : Install the resource bundle.
     */
    void install();

    /**
     * @brief uninstall : Uninstall the resource bundle.
     */
    void uninstall();

    /**
     * @brief removeDir : Remove the chosen directory
     * @param dirName the name of the directory to be removed
     * @return true if succeed, false otherwise.
     */
    bool removeDir(const QString & dirName);

private:
    QImage thumbnail;
    KoXmlResourceBundleManifest* manifest;
    KoXmlResourceBundleMeta* meta;
    KoResourceBundleManager* manager;
    bool isInstalled;

};

#endif // KORESOURCEBUNDLE_H
