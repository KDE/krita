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

#ifndef kostyle_h
#define kostyle_h

#include <qdom.h>
#include <q3valuevector.h>
#include "KoParagStyle.h"
#include "KoUserStyleCollection.h"

class KoGenStyles;
class KoParagStyle;
class KoOasisContext;
class KoSavingContext;
class KoXmlWriter;

struct KoStyleChangeDef {
    KoStyleChangeDef() {
        paragLayoutChanged = -1;
        formatChanged = -1;
    }
    KoStyleChangeDef( int parag, int format) {
        paragLayoutChanged = parag;
        formatChanged = format;
    };
    int paragLayoutChanged;
    int formatChanged;
};
typedef QMap<KoParagStyle *, KoStyleChangeDef> KoStyleChangeDefMap;

/// TODO rename to KoParagStyleCollection - or should char styles be part of it too?
class KOTEXT_EXPORT KoStyleCollection : public KoUserStyleCollection
{
public:
    KoStyleCollection();
    ~KoStyleCollection();

    //const QPtrList<KoParagStyle> & styleList() const { return m_styleList; }

    // compat method, TODO: remove
    QStringList translatedStyleNames() const { return displayNameList(); }

    /**
     * See KoUserStyleCollection::addStyle.
     * Overloaded for convenience.
     */
    KoParagStyle* addStyle( KoParagStyle* sty ) {
        return static_cast<KoParagStyle*>( KoUserStyleCollection::addStyle( sty ) );
    }

    /**
     * Find style based on the internal name @p name.
     * Overloaded for convenience.
     */
    KoParagStyle* findStyle( const QString & name ) const {
        return static_cast<KoParagStyle*>( KoUserStyleCollection::findStyle( name, QString::fromLatin1( "Standard" ) ) );
    }

    /**
     * Find style based on the display name @p displayName.
     * Overloaded for convenience.
     */
    KoParagStyle* findStyleByDisplayName( const QString & name ) const {
        return static_cast<KoParagStyle*>( KoUserStyleCollection::findStyleByDisplayName( name ) );
    }

    /**
     * Return style number @p i.
     */
    KoParagStyle* styleAt( int i ) { return static_cast<KoParagStyle*>( m_styleList[i] ); }


    /// Import a number of styles (e.g. loaded from another document)
    void importStyles( const KoStyleCollection& styleList );

    /// Loads the entire style collection, in the OASIS OpenDocument format
    /// @return the number of new styles loaded
    int loadOasisStyles( KoOasisContext& context );

    /// Save the entire style collection in the OASIS OpenDocument format
    /// @p styleType is the STYLE_* value for this style.
    void saveOasis( KoGenStyles& styles, int styleType, KoSavingContext& context ) const;

    /// Save the text:outline-style element, mostly for OOo.
    void saveOasisOutlineStyles( KoXmlWriter& writer ) const;

    /// @return the list of outline styles
    Q3ValueVector<KoParagStyle *> outlineStyles() const;

    /// @return the [first] outline style for a given level. Can be 0 if not found.
    KoParagStyle* outlineStyleForLevel( int level ) const;

    /// @return the [first] non-outline numbered style for a given level. Can be 0 if not found.
    KoParagStyle* numberedStyleForLevel( int level ) const;

    /// @return the "default" format. There isn't really such a notion at the moment
    /// (how would the user define it? etc.), and it's usually not needed, except
    /// in very specific cases (e.g. in increaseOutlineLevel() for "not a heading")
    /// The current implementation is to return Standard or the first one in the collection.
    KoParagStyle* defaultStyle() const;

#ifndef NDEBUG
    void printDebug() const;
#endif
};

#endif
