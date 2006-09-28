#ifndef _KOTEXTFORMAT_H
#define _KOTEXTFORMAT_H

// File included by korichtext.h

/* This file is part of the KDE project
   Copyright (C) 2001 David Faure <faure@kde.org>

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

#undef S_NONE // Solaris defines it in sys/signal.h

#include <QColor>
#include <QFont>
#include <QString>
#include <q3dict.h>
#include <koffice_export.h>

class KoGenStyle;
class QFontMetrics;
class KoCharStyle;
class KoTextFormatCollection;
class KoTextZoomHandler;
class KoTextStringChar;
class KoTextParag;
class KoOasisContext;
class KoSavingContext;

/**
 * Each character (KoTextStringChar) points to a KoTextFormat that defines the
 * formatting of that character (font, bold, italic, underline, misspelled etc.).
 *
 * Formats are stored in KoTextFormatCollection and are shared for all
 * characters with the same format. The API rule is that a const KoTextFormat*
 * is a temporary format (out of collection) and a KoTextFormat* is a format
 * from the collection.
 */
class KOTEXT_EXPORT KoTextFormat
{
    friend class KoTextFormatCollection; // it sets 'collection'
    //friend class KoTextDocument;

    // Having it here allows inline methods returning d->blah, for speed
private:
class KoTextFormatPrivate
{
public:
    KoTextFormatPrivate() : m_screenFont( 0L ), m_screenFontMetrics( 0L ),
                            m_refFont( 0L ), m_refFontMetrics( 0L ),
                            m_refAscent( -1 ), m_refDescent( -1 ), m_refHeight( -1 )

    {
        memset( m_screenWidths, 0, 256 * sizeof( ushort ) );
        m_charStyle = 0L;
    }
    ~KoTextFormatPrivate()
    {
        clearCache();
    }
    void clearCache();
    // caching for speedup when formatting
    QFont* m_screenFont; // font to be used when painting (zoom-dependent)
    QFontMetrics* m_screenFontMetrics; // font metrics on screen (zoom-dependent)
    QFont* m_refFont; // font to be used when formatting text for layout units
    QFontMetrics* m_refFontMetrics; // font metrics for m_refFontMetrics
    int m_refAscent;
    int m_refDescent;
    int m_refHeight;
    int m_offsetFromBaseLine;
    ushort m_screenWidths[ 256 ];
    // m_refWidths[ 256 ] would speed things up too, but ushort might not be enough for it
    double m_relativeTextSize;
    double m_underLineWidth;
    KoCharStyle *m_charStyle;

    double m_shadowDistanceX; // 0 in both x and y means no shadow
    double m_shadowDistanceY;
    QColor m_shadowColor;
    bool m_bWordByWord;
    bool m_bHyphenation;
};

public:
    enum Flags {
	NoFlags,
	Bold = 1,
	Italic = 2,
	Underline = 4,
	Family = 8,
	Size = 16,
	Color = 32,
	Misspelled = 64,
	VAlign = 128,
        // 256 is free for use
        StrikeOut = 512, // style and type strikeout
        TextBackgroundColor = 1024,
        ExtendUnderLine = 2048, // color, style and type of underline
        Language = 4096,
        ShadowText = 8192,
        OffsetFromBaseLine = 16384,
        WordByWord = 32768,
        Attribute = 65536, // lower/upper/smallcaps
        Hyphenation = 131072,
        UnderLineWidth = 262144,

	Font = Bold | Italic | Underline | Family | Size,
        // Format means "everything"
	Format = Font | Color | Misspelled | VAlign | StrikeOut | TextBackgroundColor |
                 ExtendUnderLine | Language | ShadowText | OffsetFromBaseLine |
                 WordByWord | Attribute | Hyphenation | UnderLineWidth
    };

    enum VerticalAlignment { AlignNormal, AlignSubScript, AlignSuperScript, AlignCustom }; // QRT now has it in another order, but it's too late, we use this order in KWord's file format now !
    enum UnderlineType { U_NONE = 0, U_SIMPLE = 1, U_DOUBLE = 2, U_SIMPLE_BOLD = 3, U_WAVE = 4};
    enum StrikeOutType { S_NONE = 0, S_SIMPLE = 1, S_DOUBLE = 2, S_SIMPLE_BOLD = 3};
    enum UnderlineStyle { U_SOLID = 0 , U_DASH = 1, U_DOT = 2, U_DASH_DOT = 3, U_DASH_DOT_DOT = 4};
    enum StrikeOutStyle { S_SOLID = 0 , S_DASH = 1, S_DOT = 2, S_DASH_DOT = 3, S_DASH_DOT_DOT = 4};

