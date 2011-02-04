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

#ifndef KOCLIPPATH_H
#define KOCLIPPATH_H

#include <QtCore/QList>
#include <QtCore/QSharedData>

class KoPathShape;
class QPainter;
class QTransform;
class KoViewConverter;

class KoClipData : public QSharedData
{
public:
    KoClipData( KoPathShape * clipPathShape );
    KoClipData( const QList<KoPathShape*> & clipPathShapes );
    ~KoClipData();
    QList<KoPathShape*> clipPathShapes() const;
    void removeClipShapesOwnership();
private:
    QList<KoPathShape*> m_clipPathShapes;
    bool m_deleteClipShapes;
};

class KoClipPath
{
public:
    /// Create a new shape clipping using the given path shape
    KoClipPath( KoClipData * clipData, const QTransform & shapeMatrix );

    ~KoClipPath();

    /// Applies the clipping to the given painter
    void applyClipping( QPainter & painter, const KoViewConverter &converter );

private:
    class Private;
    Private * const d;
};

#endif // KOCLIPPATH_H
