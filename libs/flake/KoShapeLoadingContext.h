/* This file is part of the KDE project
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>

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

#ifndef KOSHAPELOADINGCONTEXT_H
#define KOSHAPELOADINGCONTEXT_H

#include <QMap>
#include <QString>

#include <flake_export.h>

class KoOasisLoadingContext;
class KoShapeLayer;
class KoShape;

/**
 * Context passed to shapes during loading.
 * This class holds various variables as well as a context full of variables which all together
 * form the context of a loading operation.
 */
class FLAKE_EXPORT KoShapeLoadingContext
{
public:
    /**
     * constructor
     * @param context the context created for generic ODF loading.
     */
    KoShapeLoadingContext( KoOasisLoadingContext & context );

    /// return the embedded loading context
    KoOasisLoadingContext & koLoadingContext();

    /// Returns layer referenced by given name
    KoShapeLayer * layer( const QString & layerName );
    /// Adds a new layer to be referenced by the given name later
    void addLayer( KoShapeLayer * layer, const QString & layerName );

    /// register the id for a specific shape
    void addShapeId( KoShape * shape, const QString & id );
    /// return the shape formerly registered using addShapeId()
    KoShape * shapeById( const QString & id );

private:
    class Private;
    Private * const d;
};

#endif /* KOSHAPELOADINGCONTEXT_H */
