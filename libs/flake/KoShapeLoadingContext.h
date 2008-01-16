/* This file is part of the KDE project
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>

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
class KoShapeControllerBase;
class KoImageCollection;
class KoSharedLoadingData;

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
     * @param shapeController the shape controller. This is used to call KoShapeControllerBase::shapeCreated
     *        during loading. Please leave 0 only when you are 100% sure you don't need it.
     */
    KoShapeLoadingContext( KoOasisLoadingContext & context, KoShapeControllerBase * shapeController );

    /// destructor
    ~KoShapeLoadingContext();

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

    /// Sets the image collection used for loading images
    void setImageCollection( KoImageCollection * imageCollection );

    /// Returns the image collection for loading images
    KoImageCollection * imageCollection();

    /// Get current z-index
    int zIndex();

    /// Set z-index
    void setZIndex( int index );

    /**
     * Add the z-index 
     * 
     * Used for document which use the z-index instead of the order of the shapes
     * in the document.
     */
    void addShapeZIndex( KoShape * shape, int index );

    /// Get the save z-indices
    const QMap<KoShape*, int> & shapeZIndices();

    /**
     * Add shared data
     *
     * This can be use to pass data between shapes on loading. E.g. The decoded text styles
     * of the TextShape. With that the styles only have to be read once and can be used in 
     * all shapes that also need them.
     *
     * The ownership of the added data is passed to teh context. The KoShapeLoadingContext will
     * delete the added data when it is destroyed.
     *
     * Data inserted for a specific id will not be overwritten by calling addSharedData with 
     * the same id again.
     *
     * You get an assertion when the id is already existing.
     *
     * @see KoSharedLoadingData
     */
    void addSharedData( const QString & id, KoSharedLoadingData * data );

    /**
     * Get the shared data.
     *
     * @see KoSharedLoadingData
     *
     * @param id The id used to identify the shared data.
     * @return The shared data for the id or 0 if there is no shared data for the id.
     */
    KoSharedLoadingData * sharedData( const QString & id ) const;

private:
    /**
     * Get the shape controller
     *
     * @see KoShapeControllerBase::shapeCreated
     */
    KoShapeControllerBase * shapeController() const;

    // to allow only the KoShapeRegistry access to the KoShapeControllerBase
    friend class KoShapeRegistry;
    class Private;
    Private * const d;
};

#endif /* KOSHAPELOADINGCONTEXT_H */
