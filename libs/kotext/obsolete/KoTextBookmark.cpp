/* This file is part of the KDE project
   Copyright (C) 2002-2003 Laurent Montel <montel@kde.org>
   Copyright (C) 2006      David Faure <faure@kde.org>

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
   Boston, MA 02110-1301, USA.
*/

#include "KoTextBookmark.h"
#include "KoTextParag.h"

KoTextBookmark::KoTextBookmark( const QString& name )
    : m_name( name ),
      m_startParag( 0 ),
      m_endParag( 0 ),
      m_startIndex( 0 ),
      m_endIndex( 0)
{
}

KoTextBookmark::KoTextBookmark( const QString &name,
                                KoTextParag *startParag, KoTextParag *endParag,
                                int start, int end )
    : m_name( name ),
      m_startParag( startParag ),
      m_endParag( endParag ),
      m_startIndex( start ),
      m_endIndex( end )
{
    if ( startParag && endParag )
        Q_ASSERT( startParag->document() == endParag->document() );
}

KoTextDocument* KoTextBookmark::textDocument() const
{
    Q_ASSERT( m_startParag->document() == m_endParag->document() );
    return m_startParag->document();
}
