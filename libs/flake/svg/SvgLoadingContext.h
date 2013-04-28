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

#ifndef SVGLOADINGCONTEXT_H
#define SVGLOADINGCONTEXT_H

#include "flake_export.h"
#include <KoXmlReader.h>

class SvgGraphicsContext;
class SvgStyleParser;
class KoDocumentResourceManager;
class KoImageCollection;
class KoShape;

/// Contains data used for loading svg
class FLAKE_EXPORT SvgLoadingContext
{
public:
    explicit SvgLoadingContext(KoDocumentResourceManager *documentResourceManager);
    ~SvgLoadingContext();

    /// Returns the current graphics context
    SvgGraphicsContext *currentGC() const;

    /// Pushes a new graphics context to the stack
    SvgGraphicsContext *pushGraphicsContext(const KoXmlElement &element = KoXmlElement(), bool inherit = true);

    /// Pops the current graphics context from the stack
    void popGraphicsContext();

    /// Sets the initial xml base dir, i.e. the directory the svg file is read from
    void setInitialXmlBaseDir(const QString &baseDir);

    /// Returns the current xml base dir
    QString xmlBaseDir() const;

    /// Constructs an absolute file path from the given href and current xml base directory
    QString absoluteFilePath(const QString &href);

    /// Returns the next z-index
    int nextZIndex();

    /// Returns the image collection used for managing images
    KoImageCollection* imageCollection();

    /// Registers a shape so it can be referenced later
    void registerShape(const QString &id, KoShape *shape);

    /// Returns shape with specified id
    KoShape* shapeById(const QString &id);

    /// Adds a definition for later use
    void addDefinition(const KoXmlElement &element);

    /// Returns the definition with the specified id
    KoXmlElement definition(const QString &id) const;

    /// Checks if a definition with the specified id exists
    bool hasDefinition(const QString &id) const;

    /// Adds a css style sheet
    void addStyleSheet(const KoXmlElement &styleSheet);

    /// Returns list of css styles matching to the specified element
    QStringList matchingStyles(const KoXmlElement &element) const;

    /// Returns a style parser to parse styles
    SvgStyleParser &styleParser();

private:
    class Private;
    Private * const d;
};

#endif // SVGLOADINGCONTEXT_H
