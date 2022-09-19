/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2011 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KisScalableVectorGraphicsSaveContext_H
#define KisScalableVectorGraphicsSaveContext_H

#include <QtGlobal>

class KoXmlWriter;
class KoShape;
class QIODevice;
class QString;
class QTransform;
class QImage;

#include "kritaflake_export.h"

/// Context for saving svg files
class KRITAFLAKE_EXPORT KisScalableVectorGraphicsSaveContext
{
public:
    /// Creates a new svg saving context on the specified output device
    explicit KisScalableVectorGraphicsSaveContext(QIODevice &outputDevice, bool saveInlineImages = true);
    explicit KisScalableVectorGraphicsSaveContext(QIODevice &shapesDevice, QIODevice &styleDevice, bool saveInlineImages = true);

    /// Virtual destructor
    virtual ~KisScalableVectorGraphicsSaveContext();

    /// Provides access to the style writer
    KoXmlWriter &styleWriter();

    /// Provides access to the shape writer
    KoXmlWriter &shapeWriter();

    /// Create a unique id from the specified base text
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

    void setStrippedTextMode(bool value);
    bool strippedTextMode() const;

private:
    Q_DISABLE_COPY(KisScalableVectorGraphicsSaveContext)

private:
    class Private;
    Private * const d;
};

#endif // KisScalableVectorGraphicsSaveContext_H
