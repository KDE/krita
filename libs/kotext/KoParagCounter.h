/* This file is part of the KDE project
   Copyright (C) 2001 Shaheed Haque <srhaque@iee.org>

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

#include <QString>
#include <koffice_export.h>
class QDomElement;
class KoXmlWriter;
class KoGenStyle;
class KoTextParag;
class KoTextFormat;
class KoOasisContext;

#ifndef koparagcounter_h
#define koparagcounter_h

/**
 * This is the structure associated with a paragraph (KoTextParag),
 * to define the bullet or numbering of the paragraph.
 */
class KOTEXT_EXPORT KoParagCounter
{
public:
    KoParagCounter();

    /** Invalidate the internal cache. Use it whenever the number associated with this
     * counter may have changed. */
    void invalidate();

    /** Return the current value of the counter as a number.
     */
    int number( const KoTextParag *paragraph );
    /** Return the current value of the counter as a text.
     * This returns only the current level, e.g. "1."
     */
    QString levelText( const KoTextParag *paragraph );
    /** Return the current value of the counter as a text.
     * This returns the full text, all levels included (if displayLevels>1),
     * e.g. "1.2.1." if displayLevels==3.
     */
    QString text( const KoTextParag *paragraph );

    /**
     * Work out the width of the text required for this counter.
     * Unit : LU pixels
     */
    int width( const KoTextParag *paragraph );

    /**
     * X position of the bullet ( i.e. width of prefix )
     * Unit : LU pixels
     */
    int bulletX();

    /// KOffice-1.3 loading code
    void load( QDomElement & element );
    /// KOffice-1.3 saving code
    void save( QDomElement & element );
    /** Load from OASIS XML
     * @param heading true if heading, false if normal list
     * @param level 1-based
     * @param loadingStyle true if loading a style, false if loading a paragraph
     * @param context the context
     * @param restartNumbering if -1 then don't restart numbering, use the style value
     * @param orderedList if true, make sure the parag will will be initialised as an ordered list
     *    otherwise it may be initialised as a unordered list.
     */
    void loadOasis( KoOasisContext& context, int restartNumbering, bool orderedList, bool heading, int level, bool loadingStyle = false );
    /// Part of loadOasis that is shared with KWVariableSettings::loadOasis for footnotes/endnotes
    void loadOasisListStyle( const QDomElement& listStyle,
                             const QDomElement& listStyleProperties,
                             const QDomElement& listStyleTextProperties,
                             int restartNumbering,
                             bool orderedList, bool heading, int level, bool loadingStyle );
    /// Save as OASIS XML
    void saveOasis( KoGenStyle& listStyle, bool savingStyle = false ) const;
    /// Part of saveOasis that is shared with KoStyleCollection::saveOasisOutlineStyles
    /// and KWVariableSettings::saveOasis for footnotes/endnotes
    void saveOasisListLevel( KoXmlWriter& listLevelWriter, bool includeLevelAndProperties, bool savingStyle = false ) const;

    bool operator==( const KoParagCounter & c2 ) const;
    bool operator!=( const KoParagCounter & c2 ) const { return !(*this == c2); }

    enum Numbering
    {
        NUM_NONE = 2,       // Unnumbered. Equivalent to there being
                            // no counter structure associated with a
                            // paragraph.
        NUM_LIST = 0,       // Numbered as a list item.
        NUM_CHAPTER = 1,    // Numbered as a heading.
        NUM_FOOTNOTE = 3   // Fixed text counter, set by the code. This is used by e.g. footnotes.
    };
    enum Style // always add to the end, the numeric values are part of the DTD
    {
        STYLE_NONE = 0,
        STYLE_NUM = 1, STYLE_ALPHAB_L = 2, STYLE_ALPHAB_U = 3,
        STYLE_ROM_NUM_L = 4, STYLE_ROM_NUM_U = 5, STYLE_CUSTOMBULLET = 6,
        STYLE_CUSTOM = 7, STYLE_CIRCLEBULLET = 8, STYLE_SQUAREBULLET = 9,
        STYLE_DISCBULLET = 10, STYLE_BOXBULLET = 11
    };

    /** Numbering type and style.
     */
    Numbering numbering() const;
    void setNumbering( Numbering n );

    Style style() const;
    void setStyle( Style s );

    /**
     * Should this counter start at "startNumber" (instead of
     * being the 'last counter of the same type + 1')
     */
    bool restartCounter() const;
    void setRestartCounter( bool restart );

    /** Does this counter have a bullet style?
     */
    bool isBullet() const;
    /**
     * Helper function for finding out if a style is a bullet
     */
    static bool isBullet( Style style );

    /** The level of the numbering.
     * Depth of 0 means the major numbering. (1, 2, 3...)
     * Depth of 1 is 1.1, 1.2, 1.3 etc. */
    unsigned int depth() const;
    void setDepth( unsigned int d );

    /** Number of levels whose numbers are displayed at the current level.
     */
    int displayLevels() const;
    void setDisplayLevels( int l );

    /** Starting number.
     */
    int startNumber() const;
    void setStartNumber( int s );

    /** Prefix and suffix strings.
     */
    QString prefix() const;
    void setPrefix( QString p );
    QString suffix() const;
    void setSuffix( QString s );

    /** The character and font for STYLE_CUSTOMBULLET.
     */
    QChar customBulletCharacter() const;
    void setCustomBulletCharacter( QChar c );
    QString customBulletFont() const;
    void setCustomBulletFont( QString f );

    /** The string STYLE_CUSTOM.
     */
    QString custom() const;
    void setCustom( QString c );

    /** Counter alignment
     */
    int alignment() const;
    void setAlignment( int a );

    /**
     * Return the format to use for the counter.
     * This does no caching, it's merely to centralize code.
     */
    static KoTextFormat* counterFormat( const KoTextParag *paragraph );

    static QString makeRomanNumber( int n );
    static QString makeAlphaUpperNumber( int n );
    static QString makeAlphaLowerNumber( int n );

    static int fromRomanNumber( const QString & );
    static int fromAlphaUpperNumber( const QString & );
    static int fromAlphaLowerNumber( const QString & );

#ifndef NDEBUG
    void printRTDebug( KoTextParag* parag );
#endif

private:

    /** Return our parent paragraph, if there is such a thing. For a paragraph "1.1.",
     * the parent is the paragraph numbered "1.".
     */
    KoTextParag *parent( const KoTextParag *paragraph );

    Numbering m_numbering:3; // Numbering (maximum value: 8)
    bool m_restartCounter:1;
    bool unused:4;
    Style m_style:8;     // Style
    char m_displayLevels; // Number of levels to display (e.g. 3 => 1.2.1)
    char m_depth;

    short int m_startNumber;
    QChar m_customBulletChar;

    QString m_customBulletFont;
    QString m_custom;
    QString m_prefix;
    QString m_suffix;
    int m_align;

    class Private;
    Private *d; // define operator= and copy ctor when using this!

    /** The cached, calculated values for this counter:
     *
     *  VALUE                                       VALUE WHEN INVALID
     *  number of this counter                           -1
     *  text of this counter                             QString::null
     *  width of the label                               -1
     *  parent                                           (KoTextParag *)-1
     *  the format that was used to calculate the width  0
     */
    struct
    {
        int number;
        QString text;
        int width;
        KoTextParag *parent;
        KoTextFormat * counterFormat;
    } m_cache;
};

#endif
