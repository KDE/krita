/* This file is part of the KDE project
   Copyright (C) 2001-2005 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KOPARAGSTYLE_H
#define KOPARAGSTYLE_H

#include "KoTextFormat.h"
#include "KoParagLayout.h"
#include "KoUserStyle.h"
class KoGenStyles;
class QDomElement;

/**
 * A KoCharStyle is a set of formatting attributes (font, color, etc.)
 * to be applied to a run of text.
 */
class KOTEXT_EXPORT KoCharStyle : public KoUserStyle
{
public:
    /** Create a blank style (with default attributes) */
    KoCharStyle( const QString & name );

    /** Copy another style */
    KoCharStyle( const KoCharStyle & rhs ) : KoUserStyle( QString::null ) { *this = rhs; }

    /** Return a format. Don't forget to use the format collection
     * of your KoTextDocument from the result of that method. */
    const KoTextFormat & format() const;
    KoTextFormat & format();

protected:
    KoTextFormat m_format;
};

/**
 * A paragraph style is a combination of a character style
 * and paragraph-layout attributes, all grouped under a name.
 */
class KOTEXT_EXPORT KoParagStyle : public KoCharStyle
{
public:
    /** Create a blank style (with default attributes) */
    KoParagStyle( const QString & name );

    /** Copy another style */
    KoParagStyle( const KoParagStyle & rhs );

    ~KoParagStyle();

    void operator=( const KoParagStyle & );


    const KoParagLayout & paragLayout() const;
    KoParagLayout & paragLayout();

    KoParagStyle *followingStyle() const { return m_followingStyle; }
    void setFollowingStyle( KoParagStyle *fst );

    /// Saves the name, layout, the following style and the outline bool. Not the format.
    /// @deprecated  (1.3 format)
    void saveStyle( QDomElement & parentElem );
    /// Loads the name, layout and the outline bool. Not the "following style" nor the format.
    /// (1.3 format)
    void loadStyle( QDomElement & parentElem, int docVersion = 2 );

    /// Load the style from OASIS
    void loadStyle( QDomElement & styleElem, KoOasisContext& context );
    /// Save the style to OASIS
    /// Don't use, use the method in KoStyleCollection instead
    QString saveStyle( KoGenStyles& genStyles, int styleType, const QString& parentStyleName, KoSavingContext& context ) const;

    KoParagStyle * parentStyle() const {return m_parentStyle;}
    void setParentStyle( KoParagStyle *_style){ m_parentStyle = _style;}

    int inheritedParagLayoutFlag() const { return m_inheritedParagLayoutFlag; }
    int inheritedFormatFlag() const { return m_inheritedFormatFlag; }

    void propagateChanges( int paragLayoutFlag, int formatFlag );

    // If true, paragraphs with this style will be included in the table of contents
    bool isOutline() const { return m_bOutline; }
    void setOutline( bool b );

private:
    KoParagLayout m_paragLayout;
    KoParagStyle *m_followingStyle;
    KoParagStyle *m_parentStyle;
    int m_inheritedParagLayoutFlag;
    int m_inheritedFormatFlag;
    bool m_bOutline;
};

#endif /* KOPARAGSTYLE_H */

