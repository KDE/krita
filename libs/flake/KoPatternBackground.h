/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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

#ifndef KOPATTERNBACKGROUND_H
#define KOPATTERNBACKGROUND_H

#include "KoShapeBackground.h"
#include "flake_export.h"
#include <QtGui/QMatrix>

/// A pattern shape background
class FLAKE_EXPORT KoPatternBackground : public KoShapeBackground
{
public:
    KoPatternBackground();

    /// Creates new pattern background with the given pattern image
    KoPatternBackground( const QImage &pattern );

    virtual ~KoPatternBackground();

    /// Sets the transform matrix
    void setMatrix( const QMatrix &matrix );

    /// Returns the transform matrix
    QMatrix matrix() const;

    /// Sets a new pattern
    void setPattern( const QImage &pattern );
    /// Returns the pattern
    QImage pattern();

    /// Assignment operator
    KoPatternBackground& operator = ( const KoPatternBackground &rhs );

    /// reimplemented from KoShapeBackground
    virtual void paint( QPainter &painter, const QPainterPath &fillPath ) const;
    /// reimplemented from KoShapeBackground
    virtual void fillStyle( KoGenStyle &style, KoShapeSavingContext &context );
    /// reimplemented from KoShapeBackground
    virtual bool loadStyle( KoOdfLoadingContext & context, const QSizeF &shapeSize );

private:
    class Private;
    Private * const d;
};

#endif // KOPATTERNBACKGROUND_H
