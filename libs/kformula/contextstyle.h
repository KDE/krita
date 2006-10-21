/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
   Copyright (C) 2006 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>                  

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

#ifndef CONTEXTSTYLE_H
#define CONTEXTSTYLE_H

#include <QColor>
#include <QFont>
#include <QString>
#include <QStringList>
#include <QValueStack>
#include <kconfig.h>
#include <KoZoomHandler.h>

#include "kformuladefs.h"


KFORMULA_NAMESPACE_BEGIN

class FontStyle;
class SymbolTable;


/**
 * Contains all the style information for the formela. The idea
 * is to change the values here (user configurable) and have
 * the elements paint themselves with this information.
 *
 * All distances are stored in point. Most methods return pixel
 * values.
 */
class ContextStyle : public KoZoomHandler
{
public:

    enum Alignment { left, center, right };

    /**
     * Textstyles like in TeX. In the remaining documentation, the
     * styles are abbreviated like this:
     *
     * displayStyle: D
     *
     * textStyle: T
     *
     * scriptStyle: S
     *
     * scriptScriptStyle: SS
     **/
    enum TextStyle {
        displayStyle = 0,
        textStyle = 1,
        scriptStyle = 2,
        scriptScriptStyle = 3
    };

    enum IndexStyle {normal, cramped};

    /**
     * Build a default context style
     */
    ContextStyle();
    ~ContextStyle();

    /**
     * @param init if true fonts may be installed if needed.
     */
    void init( bool init = true );

    /**
     * @param init true if initialization may take place. This may cause font
     * installation. Mark as false when this is not intended (i. e. creating
     * configuration dialog from another component)
     */
    void readConfig( KConfig* config, bool init = true );

    bool edit() const { return m_edit; }
    void setEdit( bool e ) { m_edit = e; }

    /**
     * @returns our symbol table.
     */
    const SymbolTable& symbolTable() const;

    const FontStyle& fontStyle() const { return *m_fontStyle; }

    //copied from KoTextZoomHandler.h
    double pixelToLayoutUnitX( double x ) const;
    double pixelToLayoutUnitY( double y ) const;
    double ptToPixelX( double pt ) const
         { return pt * m_resolutionX; }
    double ptToPixelY( double pt ) const
         { return pt * m_resolutionY; }
    double ptToLayoutUnitPt( double pt ) const
    {
	    // taken from KoTextZoomHandler
	    double m_layoutUnitFactor = 20.0;
	    return pt *  m_layoutUnitFactor; }
    double ptToLayoutUnitPixX( double x_pt ) const
        { return ptToPixelX( ptToLayoutUnitPt( x_pt ) ); }
    double ptToLayoutUnitPixY( double y_pt ) const
	    { return ptToPixelY( ptToLayoutUnitPt( y_pt ) ); }
    double pixelYToPt( double y ) const
	    { return y / m_resolutionY; }
    double layoutUnitPtToPt( double lupt ) const
	    { return lupt / 20.0; }
    double layoutUnitToPixelX( double lupix ) const;
    double layoutUnitToPixelY( double lupix ) const;
    double pixelXToPt( double x ) const
	    { return x / m_resolutionX; }
    QPointF pixelToLayoutUnit( const QPointF &p ) const
        { return QPointF( pixelToLayoutUnitX( p.x() ),
		          pixelToLayoutUnitY( p.y() ) ); }
    QRectF pixelToLayoutUnit( const QRectF &r ) const
        {
	  double x = pixelToLayoutUnitX( r.x() );
	  double y = pixelToLayoutUnitY( r.y() );
	  double width = pixelToLayoutUnitX( r.x() + r.width() ) - x;
	  double height = pixelToLayoutUnitY( r.y() + r.height() ) - y;
	return QRectF( x,y,width,height ); }
    double layoutUnitToFontSize( double luSize, bool /*forPrint*/ ) const;
    double layoutUnitPtToPt( double lupt )
	    { return lupt / 20.0; }
    QPoint layoutUnitToPixel( const QPoint &p ) const
    { return QPoint( layoutUnitToPixelX( p.x() ),
	                         layoutUnitToPixelY( p.y() ) ); }	    

    
    void setZoomAndResolution( int zoom, int dpiX, int dpiY );

    /**
     * Sets the zoom by hand. This is to be used in <code>paintContent</code>.
     * @returns whether there was any change.
     */
    bool setZoomAndResolution( int zoom, double zoomX, double zoomY, bool updateViews, bool forPrint );