    enum AttributeStyle { ATT_NONE = 0, ATT_UPPER = 1, ATT_LOWER = 2 , ATT_SMALL_CAPS};


    KoTextFormat();
    ~KoTextFormat();

    /// A simple text format with some default settings
    /// Only used for the default format
    KoTextFormat( const QFont &f, const QColor &c, const QString &_language,
                  bool hyphenation, KoTextFormatCollection *parent = 0 );

    /// A complete text format (used by KoFontDia)
    KoTextFormat( const QFont &_font,
                  VerticalAlignment _valign,
                  const QColor & _color,
                  const QColor & _backGroundColor,
                  const QColor & _underlineColor,
                  KoTextFormat::UnderlineType _underlineType,
                  KoTextFormat::UnderlineStyle _underlineStyle,
                  KoTextFormat::StrikeOutType _strikeOutType,
                  KoTextFormat::StrikeOutStyle _strikeOutStyle,
                  KoTextFormat::AttributeStyle _fontAttribute,
                  const QString &_language,
                  double _relativeTextSize,
                  int _offsetFromBaseLine,
                  bool _wordByWord,
                  bool _hyphenation,
                  double _shadowDistanceX,
                  double _shadowDistanceY,
                  const QColor& shadowColor );

    KoTextFormat( const KoTextFormat &fm );
    //KoTextFormat makeTextFormat( const QStyleSheetItem *style, const QMap<QString,QString>& attr ) const;
    KoTextFormat& operator=( const KoTextFormat &fm );
    void copyFormat( const KoTextFormat &fm, int flags );
    QColor color() const;
    QFont font() const;
    int pointSize() const { return font().pointSize(); }
    bool isMisspelled() const;
    VerticalAlignment vAlign() const;
    //int minLeftBearing() const;
    //int minRightBearing() const;
    /**
     * Return the width of one char (from a string, not necessarily from a paragraph) in LU pixels.
     * Do not call this for custom items, or for complex glyphs.
     * But this can still be used for ' ' (for parag counters), 'x' (for tabs) etc.
     */
    int width( const QChar &c ) const;
    int width( const QString &str, int pos ) const;
    int height() const; // in LU pixels
    int ascent() const; // in LU pixels
    int descent() const; // in LU pixels
    //bool useLinkColor() const;
    int offsetX() const; // in LU pixels
    int offsetY() const; // in LU pixels

    void setBold( bool b );
    void setItalic( bool b );
    void setUnderline( bool b );
    void setFamily( const QString &f );
    void setPointSize( int s );
    void setFont( const QFont &f );
    void setColor( const QColor &c );
    void setMisspelled( bool b );
    void setVAlign( VerticalAlignment a );

    bool operator==( const KoTextFormat &f ) const;
    KoTextFormatCollection *parent() const;
    void setCollection( KoTextFormatCollection *parent ) { collection = parent; }
    QString key() const;

    static QString getKey( const QFont &f, const QColor &c, bool misspelled, VerticalAlignment vAlign );

    void addRef();
    void removeRef();

    /** Return a set of flags showing the differences between this and 'format' */
    int compare( const KoTextFormat & format ) const;

    /** Call this when a text color is set to 'invalid', meaning 'whatever the
     * default for the color scheme is' */
    static QColor defaultTextColor( QPainter * painter );

    void setStrikeOutType (StrikeOutType _type);
    StrikeOutType strikeOutType()const {return m_strikeOutType;}

    void setStrikeOutStyle( StrikeOutStyle _type );
    StrikeOutStyle strikeOutStyle()const {return m_strikeOutStyle;}


    void setTextBackgroundColor(const QColor &);
    QColor textBackgroundColor()const {return m_textBackColor;}

    void setTextUnderlineColor(const QColor &);
    QColor textUnderlineColor()const {return m_textUnderlineColor;}

    void setUnderlineType (UnderlineType _type);
    UnderlineType underlineType()const {return m_underlineType;}

    void setUnderlineStyle (UnderlineStyle _type);
    UnderlineStyle underlineStyle()const {return m_underlineStyle;}

    void setLanguage( const QString & _lang);
    QString language() const { return m_language;}

    void setHyphenation( bool b );
    bool hyphenation() const { return d->m_bHyphenation; }

    // This settings is a bit different - it's cached into the KoTextFormat,
    // but it's not directly settable by the user, nor loaded/saved.
    void setUnderLineWidth( double ulw );
    double underLineWidth() const { return d->m_underLineWidth; }


