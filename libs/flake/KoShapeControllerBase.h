/* This file is part of the KDE project

   Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2006, 2010 Thomas Zander <zander@kde.org>
   Copyright (C) 2008 C. Boemann <cbo@boemann.dk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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
