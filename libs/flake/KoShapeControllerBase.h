/* This file is part of the KDE project

   SPDX-FileCopyrightText: 2006 Jan Hambrecht <jaham@gmx.net>
   SPDX-FileCopyrightText: 2006, 2010 Thomas Zander <zander@kde.org>
   SPDX-FileCopyrightText: 2008 C. Boemann <cbo@boemann.dk>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOshapeControllerBASE_H
#define KOshapeControllerBASE_H

#include "kritaflake_export.h"

#include <QList>

class QRectF;
class KoShape;
class KoShapeContainer;
class KoshapeControllerBasePrivate;
class KoDocumentResourceManager;
class KUndo2Command;

/**
 * The  KoshapeControllerBase is an abstract interface that the application's class
 * that owns the shapes should implement. This tends to be the document.
 * @see KoShapeDeleteCommand, KoShapeCreateCommand
 */
class KRITAFLAKE_EXPORT KoShapeControllerBase
{
public:
    KoShapeControllerBase();
    virtual ~KoShapeControllerBase();

    /**
     * When shapes are dropped to the canvas, the document should decide, where to
     * which parent to put them. In some cases the document should even create a
     * special layer for the new \p shapes.
     *
     * \return the proposed parent for \p shapes
     * \param parentCommand the command, which should be executed before the
     *                      proposed parent will be added to the document (if
     *                      new layer should be created)
     */
    virtual KoShapeContainer* createParentForShapes(const QList<KoShape*> shapes, KUndo2Command *parentCommand);

    /**
     * Return a pointer to the resource manager associated with the
     * shape-set (typically a document). The resource manager contains
     * document wide resources * such as variable managers, the image
     * collection and others.
     */
    virtual KoDocumentResourceManager *resourceManager() const;

    /**
     * The size of the document measured in rasterized pixels. This information is needed for loading
     * SVG documents that use 'px' as the default unit.
     */
    virtual QRectF documentRectInPixels() const = 0;

    /**
     * The size of the document measured in 'pt'
     */
    QRectF documentRect() const;

    /**
     * Resolution of the rasterized representation of the document. Used to load SVG documents correctly.
     */
    virtual qreal pixelsPerInch() const = 0;

private:
    KoshapeControllerBasePrivate * const d;
};

#endif
