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

#ifndef KOTEXTBOOKMARK_H
#define KOTEXTBOOKMARK_H

#include <koffice_export.h>
#include <QString>
#include <q3valuelist.h>
#include <QMap>
class KoTextParag;
class KoTextDocument;

/*
 * A bookmark is a name associated with a single position or a run of text (start and end) in a text document.
 */
class KOTEXT_EXPORT KoTextBookmark {
public:
    KoTextBookmark( const QString& name = QString::null /*for QValueList; remove default value when going Qt4*/ );
    KoTextBookmark( const QString& name,
                    KoTextParag* startParag, KoTextParag* endParag,
                    int start, int end );

    QString bookmarkName() const { return m_name; }
    void setBookmarkName( const QString &name ) { m_name = name; }

    // Note: the text document always m_startParag->document(), which is also m_endParag->document().
    KoTextDocument* textDocument() const;

    KoTextParag* startParag() const { return m_startParag; }
    void setStartParag( KoTextParag* parag ) { m_startParag = parag; }

    KoTextParag* endParag() const { return m_endParag; }
    void setEndParag( KoTextParag* parag ) { m_endParag = parag; }

    void setBookmarkStartIndex( int pos ) { m_startIndex = pos; }
    int bookmarkStartIndex() const { return m_startIndex; }

    void setBookmarkEndIndex( int end ) { m_endIndex = end; }
    int bookmarkEndIndex() const { return m_endIndex; }

    bool isSimple() const { return m_startParag == m_endParag && m_startIndex == m_endIndex; }

private:
    QString m_name;
    KoTextParag* m_startParag;
    KoTextParag* m_endParag;
    int m_startIndex;
    int m_endIndex;
};

class KOTEXT_EXPORT KoTextBookmarkList : public Q3ValueList<KoTextBookmark>
{
public:
    const_iterator findByName( const QString& name ) const {
        for ( const_iterator it = begin(), itend = end(); it != itend; ++it )
            if ( (*it).bookmarkName() == name )
                return it;
        return end();
    }
    iterator findByName( const QString& name ) {
        for ( iterator it = begin(), itend = end(); it != itend; ++it )
            if ( (*it).bookmarkName() == name )
                return it;
        return end();
    }
    bool removeByName( const QString& name ) {
        for ( iterator it = begin(), itend = end(); it != itend; ++it )
            if ( (*it).bookmarkName() == name ) {
                remove( it );
                return true;
            }
        return false;
    }
    /// return a map of bookmarks per paragraph. Note that multi-paragraph bookmarks
    /// will be present twice in the map.
    QMap<const KoTextParag*, KoTextBookmarkList> bookmarksPerParagraph() const {
        QMap<const KoTextParag*, KoTextBookmarkList> ret;
        for ( const_iterator it = begin(), itend = end(); it != itend; ++it ) {
            ret[ (*it).startParag() ].append( *it );
            if ( (*it).startParag() != (*it).endParag() )
                ret[ (*it).endParag() ].append( *it );
        }
        return ret;
    }
};

#endif /* KOTEXTBOOKMARK_H */

