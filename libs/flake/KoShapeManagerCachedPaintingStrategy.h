/* This file is part of the KDE project
 * Copyright (C) 2010 KO GmbH <boud@kogmbh.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (  at your option ) any later version.
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
#ifndef KOSHAPEMANAGERCACHEDPAINTINGSTRATEGY_H
#define KOSHAPEMANAGERCACHEDPAINTINGSTRATEGY_H

#include "KoShape.h"
#include "KoShapeManagerPaintingStrategy.h"
#include "flake_export.h"

/**
 * The cached painting strategy uses QPixmapCache to paint shapes
 * using cached QPixmaps whereever possible. Setting this strategy
 * on your KoShapeManager can improve scrolling performance at the
 * cost of extra memory usage.
 */
class FLAKE_EXPORT KoShapeManagerCachedPaintingStrategy : public KoShapeManagerPaintingStrategy
{
public:

    KoShapeManagerCachedPaintingStrategy(KoShapeManager *shapeManager);

    virtual ~KoShapeManagerCachedPaintingStrategy();

    void paint(KoShape *shape, QPainter &painter, const KoViewConverter &converter, bool forPrint);

    void adapt(KoShape *shape, QRectF &rect);
};

#endif // KOSHAPEMANAGERCACHEDPAINTINGSTRATEGY_H
