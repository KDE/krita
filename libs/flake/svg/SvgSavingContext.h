/* This file is part of the KDE project
 * Copyright (C) 2011 Jan Hambrecht <jaham@gmx.net>
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

#ifndef SVGSAVINGCONTEXT_H
#define SVGSAVINGCONTEXT_H

class KoXmlWriter;
class KoShape;
class KoImageData;
class QIODevice;
class QString;
class QTransform;
class QImage;

#include "flake_export.h"

/// Context for saving svg files
class FLAKE_EXPORT SvgSavingContext
{
public:
    /// Creates a new svg saving context on the specified output device
    explicit SvgSavingContext(QIODevice &outputDevice, bool saveInlineImages = true);

    /// Virtual destructor
    virtual ~SvgSavingContext();

    /// Provides access to the style writer
    KoXmlWriter &styleWriter();

    /// Provides access to the shape writer
    KoXmlWriter &shapeWriter();

    /// Create a unqiue id from the specified base text
    QString createUID(const QString &base);

    /// Returns the unique id for the given shape
    QString getID(const KoShape *obj);

    /// Returns the transformation used to transform into usre space
    QTransform userSpaceTransform() const;

    /// Returns if image should be saved inline
    bool isSavingInlineImages() const;

    /// Create a filename suitable for saving external data
    QString createFileName(const QString &extension);

    /// Saves given image and returns the href used
    QString saveImage(const QImage &image);

    /// Saves given image and returns the href used
    QString saveImage(KoImageData *image);

private:
    class Private;
    Private * const d;
};

#endif // SVGSAVINGCONTEXT_H