    void setAttributeFont( KoTextFormat::AttributeStyle _att );
    KoTextFormat::AttributeStyle attributeFont() const { return m_attributeFont;}


    double shadowDistanceX() const { return d->m_shadowDistanceX; }
    double shadowDistanceY() const { return d->m_shadowDistanceY; }
    QColor shadowColor() const;
    /// Return the amount of pixels for the horizontal shadow distance at a given zoom level
    int shadowX( KoTextZoomHandler *zh ) const;
    /// Return the amount of pixels for the vertical shadow distance at a given zoom level
    int shadowY( KoTextZoomHandler *zh ) const;
    void setShadow( double shadowDistanceX, double shadowDistanceY, const QColor& shadowColor );
    /// Return css string for the shadow, used when saving
    QString shadowAsCss() const;
    static QString shadowAsCss( double shadowDistanceX, double shadowDistanceY, const QColor& shadowColor );
    /// Load shadow attributes from a css string, used when loading
    void parseShadowFromCss( const QString& css );

    double relativeTextSize() const { return d->m_relativeTextSize;}
    void setRelativeTextSize( double _size );

    //we store this offset into as point => int
    int offsetFromBaseLine() const { return d->m_offsetFromBaseLine;}
    void setOffsetFromBaseLine( int _offset );

    bool wordByWord() const { return d->m_bWordByWord;}
    void setWordByWord( bool _b );

    bool doubleUnderline() const { return (m_underlineType==U_DOUBLE ); }
    bool waveUnderline() const { return (m_underlineType==U_WAVE ); }
    bool underline() const { return (m_underlineType==U_SIMPLE ); }
    bool strikeOut() const { return (m_strikeOutType==S_SIMPLE ); }
    bool doubleStrikeOut() const { return (m_strikeOutType==S_DOUBLE ); }
    bool isStrikedOrUnderlined() const { return ((m_underlineType != U_NONE) ||(m_strikeOutType!=S_NONE));}

    /**
     * @return the reference point size, i.e. the size specified by the user.
     * This is the one used during formatting, independently from the zoom level.
     * This method takes care of superscript and subscript (smaller font).
     */
    float refPointSize() const;

    /**
     * @return the point size to use on screen, given @p zh
     * This method takes care of superscript and subscript (smaller font).
     */
    float screenPointSize( const KoTextZoomHandler* zh ) const;

    /**
     * @return the metrics for the reference font, i.e. with the size specified by the user.
     * This is the one used during formatting, independently from the zoom level.
     * This method takes care of superscript and subscript (smaller font).
     */
    const QFontMetrics& refFontMetrics() const;

    /**
     * Returns the font metrics for the font used at the zoom & resolution
     * given by 'zh'. Despite the name, this is probably valid for printing too.
     * This method takes care of superscript and subscript (smaller font).
     */
    const QFontMetrics& screenFontMetrics( const KoTextZoomHandler* zh ) const;

    /**
     * @return the reference font, i.e. with the size specified by the user.
     * This is used at text layout time (e.g. kotextformatter)
     */
    QFont refFont() const;

    /**
     * Returns the font to be used at the zoom & resolution given by 'zh'.
     * Despite the name, this is probably valid for printing too.
     * This method takes care of superscript and subscript (smaller font).
     */
    QFont screenFont( const KoTextZoomHandler* zh ) const;

    QFont smallCapsFont( const KoTextZoomHandler* zh, bool applyZoom ) const;

    /**
     * Return the width of one char in one paragraph.
     * Used by KoTextFormatter twice: once for the 100% zoom pointsize (via charWidthLU),
     * and once for the current zoom pointsize.
     */
    int charWidth( const KoTextZoomHandler* zh, bool applyZoom, const KoTextStringChar* c,
                   const KoTextParag* parag, int i ) const;

    /**
     * Return the width of one char in LU pixels.
     * Equivalent to ptToLayoutUnitPt( charWidth( 0L, false, c, parag, i ) )
     */
    int charWidthLU( const KoTextStringChar* c,
                     const KoTextParag* parag, int i ) const;

    void applyCharStyle( KoCharStyle *_style );
    KoCharStyle *style() const;
    static QString underlineStyleToString( UnderlineStyle _lineType );
    static QString strikeOutStyleToString( StrikeOutStyle _lineType );
    static UnderlineStyle stringToUnderlineStyle( const QString & _str );
    static StrikeOutStyle stringToStrikeOutStyle( const QString & _str );

    static QString attributeFontToString( KoTextFormat::AttributeStyle _attr );
    static AttributeStyle stringToAttributeFont( const QString & _str );

