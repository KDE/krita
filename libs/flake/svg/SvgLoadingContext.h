/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2011 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef SVGLOADINGCONTEXT_H
#define SVGLOADINGCONTEXT_H

#include <functional>
#include <QStringList>
#include <KoXmlReader.h>
#include "kritaflake_export.h"

class SvgGraphicsContext;
class SvgStyleParser;
class KoDocumentResourceManager;
class KoImageCollection;
class KoShape;
class KoColorProfile;

/// Contains data used for loading svg
class KRITAFLAKE_EXPORT SvgLoadingContext
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

    QString relativeFilePath(const QString &href);

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
    QStringList matchingCssStyles(const KoXmlElement &element) const;

    /// Returns a style parser to parse styles
    SvgStyleParser &styleParser();

    /// parses 'color-profile' tag and saves it in the context
    void parseProfile(const KoXmlElement &element);

    /// Return the profiles in the context.
    QHash<QString, const KoColorProfile*> profiles();

    bool isRootContext() const;

    typedef std::function<QByteArray(const QString&)> FileFetcherFunc;
    void setFileFetcher(FileFetcherFunc func);

    QByteArray fetchExternalFile(const QString &url);

private:
    class Private;
    Private * const d;
};

#endif // SVGLOADINGCONTEXT_H
