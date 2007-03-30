/* This file is part of the KDE project
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006,2007 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoParameterToPathCommand.h"
#include "KoParameterShape.h"
#include <klocale.h>

KoParameterToPathCommand::KoParameterToPathCommand( KoParameterShape *shape, QUndoCommand *parent )
: QUndoCommand( parent )
, m_newPointsActive( false )
{
    m_shapes.append( shape );

    setText( i18n( "Convert to Path" ) );
}

KoParameterToPathCommand::KoParameterToPathCommand( const QList<KoParameterShape*> &shapes, QUndoCommand *parent )
: QUndoCommand( parent )
, m_shapes( shapes )
, m_newPointsActive( false )
{
    foreach( KoParameterShape *shape, m_shapes )
    {
        KoSubpathList subpaths = shape->m_subpaths;
        KoSubpathList newSubpaths;
        // make a deep copy of the subpaths
        KoSubpathList::const_iterator pathIt( subpaths.begin() );
        for (  ; pathIt != subpaths.end(); ++pathIt )
        {
            KoSubpath * newSubpath = new KoSubpath();
            newSubpaths.append( newSubpath );
            KoSubpath::const_iterator it(  (  *pathIt )->begin() );
            for (  ; it != (  *pathIt )->end(); ++it )
            {
                newSubpath->append( new KoPathPoint( **it ) );
            }
        }
        m_oldSubpaths.append( subpaths );
        m_newSubpaths.append( newSubpaths );
    }
    setText( i18n( "Convert to Path" ) );
}

KoParameterToPathCommand::~KoParameterToPathCommand()
{
    // clear the no longer needed points
    if ( m_newPointsActive )
    {
        QList<KoSubpathList>::iterator it( m_oldSubpaths.begin() );
        for ( ; it != m_oldSubpaths.end(); ++it )
        {
            KoSubpathList::iterator pathIt( it->begin() );
            for ( ; pathIt != it->end(); ++pathIt )
            {
                qDeleteAll( **pathIt );
            }
            qDeleteAll( *it );
        }
    }
    else
    {
        QList<KoSubpathList>::iterator it( m_newSubpaths.begin() );
        for ( ; it != m_newSubpaths.end(); ++it )
        {
            KoSubpathList::iterator pathIt( it->begin() );
            for ( ; pathIt != it->end(); ++pathIt )
            {
                qDeleteAll( **pathIt );
            }
            qDeleteAll( *it );
        }
    }
}

void KoParameterToPathCommand::redo()
{
    QUndoCommand::redo();
    for ( int i = 0; i < m_shapes.size(); ++i )
    {
        KoParameterShape * parameterShape = m_shapes.at( i );
        parameterShape->setModified( true );
        parameterShape->m_subpaths = m_newSubpaths[i];
        parameterShape->repaint();
    }
    m_newPointsActive = true;
}

void KoParameterToPathCommand::undo()
{
    QUndoCommand::undo();
    for ( int i = 0; i < m_shapes.size(); ++i )
    {
        KoParameterShape * parameterShape = m_shapes.at( i );
        parameterShape->setModified( false );
        parameterShape->m_subpaths = m_oldSubpaths[i];
        parameterShape->repaint();
    }
    m_newPointsActive = false;
}