    QString displayedString( const QString& c )const;
    static QStringList underlineTypeList();
    static QStringList strikeOutTypeList();
    static QStringList fontAttributeList();
    static QStringList underlineStyleList();
    static QStringList strikeOutStyleList();

    /// Load a text format from OASIS XML
    void load( KoOasisContext& context );
    /// Save a text format to OASIS XML
    /// Only saves what differs from refFormat, if set.
    void save( KoGenStyle& gs, KoSavingContext& context, KoTextFormat * refFormat = 0 ) const;

#ifndef NDEBUG
    void printDebug();
#endif

    /// Called when the zoom or resolution changes
    void zoomChanged();

protected:
    QChar displayedChar( QChar c )const;
    void generateKey();

private:
    void update();

    QColor m_textBackColor;
    QColor m_textUnderlineColor;
    UnderlineType m_underlineType;
    StrikeOutType m_strikeOutType;
    UnderlineStyle m_underlineStyle;
    StrikeOutStyle m_strikeOutStyle;
    QString m_language;
    AttributeStyle m_attributeFont;
    KoTextFormatPrivate *d;

    QFont fn;
    QColor col;
    uint missp : 1;
    //uint linkColor : 1;
    VerticalAlignment va;
    KoTextFormatCollection *collection;
    int ref;
    QString m_key;
};

#if defined(Q_TEMPLATEDLL)
#ifndef Q_MOC_RUN
template class Q_EXPORT Q3Dict<KoTextFormat>;
#endif
#endif

class KOTEXT_EXPORT KoTextFormatCollection
{
    friend class KoTextDocument;
    friend class KoTextFormat;

public:
    KoTextFormatCollection();
    /** Constructor.
     * @param defaultFont the font to use for the default format
     * @param defaultLanguage the language to use for the default format
     * @param defaultHyphenation the hyphenation setting for the default format
     */
    KoTextFormatCollection( const QFont& defaultFont, const QColor& defaultColor,
                            const QString & defaultLanguage, bool defaultHyphenation );
    /*virtual*/ ~KoTextFormatCollection();

    void setDefaultFormat( KoTextFormat *f );
    KoTextFormat *defaultFormat() const;
    /*virtual*/ KoTextFormat *format( const KoTextFormat *f );
    /*virtual*/ KoTextFormat *format( const KoTextFormat *of, const KoTextFormat *nf, int flags );
    // Only used for the default format
//    /*virtual*/ KoTextFormat *format( const QFont &f, const QColor &c , const QString &_language, bool hyphen );
    /*virtual*/ void remove( KoTextFormat *f );
    /*virtual*/ KoTextFormat *createFormat( const KoTextFormat &f ) { return new KoTextFormat( f ); }
    // Only used for the default format
//    /*virtual*/ KoTextFormat *createFormat( const QFont &f, const QColor &c, const QString & _language, bool hyphen ) { return new KoTextFormat( f, c, _language, hyphen, this ); }
    void debug();

    // Called when the zoom or resolution changes
    void zoomChanged();

    //void setPainter( QPainter *p );
    //QStyleSheet *styleSheet() const { return sheet; }
    //void setStyleSheet( QStyleSheet *s ) { sheet = s; }
    //void updateStyles();
    //void updateFontSizes( int base );
    //void updateFontAttributes( const QFont &f, const QFont &old );

    Q3Dict<KoTextFormat> & dict() { return cKey; }

private:
    KoTextFormat *defFormat, *lastFormat, *cachedFormat;
    Q3Dict<KoTextFormat> cKey;
    KoTextFormat *cres;
    QFont cfont;
    QColor ccol;
    QString kof, knf;
    int cflags;
    //QStyleSheet *sheet;
};

inline QColor KoTextFormat::color() const
{
    return col;
}

inline QFont KoTextFormat::font() const
{
    return fn;
}

inline bool KoTextFormat::isMisspelled() const
{
    return missp;
}

inline KoTextFormat::VerticalAlignment KoTextFormat::vAlign() const
{
    return va;
}

inline bool KoTextFormat::operator==( const KoTextFormat &f ) const
{
    return key() == f.key();
}

inline KoTextFormatCollection *KoTextFormat::parent() const
{
    return collection;
}

//inline bool KoTextFormat::useLinkColor() const
//{
//    return linkColor;
//}

inline void KoTextFormatCollection::setDefaultFormat( KoTextFormat *f )
{
    defFormat = f;
}

inline KoTextFormat *KoTextFormatCollection::defaultFormat() const
{
    return defFormat;
}

#endif