    bool syntaxHighlighting() const { return m_syntaxHighlighting; }
    void setSyntaxHighlighting( bool highlight ) { m_syntaxHighlighting = highlight; }

    QColor getDefaultColor()  const { return defaultColor; }
    QColor getNumberColorPlain()   const { return numberColor; }
    QColor getOperatorColorPlain() const { return operatorColor; }
    QColor getErrorColorPlain()    const { return errorColor; }
    QColor getEmptyColorPlain()    const { return emptyColor; }
    QColor getHelpColorPlain()     const { return helpColor; }
    QColor getNumberColor()   const;
    QColor getOperatorColor() const;
    QColor getErrorColor()    const;
    QColor getEmptyColor()    const;
    QColor getHelpColor()     const;

    void setDefaultColor( const QColor& );
    void setNumberColor( const QColor& );
    void setOperatorColor( const QColor& );
    void setErrorColor( const QColor& );
    void setEmptyColor( const QColor& );
    void setHelpColor( const QColor& );

    QString getFontStyle() const { return m_fontStyleName; }
    void setFontStyle( const QString& fontStyle, bool init = true );

    QFont getMathFont()       const { return mathFont; }
    QFont getBracketFont()    const { return bracketFont; }
    QFont getDefaultFont()    const { return defaultFont; }
    QFont getNameFont()       const { return nameFont; }
    QFont getNumberFont()     const { return numberFont; }
    QFont getOperatorFont()   const { return operatorFont; }
    QFont getSymbolFont()     const { return symbolFont; }

    void setMathFont( QFont f )     { defaultFont = f; }
    void setBracketFont( QFont f )  { bracketFont = f; }
    void setDefaultFont( QFont f )  { defaultFont = f; }
    void setNameFont( QFont f )     { nameFont = f; }
    void setNumberFont( QFont f )   { numberFont = f; }
    void setOperatorFont( QFont f ) { operatorFont = f; }

    //const QStringList& requestedFonts() const;
    //void setRequestedFonts( const QStringList& list );

    double getReductionFactor( TextStyle tstyle ) const;

    luPt getBaseSize() const;
    int baseSize() const { return m_baseSize; }
    void setBaseSize( int pointSize );
    void setSizeFactor( double factor );

    TextStyle getBaseTextStyle() const { return m_baseTextStyle; }
    bool isScript( TextStyle tstyle ) const { return ( tstyle == scriptStyle ) ||
                                                     ( tstyle == scriptScriptStyle ); }

    /**
     * TeX like spacings.
     */
    luPixel getSpace( TextStyle tstyle, SpaceWidth space, double factor ) const;
    luPixel getThinSpace( TextStyle tstyle, double factor ) const;
    luPixel getMediumSpace( TextStyle tstyle, double factor ) const;
    luPixel getThickSpace( TextStyle tstyle, double factor ) const;
    luPixel getQuadSpace( TextStyle tstyle, double factor ) const;

    luPixel axisHeight( TextStyle tstyle, double factor ) const;

    /**
     * Calculates the font size corresponding to the given TextStyle.
     */
    luPt getAdjustedSize( TextStyle tstyle, double factor ) const;

    /**
     * All simple lines like the one that makes up a fraction.
     */
    luPixel getLineWidth( double factor ) const;

    luPixel getEmptyRectWidth( double factor ) const;
    luPixel getEmptyRectHeight( double factor ) const;

    Alignment getMatrixAlignment() const { return center; }

    bool getCenterSymbol() const { return centerSymbol; }

    /**
     * Font-conversions a la TeX.
     *
     * For fractions (and also matrices), we have the following conversions:
     * D->T, T->S, S,SS->SS
     */
    TextStyle convertTextStyleFraction( TextStyle tstyle ) const;

    /**
     * Font-conversions a la TeX.
     *
     * For indices, we have the following conversions:
     * D->S, T->S, S,SS->SS
     */
    TextStyle convertTextStyleIndex( TextStyle tstyle ) const;

    /**
     * Index-style-conversions a la TeX.
     *
     * The function convertIndexStyleUpper is responsible for everything
     * that ends 'up', like nominators of fractions, or upper indices.
     *
     * We have the following rule:
     * normal->normal, cramped->cramped
     */
    IndexStyle convertIndexStyleUpper( IndexStyle istyle ) const {
	return istyle; }


