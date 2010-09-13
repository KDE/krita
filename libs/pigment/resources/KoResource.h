/*  This file is part of the KDE project
    Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
    Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef KORESOURCE_H
#define KORESOURCE_H

#include <QtGui/QImage>
#include <QtCore/QString>
#include <pigment_export.h>

class QDomDocument;
class QDomElement;

/**
 * The KoResource class provides a representation of resources.  This
 * includes, but not limited to, brushes and patterns.
 *
 * This replaces the KisKrayon facility that used to be present in Krayon.
 */
class PIGMENTCMS_EXPORT KoResource
{

public:

    /**
     * Creates a new KoResource object using @p filename.  No file is opened
     * in the constructor, you have to call load.
     *
     * @param filename the file name to save and load from.
     */
    KoResource(const QString& filename);
    virtual ~KoResource();

public:
    /**
     * Load this resource.
     */
    virtual bool load() = 0;

    /**
     * Save this resource.
     */
    virtual bool save() = 0;

    /**
     * Returns a QImage representing this resource.  This image could be null.
     */
    virtual QImage image() const {
        return QImage();
    }

private:
    /// XXX: doc?
    virtual void toXML(QDomDocument& , QDomElement&) const;

public:

    /// Returns if resource can be removed by the user
    bool removable() const;

    /// Returns the default file extension which should be when saving the resource
    virtual QString defaultFileExtension() const;

public:
    QString filename() const;
    void setFilename(const QString& filename);

    /// Name of the file without the path
    QString shortFilename() const;
    QString name() const;
    void setName(const QString& name);
    bool valid() const;
    void setValid(bool valid);

private:
    KoResource(const KoResource&);
    KoResource& operator=(const KoResource&);

private:
    struct Private;
    Private* const d;
};

#endif // KORESOURCE_H_

