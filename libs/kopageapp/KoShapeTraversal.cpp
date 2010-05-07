/* This file is part of the KDE project
   Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>

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

#include "KoShapeTraversal.h"

#include <kdebug.h>

#include <KoShape.h>
#include <KoShapeContainer.h>

KoShape * KoShapeTraversal::nextShape( const KoShape * current )
{
    return nextShapeStep( current, 0 );
}

KoShape * KoShapeTraversal::nextShape( const KoShape * current, const QString & shapeId )
{
    KoShape * next = nextShapeStep( current, 0 );

    while ( next != 0 && next->shapeId() != shapeId ) {
        next = nextShapeStep( next, 0 );
    }

    return next;
}

KoShape * KoShapeTraversal::previousShape( const KoShape * current )
{
    return previousShapeStep( current, 0 );
}

KoShape * KoShapeTraversal::previousShape( const KoShape * current, const QString & shapeId )
{
    KoShape * previous = previousShapeStep( current, 0 );

    while ( previous != 0 && previous->shapeId() != shapeId ) {
        previous = previousShapeStep( previous, 0 );
    }

    return previous;
}

KoShape * KoShapeTraversal::last( KoShape * current )
{
    KoShape * last = current;
    while ( const KoShapeContainer * container = dynamic_cast<const KoShapeContainer *>( last ) ) {
        QList<KoShape*> shapes = container->shapes();
        if ( !shapes.isEmpty() ) {
            last = shapes.last();
        }
        else {
            break;
        }
    }
    return last;
}

KoShape * KoShapeTraversal::nextShapeStep( const KoShape * current, const KoShapeContainer * parent )
{
    Q_ASSERT( current );
    if ( !current ) {
        return 0;
    }

    KoShape * next = 0;

    if ( parent ) {
        const QList<KoShape*> shapes = parent->shapes();
        QList<KoShape*>::const_iterator it( qFind( shapes, current ) );
        Q_ASSERT( it != shapes.end() );

        if ( it == shapes.end() ) {
            kWarning(30010) << "the shape is not in the list of children of his parent";
            return 0;
        }

        ++it;
        if ( it != shapes.end() ) {
            next = *it;
        }
        else {
            KoShapeContainer * currentParent = parent->parent();
            next = currentParent ? nextShapeStep( parent, currentParent ) : 0;
        }
    }
    else {
        if ( const KoShapeContainer * container = dynamic_cast<const KoShapeContainer *>( current ) ) {
            QList<KoShape*> shapes = container->shapes();
            if ( !shapes.isEmpty() ) {
                next = shapes[0];
            }
        }

        if ( next == 0 ) {
            KoShapeContainer * currentParent = current->parent();
            next = currentParent ? nextShapeStep( current, currentParent ) : 0;
        }
    }

    return next;
}

KoShape * KoShapeTraversal::previousShapeStep( const KoShape * current, const KoShapeContainer * parent )
{
    Q_ASSERT( current );
    if ( !current ) {
        return 0;
    }

    KoShape * previous = 0;

    if ( parent ) {
        if ( previous == 0 ) {
            const QList<KoShape*> shapes = parent->shapes();
            QList<KoShape*>::const_iterator it( qFind( shapes, current ) );
            Q_ASSERT( it != shapes.end() );

            if ( it == shapes.end() ) {
                kWarning(30010) << "the shape is not in the list of children of his parent";
                return 0;
            }

            if ( it != shapes.begin() ) {
                --it;
                previous = last( *it );
            }
            else {
                previous = current->parent();
            }
        }
    }
    else {
        KoShapeContainer * currentParent = current->parent();
        previous = currentParent ? previousShapeStep( current, currentParent ) : 0;
    }

    return previous;
}
