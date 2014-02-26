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
#include "kowidgets_export.h"

class KoXmlResourceBundleManifest;
class KoXmlResourceBundleMeta;
class KoResourceBundleManager;


class KOWIDGETS_EXPORT KoResourceBundle : public KoResource
{

public:
    KoResourceBundle(QString const&);
        
    ~KoResourceBundle();

    /**
     * Load this resource.
     */
    bool load();

    /**
     * Save this resource.
     */
    bool save();

    /**
     * Returns a QImage representing this resource.  This image could be null. The image can
     * be in any valid QImage::Format.
     */
    QImage image() const;

    /// Returns the default file extension which should be when saving the resource
    QString defaultFileExtension() const;

    void addFile();
    
private:
    QImage thumbnail;
    KoXmlResourceBundleManifest* manifest;
    KoXmlResourceBundleMeta* meta;
    KoResourceBundleManager* man;
};

#endif // KORESOURCEBUNDLE_H
