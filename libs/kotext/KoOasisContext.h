/* This file is part of the KDE project
   Copyright (C) 2004-2006 David Faure <faure@kde.org>

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

#ifndef KOOASISCONTEXT_H
#define KOOASISCONTEXT_H

#include <KoOasisLoadingContext.h>
//Added by qt3to4:
#include <Q3ValueList>

class KoVariableSettings;
class KoTextParag;
class KoParagStyle;
class KoGenStyles;
class KoVariableCollection;

#include "KoListStyleStack.h"
#include "KoTextBookmark.h"

// ####### TODO rename to KoTextOasisLoadingContext (wow that's long).
// ####### maybe KoTextLoadingContext?

/**
 * Used during loading of Oasis format (and discarded at the end of the loading).
 * In addition to what KoOasisLoadingContext stores, this class has 'state' information:
 * a stack with the currently used styles (with its ancestors in the stack),
 * another one with the list styles currently applicable (one item in the stack per list level).
 *
 * @author David Faure <faure@kde.org>
 */
class KOTEXT_EXPORT KoOasisContext : public KoOasisLoadingContext
{
public:
    /**
     * Stores reference to the KoOasisStyles parsed by KoDocument.
     * Make sure that the KoOasisStyles instance outlives this KoOasisContext instance.
     * (This is the case during loaiding, when using the KoOasisStyles given by KoDocument)
     * @param doc KoDocument, needed by some field variables
     * @param varColl reference to the collection that creates and stores variables (fields)
     * @param styles reference to the KoOasisStyles parsed by KoDocument
     * @param store pointer to store, if available, for e.g. loading images.
     */
    KoOasisContext( KoDocument* doc, KoVariableCollection& varColl,
                    KoOasisStyles& styles, KoStore* store );
    ~KoOasisContext();

    KoVariableCollection& variableCollection() { return m_varColl; }

    ///// List handling

    KoListStyleStack& listStyleStack() { return m_listStyleStack; }
    QString currentListStyleName() const { return m_currentListStyleName; }
    void setCurrentListStyleName( const QString& s ) { m_currentListStyleName = s; }

    /// Used for lists (numbered paragraphs)
    /// @return true on success (a list style was found and pushed)
    bool pushListLevelStyle( const QString& listStyleName, int level );
    /// Used for outline levels
    bool pushOutlineListLevelStyle( int level );

    /// Set cursor position (set by KoTextParag upon finding the processing instruction)
    void setCursorPosition( KoTextParag* cursorTextParagraph,
                            int cursorTextIndex );

    KoTextParag* cursorTextParagraph() const { return m_cursorTextParagraph; }
    int cursorTextIndex() const { return m_cursorTextIndex; }

private:
    /// @return true on success (a list style was found and pushed)
    bool pushListLevelStyle( const QString& listStyleName, const QDomElement& fullListStyle, int level );

private:
    KoListStyleStack m_listStyleStack;
    QString m_currentListStyleName;
    KoVariableCollection& m_varColl;

    KoTextParag* m_cursorTextParagraph;
    int m_cursorTextIndex;

    class Private;
    Private *d;
};

// TODO extract non-text base class for kofficecore, see KoOasisLoadingContext

// ####### TODO rename to KoTextOasisSavingContext (wow that's long).
// ####### maybe KoTextSavingContext?

/**
 * Used during saving to Oasis format (and discarded at the end of the saving).
 *
 * Among other things, this class acts as a repository of fonts used by a
 * document during saving, in order to create the office:font-face-decls element.
 *
 * @author David Faure <faure@kde.org>
 */
class KOTEXT_EXPORT KoSavingContext
{
public:
    enum SavingMode { Store, Flat };

    /// Constructor
    /// @param mainStyles
    /// @param settings optional, used for saving the page-number in the first paragraph
    /// @param hasColumns optional, used by KoParagLayout for the type of page breaks
    /// @param savingMode either Store (a KoStore will be used) or Flat (all data must be inline in the XML)
    KoSavingContext( KoGenStyles& mainStyles, KoVariableSettings* settings = 0, bool hasColumns = false, SavingMode savingMode = Store );

    ~KoSavingContext();


    KoGenStyles& mainStyles() { return m_mainStyles; }

    /// @return the saving mode: Store (a KoStore will be used) or Flat (all data must be inline in the XML)
    SavingMode savingMode() const { return m_savingMode; }

    /// Set cursor position (so that KoTextParag can insert a processing instruction)
    void setCursorPosition( KoTextParag* cursorTextParagraph,
                            int cursorTextIndex );

    KoTextParag* cursorTextParagraph() const { return m_cursorTextParagraph; }
    int cursorTextIndex() const { return m_cursorTextIndex; }

    /// Store bookmarks [for the current text paragraph beind saved]
    /// so that KoTextParag can save them at the right place inside the text
    struct BookmarkPosition {
        BookmarkPosition() : name(), pos( -1 ), startEqualsEnd( false ) {} // for QValueList
        BookmarkPosition( const QString& nm, int p, bool simple )
            : name( nm ), pos( p ), startEqualsEnd( simple ) {}
        QString name;
        int pos;
        bool startEqualsEnd;
        bool operator<( const BookmarkPosition& rhs ) const {
            return pos < rhs.pos;
        }
    };
    typedef Q3ValueList<BookmarkPosition> BookmarkPositions;
    void setBookmarkPositions( const BookmarkPositions& bkStarts,
                               const BookmarkPositions& bkEnds ) {
        m_bookmarkStarts = bkStarts;
        m_bookmarkEnds = bkEnds;
    }
    const BookmarkPositions& bookmarkStarts() const { return m_bookmarkStarts; }
    const BookmarkPositions& bookmarkEnds() const { return m_bookmarkEnds; }

    void addFontFace( const QString& fontName );
    typedef QMap<QString, bool> FontFaces;
    void writeFontFaces( KoXmlWriter& writer );

    // See KoParagLayout::saveOasis
    bool hasColumns() const { return m_hasColumns; }

    // See KoTextParag::saveOasis
    KoVariableSettings* variableSettings() const { return m_variableSettings; }

private:
    KoGenStyles& m_mainStyles;
    SavingMode m_savingMode;

    BookmarkPositions m_bookmarkStarts, m_bookmarkEnds;

    KoTextParag* m_cursorTextParagraph;
    int m_cursorTextIndex;
    FontFaces m_fontFaces;
    KoVariableSettings* m_variableSettings;
    bool m_hasColumns;

    class Private;
    Private *d;
};

#endif /* KOOASISCONTEXT_H */