    /**
     * Index-style-conversions a la TeX.
     *
     * The function convertIndexStyleLower is responsible for everything
     * that ends 'down', like nominators of fractions, or upper indices.
     *
     * We have the following rule:
     * normal->cramped, cramped->cramped
     */
    IndexStyle convertIndexStyleLower( IndexStyle /*istyle*/ ) const {
	return cramped; }

private:

    void setup();

    struct TextStyleValues {

        void setup( double reduction ) { reductionFactor = reduction; }

        luPt thinSpace( luPt quad ) const   { return static_cast<luPt>( reductionFactor*static_cast<double>( quad )/6. ); }
        luPt mediumSpace( luPt quad ) const { return static_cast<luPt>( reductionFactor*static_cast<double>( quad )*2./9. ); }
        luPt thickSpace( luPt quad ) const  { return static_cast<luPt>( reductionFactor*static_cast<double>( quad )*5./18. ); }
        luPt quadSpace( luPt quad ) const   { return quad; }

        luPixel axisHeight( luPixel height ) const { return static_cast<luPixel>( reductionFactor*height ); }
        double reductionFactor;
    };

    TextStyleValues textStyleValues[ 4 ];

    QFont mathFont;
    QFont bracketFont;
    QFont defaultFont;
    QFont nameFont;
    QFont numberFont;
    QFont operatorFont;
    QFont symbolFont;

    //QStringList m_requestedFonts;

    QColor defaultColor;
    QColor numberColor;
    QColor operatorColor;
    QColor errorColor;
    QColor emptyColor;
    QColor helpColor;

    /**
     * The cursors movement style. You need to notify each cursor
     * if you change this.
     */
    bool linearMovement;

    /**
     * The (font) size of the formula's main sequence.
     */
    int m_baseSize;

    /**
     * Hack! Each formula might set this to a value not too far from one
     * to get a size different from the default one.
     */
    double m_sizeFactor;

    /**
     * The base text style of the formula.
     **/
    TextStyle m_baseTextStyle;

    /**
     * The thickness of our lines.
     */
    pt lineWidth;

    /**
     * Size of one quad.
     */
    luPt quad;

    /**
     * Distance between base line and axis.
     */
    luPixel m_axisHeight;

    /**
     * true means to center the symbol between its indexes.
     * false means alignment to the right.
     */
    bool centerSymbol;

    /**
     * Whether we want colored formulae.
     */
    bool m_syntaxHighlighting;

    /**
     * Whether we are in edit mode.
     */
    bool m_edit;

    /**
     * The symbols/names that are "known" to the system.
     */
    //SymbolTable table;

    FontStyle* m_fontStyle;
    QString m_fontStyleName;
};

// Section 3.3.4.2, default values
const double scriptsizemultiplier   = 0.71;
const double scriptminsize          = 8;
const double veryverythinmathspace  = 0.0555556;
const double verythinmathspace      = 0.111111;
const double thinmathspace          = 0.166667;
const double mediummathspace        = 0.222222;
const double thickmathspace         = 0.277778;
const double verythickmathspace     = 0.333333;
const double veryverythickmathspace = 0.388889;

class StyleAttributes {
 public:
    double sizeFactor() const ;
    bool customMathVariant() const ;
    CharStyle charStyle() const ;
    CharFamily charFamily() const ;
    QColor color() const ;
    QColor background() const ;
    QFont font() const ;
    bool fontWeight() const ;
    bool customFontWeight() const ;
    bool fontStyle() const ;
    bool customFontStyle() const ;
    bool customFont() const ;

    int scriptLevel() const ;
    double scriptSizeMultiplier() const ;
    double scriptMinSize() const ;
    double veryVeryThinMathSpace() const ;
    double veryThinMathSpace() const ;
    double thinMathSpace() const ;
    double mediumMathSpace() const ;
    double thickMathSpace() const ;
    double veryThickMathSpace() const ;
    double veryVeryThickMathSpace() const ;
    bool displayStyle() const ;
    bool customDisplayStyle() const ;

    double getSpace( SizeType type, double length ) const ;

