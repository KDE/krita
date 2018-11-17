/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef LIBKIS_RESOURCE_H
#define LIBKIS_RESOURCE_H

#include <QObject>
#include <kis_types.h>
#include "kritalibkis_export.h"
#include "libkis.h"
#include <KoResource.h>

/**
 * A Resource represents a gradient, pattern, brush tip, brush preset, palette or 
 * workspace definition.
 * 
 * @code
 * allPresets = Application.resources("preset")
 * for preset in allPresets:
 *     print(preset.name())
 * @endcode
 * 
 * Resources are identified by their type, name and filename. If you want to change
 * the contents of a resource, you should read its data using data(), parse it and
 * write the changed contents back.
 */
class KRITALIBKIS_EXPORT Resource : public QObject
{
    Q_OBJECT

public:
    explicit Resource(KoResourceSP resource, QObject *parent = 0);
    ~Resource() override;
    bool operator==(const Resource &other) const;
    bool operator!=(const Resource &other) const;


public Q_SLOTS:
    
    /**
     * Return the type of this resource. Valid types are:
     * <ul>
     * <li>pattern: a raster image representing a pattern
     * <li>gradient: a gradient
     * <li>brush: a brush tip
     * <li>preset: a brush preset
     * <li>palette: a color set
     * <li>workspace: a workspace definition.
     * </ul>
     */
    QString type() const;

    /**
     * The user-visible name of the resource.
     */
    QString name() const;
    
    /**
     * setName changes the user-visible name of the current resource.
     */
    void setName(QString value);

    /**
     * The filename of the resource, if present. Not all resources
     * are loaded from files.
     */
    QString filename() const;

    /**
     * An image that can be used to represent the resource in the
     * user interface. For some resources, like patterns, the 
     * image is identical to the resource, for others it's a mere
     * icon.
     */
    QImage image() const;
    
    /**
     * Change the image for this resource.
     */
    void setImage(QImage image);

    /**
     * Return the resource as a byte array.
     */
    QByteArray data() const;
    
    /**
     * Change the internal data of the resource to the given byte 
     * array. If the byte array is not valid, setData returns
     * false, otherwwise true.
     */
    bool setData(QByteArray data);

private:

    friend class PresetChooser;
    friend class View;
    friend class Palette;
    KoResourceSP resource() const;

    struct Private;
    const Private *const d;

};

#endif // LIBKIS_RESOURCE_H