    void setSizeFactor( double s ) { m_size.push( s ); }
    void setCustomMathVariant( bool cmv ) { m_customMathVariant.push( cmv ); }
    void setCharStyle( CharStyle cs ) { m_charStyle.push( cs ); }
    void setCharFamily( CharFamily cf ) { m_charFamily.push( cf ); }
    void setColor( const QColor& c ) { m_color.push( c ); }
    void setBackground( const QColor& bg ) { m_background.push( bg ); }
    void setFont( const QFont& f ) { m_font.push( f ); }
    void setCustomFont( bool cf ) { m_customFontFamily.push ( cf ); }
    void setCustomFontWeight( bool cfw ) { m_customFontWeight.push( cfw ); }
    void setFontWeight( bool fw ) { m_fontWeight.push( fw ); }
    void setCustomFontStyle( bool cfs ) { m_customFontStyle.push( cfs ); }
    void setFontStyle( bool fs ) { m_fontStyle.push( fs ); }

    void setScriptLevel( int s ) { m_scriptLevel.push( s ); }
    void setScriptSizeMultiplier( double s ) { m_scriptSizeMultiplier.push( s ); }
    void setScriptMinSize( double s ) { m_scriptMinSize.push( s ); }
    void setVeryVeryThinMathSpace( double s ) { m_veryVeryThinMathSpace.push( s ); }
    void setVeryThinMathSpace( double s ) { m_veryThinMathSpace.push( s ); }
    void setThinMathSpace( double s ) { m_thinMathSpace.push( s ); }
    void setMediumMathSpace( double s ) { m_mediumMathSpace.push( s ); }
    void setThickMathSpace( double s ) { m_thickMathSpace.push( s ); }
    void setVeryThickMathSpace( double s ) { m_veryThickMathSpace.push( s ); }
    void setVeryVeryThickMathSpace( double s ) { m_veryVeryThickMathSpace.push( s ); }
    void setDisplayStyle( bool ds ) { m_displayStyle.push( ds ); }
    void setCustomDisplayStyle( bool cds ) { m_customDisplayStyle.push( cds ); }

    void reset();
    void resetSize();
    void resetCharStyle();
    void resetCharFamily();
    void resetColor();
    void resetBackground();
    void resetFontFamily();
    void resetFontWeight();
    void resetFontStyle();

    void resetScriptLevel();
    void resetScriptSizeMultiplier();
    void resetScriptMinSize();
    void resetVeryVeryThinMathSpace();
    void resetVeryThinMathSpace();
    void resetThinMathSpace();
    void resetMediumMathSpace();
    void resetThickMathSpace();
    void resetVeryThickMathSpace();
    void resetVeryVeryThickMathSpace();
    void resetDisplayStyle();

 private:
    // Size of the font in points (mathsize / fontsize)
    QValueStack<double> m_size;

    // Whether a custom mathvariant attribute is in use
    QValueStack<bool> m_customMathVariant;

    // Font style (mathvariant, fontweight, fontstyle)
    QValueStack<CharStyle> m_charStyle;

    // Font family (mathvariant)
    QValueStack<CharFamily> m_charFamily;

    // Foreground color (mathcolor, color)
    QValueStack<QColor> m_color;

    // Background color (mathbackground)
    QValueStack<QColor> m_background;

    // Font family (fontfamily)
    QValueStack<QFont> m_font;

    // Whether a custom fontfamily attribute is in use (instead of CharFamily)
    QValueStack<bool> m_customFontFamily;

    // Font Weight (fontweight)
    QValueStack<bool> m_fontWeight;

    // Whether a custom fontweight attribute is in use
    QValueStack<bool> m_customFontWeight;

    // Font Style (fontstyle)
    QValueStack<bool> m_fontStyle;

    // Whether a custom fontstyle attribute is in use
    QValueStack<bool> m_customFontStyle;

    QValueStack<int> m_scriptLevel;
    QValueStack<double> m_scriptSizeMultiplier;
    QValueStack<double> m_scriptMinSize;
    QValueStack<double> m_veryVeryThinMathSpace;
    QValueStack<double> m_veryThinMathSpace;
    QValueStack<double> m_thinMathSpace;
    QValueStack<double> m_mediumMathSpace;
    QValueStack<double> m_thickMathSpace;
    QValueStack<double> m_veryThickMathSpace;
    QValueStack<double> m_veryVeryThickMathSpace;
    QValueStack<bool> m_displayStyle;
    QValueStack<bool> m_customDisplayStyle;
};


KFORMULA_NAMESPACE_END

#endif // CONTEXTSTYLE_H
