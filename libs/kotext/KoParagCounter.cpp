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

#include "KoParagCounter.h"
#include "KoTextParag.h"
#include "KoTextZoomHandler.h"
#include "KoTextFormat.h"
#include "KoTextDocument.h"
#include "KoOasisContext.h"
#include <KoXmlWriter.h>
#include <KoGenStyles.h>
#include <KoXmlNS.h>
#include <kdebug.h>
#include <qdom.h>
#include <QBuffer>
//Added by qt3to4:
#include <QByteArray>

static KoTextParag * const INVALID_PARAG = (KoTextParag *)-1;

KoParagCounter::KoParagCounter()
{
    m_numbering = NUM_NONE;
    m_style = STYLE_NONE;
    m_depth = 0;
    m_startNumber = 1;
    m_displayLevels = 1;
    m_restartCounter = false;
    m_customBulletChar = QChar( '-' );
    m_customBulletFont = QString::null;
    m_align = Qt::AlignLeft;
    invalidate();
}

bool KoParagCounter::operator==( const KoParagCounter & c2 ) const
{
    // ## This is kinda wrong. Unused fields (depending on the counter style) shouldn't be compared.
    return (m_numbering==c2.m_numbering &&
            m_style==c2.m_style &&
            m_depth==c2.m_depth &&
            m_startNumber==c2.m_startNumber &&
            m_displayLevels==c2.m_displayLevels &&
            m_restartCounter==c2.m_restartCounter &&
            m_prefix==c2.m_prefix &&
            m_suffix==c2.m_suffix &&
            m_customBulletChar==c2.m_customBulletChar &&
            m_customBulletFont==c2.m_customBulletFont &&
            m_align==c2.m_align &&
            m_custom==c2.m_custom);
}

QString KoParagCounter::custom() const
{
    return m_custom;
}

QChar KoParagCounter::customBulletCharacter() const
{
    return m_customBulletChar;
}

QString KoParagCounter::customBulletFont() const
{
    return m_customBulletFont;
}

unsigned int KoParagCounter::depth() const
{
    return m_depth;
}

void KoParagCounter::invalidate()
{
    m_cache.number = -1;
    m_cache.text = QString::null;
    m_cache.width = -1;
    m_cache.parent = INVALID_PARAG;
    m_cache.counterFormat = 0;
}

bool KoParagCounter::isBullet( Style style ) // static
{
    switch ( style )
    {
    case STYLE_DISCBULLET:
    case STYLE_SQUAREBULLET:
    case STYLE_BOXBULLET:
    case STYLE_CIRCLEBULLET:
    case STYLE_CUSTOMBULLET:
        return true;
    default:
        return false;
    }
}

bool KoParagCounter::isBullet() const
{
    return isBullet( m_style );
}

void KoParagCounter::load( QDomElement & element )
{
    m_numbering = static_cast<Numbering>( element.attribute("numberingtype", "2").toInt() );
    m_style = static_cast<Style>( element.attribute("type").toInt() );
    // Old docs have this:
    if ( m_numbering == NUM_LIST && m_style == STYLE_NONE )
        m_numbering = NUM_NONE;
    m_depth = element.attribute("depth").toInt();
    m_customBulletChar = QChar( element.attribute("bullet").toInt() );
    m_prefix = element.attribute("lefttext");
    if ( m_prefix.toLower() == "(null)" ) // very old kword thing
        m_prefix = QString::null;
    m_suffix = element.attribute("righttext");
    if ( m_suffix.toLower() == "(null)" )
        m_suffix = QString::null;
    QString s = element.attribute("start");
    if ( s.isEmpty() )
        m_startNumber = 1;
    else if ( s[0].isDigit() )
        m_startNumber = s.toInt();
    else // support for very-old files
        m_startNumber = s.toLower()[0].toLatin1() - 'a' + 1;
    s = element.attribute("display-levels");
    if ( !s.isEmpty() )
        m_displayLevels = qMin( s.toInt(), m_depth+1 ); // can't be > depth+1
    else // Not specified -> compat with koffice-1.2: make equal to depth+1
        m_displayLevels = m_depth+1;
    m_customBulletFont = element.attribute("bulletfont");
    m_custom = element.attribute("customdef");
    m_align = element.attribute("align", "0").toInt(); //AlignAuto as defeult
    QString restart = element.attribute("restart");
    m_restartCounter = (restart == "true") || (restart == "1");
    invalidate();
}

static int importCounterType( QChar numFormat )
{
    if ( numFormat == '1' )
        return KoParagCounter::STYLE_NUM;
    if ( numFormat == 'a' )
        return KoParagCounter::STYLE_ALPHAB_L;
    if ( numFormat == 'A' )
        return KoParagCounter::STYLE_ALPHAB_U;
    if ( numFormat == 'i' )
        return KoParagCounter::STYLE_ROM_NUM_L;
    if ( numFormat == 'I' )
        return KoParagCounter::STYLE_ROM_NUM_U;
    return KoParagCounter::STYLE_NONE;
}

// should only be called with style != none and != a bullet.
static QChar exportCounterType( KoParagCounter::Style style )
{
    static const int s_oasisCounterTypes[] =
        { '\0', '1', 'a', 'A', 'i', 'I',
          '\0', '\0', // custombullet, custom
          0x2022, // circle -> small disc
          0xE00A, // square
          0x25CF, // disc -> large disc
          0x27A2  // box -> right-pointing triangle
        };
    return QChar( s_oasisCounterTypes[ style ] );
}

void KoParagCounter::loadOasis( KoOasisContext& context, int restartNumbering,
                                bool orderedList, bool heading, int level, bool loadingStyle )
{
    const QDomElement listStyle = context.listStyleStack().currentListStyle();
    const QDomElement listStyleProperties = context.listStyleStack().currentListStyleProperties();
    const QDomElement listStyleTextProperties = context.listStyleStack().currentListStyleTextProperties();
    loadOasisListStyle( listStyle, listStyleProperties, listStyleTextProperties,
                        restartNumbering, orderedList, heading, level, loadingStyle );
}

void KoParagCounter::loadOasisListStyle( const QDomElement& listStyle,
                                         const QDomElement& listStyleProperties,
                                         const QDomElement& listStyleTextProperties,
                                         int restartNumbering,
                                         bool orderedList, bool heading, int level,
                                         bool loadingStyle )
{
    m_numbering = heading ? NUM_CHAPTER : NUM_LIST;
    m_depth = level - 1; // depth start at 0
    // restartNumbering can either be provided by caller, or taken from the style
    if ( restartNumbering == -1 && listStyle.hasAttributeNS( KoXmlNS::text, "start-value" ) )
        restartNumbering = listStyle.attributeNS( KoXmlNS::text, "start-value", QString::null ).toInt();

    // styles have a start-value, but that doesn't mean restartNumbering, as it does for paragraphs
    m_restartCounter = loadingStyle ? false : ( restartNumbering != -1 );
    m_startNumber = ( restartNumbering != -1 ) ? restartNumbering : 1;
    //kDebug() << k_funcinfo << "IN: restartNumbering=" << restartNumbering << " OUT: m_restartCounter=" << m_restartCounter << " m_startNumber=" << m_startNumber << endl;

    m_prefix = listStyle.attributeNS( KoXmlNS::style, "num-prefix", QString::null );
    m_suffix = listStyle.attributeNS( KoXmlNS::style, "num-suffix", QString::null );

    if ( orderedList || heading ) {
        m_style = static_cast<Style>( importCounterType( listStyle.attributeNS( KoXmlNS::style, "num-format", QString::null)[0] ) );
        QString dl = listStyle.attributeNS( KoXmlNS::text, "display-levels", QString::null );
        m_displayLevels = dl.isEmpty() ? 1 : dl.toInt();
    } else { // bullets, see 3.3.6 p138
        m_style = STYLE_CUSTOMBULLET;
        QString bulletChar = listStyle.attributeNS( KoXmlNS::text, "bullet-char", QString::null );
        if ( !bulletChar.isEmpty() ) {
            // Reverse engineering, I found those codes:
            switch( bulletChar[0].unicode() ) {
            case 0x2022: // small disc -> circle
                m_style = STYLE_CIRCLEBULLET;
                break;
            case 0x25CF: // large disc -> disc
            case 0xF0B7: // #113361
                m_style = STYLE_DISCBULLET;
                break;
            case 0xE00C: // losange - TODO in kotext. Not in OASIS either (reserved Unicode area!)
                m_style = STYLE_BOXBULLET;
                break;
            case 0xE00A: // square. Not in OASIS (reserved Unicode area!), but used in both OOo and kotext.
                m_style = STYLE_SQUAREBULLET;
                break;
            case 0x27A2: // two-colors right-pointing triangle
                         // mapping (both ways) to box for now.
                m_style = STYLE_BOXBULLET;
                break;
            default:
                kDebug() << "Unhandled bullet code 0x" << QString::number( (uint)m_customBulletChar.unicode(), 16 ) << endl;
                // fallback
            case 0x2794: // arrow
            case 0x2717: // cross
            case 0x2714: // checkmark
                m_customBulletChar = bulletChar[0];
                // often StarSymbol when it comes from OO; doesn't matter, Qt finds it in another font if needed.
                if ( listStyleProperties.hasAttributeNS( KoXmlNS::style, "font-name" ) )
                {
                    m_customBulletFont = listStyleProperties.attributeNS( KoXmlNS::style, "font-name", QString::null );
                    kDebug() << "m_customBulletFont style:font-name = " << listStyleProperties.attributeNS( KoXmlNS::style, "font-name", QString::null ) << endl;
                }
                else if ( listStyleTextProperties.hasAttributeNS( KoXmlNS::fo, "font-family" ) )
                {
                    m_customBulletFont = listStyleTextProperties.attributeNS( KoXmlNS::fo, "font-family", QString::null );
                    kDebug() << "m_customBulletFont fo:font-family = " << listStyleTextProperties.attributeNS( KoXmlNS::fo, "font-family", QString::null ) << endl;
                }
                // ## TODO in fact we're supposed to read it from the style pointed to by text:style-name
                break;
            }
        } else { // can never happen
            m_style = STYLE_DISCBULLET;
        }
    }
    invalidate();
}

void KoParagCounter::saveOasis( KoGenStyle& listStyle, bool savingStyle ) const
{
    Q_ASSERT( (Style)m_style != STYLE_NONE );

    // Prepare a sub-xmlwriter for the list-level-style-* element
    QBuffer buffer;
    buffer.open( QIODevice::WriteOnly );
    KoXmlWriter listLevelWriter( &buffer, 3 /*indentation*/ );
    const char* tagName = isBullet() ? "text:list-level-style-bullet" : "text:list-level-style-number";
    listLevelWriter.startElement( tagName );

    saveOasisListLevel( listLevelWriter, true, savingStyle );

    listLevelWriter.endElement();
    const QString listLevelContents = QString::fromUtf8( buffer.buffer(), buffer.buffer().size() );
    listStyle.addChildElement( tagName, listLevelContents );
}

void KoParagCounter::saveOasisListLevel( KoXmlWriter& listLevelWriter, bool includeLevelAndProperties, bool savingStyle ) const
{
    if ( includeLevelAndProperties ) // false when called for footnotes-configuration
        listLevelWriter.addAttribute( "text:level", (int)m_depth + 1 );
    // OASIS allows to specify a text:style, the character style to use for the numbering...
    // We currently always format as per the first character of the paragraph, but that's not perfect.

    if ( isBullet() )
    {
        QChar bulletChar;
        if ( (Style)m_style == STYLE_CUSTOMBULLET )
        {
            bulletChar = m_customBulletChar;
            // TODO font (text style)
        }
        else
        {
            bulletChar = exportCounterType( (Style)m_style );
        }
        listLevelWriter.addAttribute( "text:bullet-char", QString( bulletChar ) );
    }
    else
    {
        if ( includeLevelAndProperties ) // not for KWVariableSettings
            listLevelWriter.addAttribute( "text:display-levels", m_displayLevels );
        if ( (Style)m_style == STYLE_CUSTOM )
            ; // not implemented
        else
            listLevelWriter.addAttribute( "style:num-format", QString( exportCounterType( (Style)m_style ) ) );

        // m_startNumber/m_restartCounter is saved by kotextparag itself, except for styles.
        if ( savingStyle && m_restartCounter ) {
            listLevelWriter.addAttribute( "text:start-value", m_startNumber );
        }

    }
    // m_numbering isn't saved, it's set depending on context (NUM_CHAPTER for headings).

    listLevelWriter.addAttribute( "style:num-prefix", m_prefix );
    listLevelWriter.addAttribute( "style:num-suffix", m_suffix );

    if ( includeLevelAndProperties ) // false when called for footnotes-configuration
    {
        listLevelWriter.startElement( "style:list-level-properties" );
        listLevelWriter.addAttribute( "fo:text-align", KoParagLayout::saveOasisAlignment( (Qt::AlignmentFlag)m_align ) );
        // OASIS has other style properties: text:space-before (indent), text:min-label-width (TODO),
        // text:min-label-distance, style:font-name (for bullets), image size and vertical alignment.
        listLevelWriter.endElement(); // style:list-level-properties
    }
}

int KoParagCounter::number( const KoTextParag *paragraph )
{
    // Return cached value if possible.
    if ( m_cache.number != -1 )
        return m_cache.number;

    // Should we start a new list?
    if ( m_restartCounter ) {
        Q_ASSERT( m_startNumber != -1 );
        m_cache.number = m_startNumber;
        return m_startNumber;
    }

    // Go looking for another paragraph at the same level or higher level.
    // (This code shares logic with parent())
    KoTextParag *otherParagraph = paragraph->prev();
    KoParagCounter *otherCounter;

    switch ( m_numbering )
    {
    case NUM_NONE:
        // This should not occur!
    case NUM_FOOTNOTE:
        m_cache.number = 0;
        break;
    case NUM_CHAPTER:
        m_cache.number = m_startNumber;
        // Go upwards...
        while ( otherParagraph )
        {
            otherCounter = otherParagraph->counter();
            if ( otherCounter &&               // ...look at numbered paragraphs only
                ( otherCounter->m_numbering == NUM_CHAPTER ) &&     // ...same number type.
                ( otherCounter->m_depth <= m_depth ) )        // ...same or higher level.
            {
                if ( ( otherCounter->m_depth == m_depth ) &&
                   ( otherCounter->m_style == m_style ) )
                {
                    // Found a preceding paragraph of exactly our type!
                    m_cache.number = otherCounter->number( otherParagraph ) + 1;
                }
                else
                {
                    // Found a preceding paragraph of higher level!
                    m_cache.number = m_startNumber;
                }
                break;
            }
            otherParagraph = otherParagraph->prev();
        }
        break;
    case NUM_LIST:
        m_cache.number = m_startNumber;
        // Go upwards...
        while ( otherParagraph )
        {
            otherCounter = otherParagraph->counter();
            if ( otherCounter )                                         // look at numbered paragraphs only
            {
                if ( ( otherCounter->m_numbering == NUM_LIST ) &&       // ...same number type.
                     !isBullet( otherCounter->m_style ) &&    // ...not a bullet
                    ( otherCounter->m_depth <= m_depth ) )    // ...same or higher level.
                {
                    if ( ( otherCounter->m_depth == m_depth ) &&
                       ( otherCounter->m_style == m_style ) )
                    {
                        // Found a preceding paragraph of exactly our type!
                        m_cache.number = otherCounter->number( otherParagraph ) + 1;
                    }
                    else
                    {
                        // Found a preceding paragraph of higher level!
                        m_cache.number = m_startNumber;
                    }
                    break;
                }
                else
                if ( otherCounter->m_numbering == NUM_CHAPTER )        // ...heading number type.
                {
                    m_cache.number = m_startNumber;
                    break;
                }
            }
/*            else
            {
                // There is no counter at all.
                m_cache.number = m_startNumber;
                break;
            }*/
            otherParagraph = otherParagraph->prev();
        }
        break;
    }
    Q_ASSERT( m_cache.number != -1 );
    return m_cache.number;
}

KoParagCounter::Numbering KoParagCounter::numbering() const
{
    return m_numbering;
}

// Go looking for another paragraph at a higher level.
KoTextParag *KoParagCounter::parent( const KoTextParag *paragraph )
{
    // Return cached value if possible.
    if ( m_cache.parent != INVALID_PARAG )
        return m_cache.parent;

    KoTextParag *otherParagraph = paragraph->prev();
    KoParagCounter *otherCounter;

    // (This code shares logic with number())
    switch ( m_numbering )
    {
    case NUM_NONE:
        // This should not occur!
    case NUM_FOOTNOTE:
        otherParagraph = 0L;
        break;
    case NUM_CHAPTER:
        // Go upwards while...
        while ( otherParagraph )
        {
            otherCounter = otherParagraph->counter();
            if ( otherCounter &&                                        // ...numbered paragraphs.
                ( otherCounter->m_numbering == NUM_CHAPTER ) &&         // ...same number type.
                ( otherCounter->m_depth < m_depth ) )         // ...higher level.
            {
                break;
            }
            otherParagraph = otherParagraph->prev();
        }
        break;
    case NUM_LIST:
        // Go upwards while...
        while ( otherParagraph )
        {
            otherCounter = otherParagraph->counter();
            if ( otherCounter )                                         // ...numbered paragraphs.
            {
                if ( ( otherCounter->m_numbering == NUM_LIST ) &&       // ...same number type.
                     !isBullet( otherCounter->m_style ) &&    // ...not a bullet
                    ( otherCounter->m_depth < m_depth ) )     // ...higher level.
                {
                    break;
                }
                else
                if ( otherCounter->m_numbering == NUM_CHAPTER )         // ...heading number type.
                {
                    otherParagraph = 0L;
                    break;
                }
            }
            otherParagraph = otherParagraph->prev();
        }
        break;
    }
    m_cache.parent = otherParagraph;
    return m_cache.parent;
}

QString KoParagCounter::prefix() const
{
    return m_prefix;
}

void KoParagCounter::save( QDomElement & element )
{
    element.setAttribute( "type", static_cast<int>( m_style ) );
    element.setAttribute( "depth", m_depth );
    if ( (Style)m_style == STYLE_CUSTOMBULLET )
    {
        element.setAttribute( "bullet", m_customBulletChar.unicode() );
        if ( !m_customBulletFont.isEmpty() )
            element.setAttribute( "bulletfont", m_customBulletFont );
    }
    if ( !m_prefix.isEmpty() )
        element.setAttribute( "lefttext", m_prefix );
    if ( !m_suffix.isEmpty() )
        element.setAttribute( "righttext", m_suffix );
    if ( m_startNumber != 1 )
        element.setAttribute( "start", m_startNumber );
    //if ( m_displayLevels != m_depth ) // see load()
        element.setAttribute( "display-levels", m_displayLevels );
    // Don't need to save NUM_FOOTNOTE, it's updated right after loading
    if ( m_numbering != NUM_NONE && m_numbering != NUM_FOOTNOTE )
        element.setAttribute( "numberingtype", static_cast<int>( m_numbering ) );
    if ( !m_custom.isEmpty() )
        element.setAttribute( "customdef", m_custom );
    if ( m_restartCounter )
        element.setAttribute( "restart", "true" );
    if ( !m_cache.text.isEmpty() )
        element.setAttribute( "text", m_cache.text );
    element.setAttribute( "align", m_align );
}

void KoParagCounter::setCustom( QString c )
{
    m_custom = c;
    invalidate();
}

void KoParagCounter::setCustomBulletCharacter( QChar c )
{
    m_customBulletChar = c;
    invalidate();
}

void KoParagCounter::setCustomBulletFont( QString f )
{
    m_customBulletFont = f;
    invalidate();
}

void KoParagCounter::setDepth( unsigned int d )
{
    m_depth = d;
    invalidate();
}

void KoParagCounter::setNumbering( Numbering n )
{
    m_numbering = n;
    invalidate();
}

void KoParagCounter::setPrefix( QString p )
{
    m_prefix = p;
    invalidate();
}
void KoParagCounter::setStartNumber( int s )
{
    m_startNumber = s;
    invalidate();
}

void KoParagCounter::setDisplayLevels( int l )
{
    m_displayLevels = l;
    invalidate();
}

void KoParagCounter::setAlignment( int a )
{
    m_align = a;
    invalidate();
}

void KoParagCounter::setStyle( Style s )
{
    m_style = s;
    invalidate();
}

void KoParagCounter::setSuffix( QString s )
{
    m_suffix = s;
    invalidate();
}

int KoParagCounter::startNumber() const
{
    return m_startNumber;
}

int KoParagCounter::displayLevels() const
{
    return m_displayLevels;
}

int KoParagCounter::alignment() const
{
    return m_align;
}

KoParagCounter::Style KoParagCounter::style() const
{
    return m_style;
}

QString KoParagCounter::suffix() const
{
    return m_suffix;
}

bool KoParagCounter::restartCounter() const
{
    return m_restartCounter;
}

void KoParagCounter::setRestartCounter( bool restart )
{
    m_restartCounter = restart;
    invalidate();
}

// Return the text for that level only
QString KoParagCounter::levelText( const KoTextParag *paragraph )
{
    if ( m_numbering == NUM_NONE )
        return "";

    bool bullet = isBullet( m_style );

    if ( bullet && m_numbering == NUM_CHAPTER ) {
        // Shome mishtake surely! (not sure how it can happen though)
        m_style = STYLE_NUM;
        bullet = false;
    }

    QString text;
    if ( !bullet )
    {
        // Ensure paragraph number is valid.
        number( paragraph );

        switch ( m_style )
        {
        case STYLE_NONE:
        if ( m_numbering == NUM_LIST )
            text = ' ';
        break;
        case STYLE_NUM:
            text.setNum( m_cache.number );
            break;
        case STYLE_ALPHAB_L:
            text = makeAlphaLowerNumber( m_cache.number );
            break;
        case STYLE_ALPHAB_U:
            text = makeAlphaUpperNumber( m_cache.number );
            break;
        case STYLE_ROM_NUM_L:
            text = makeRomanNumber( m_cache.number ).toLower();
            break;
        case STYLE_ROM_NUM_U:
            text = makeRomanNumber( m_cache.number ).toUpper();
            break;
        case STYLE_CUSTOM:
            ////// TODO
        default: // shut up compiler
            text.setNum( m_cache.number );
            break;
        }
    }
    else
    {
        switch ( m_style )
        {
            // --- these are used in export filters but are ignored by KoTextParag::drawLabel (for bulleted lists - which they are :))  ---
        case KoParagCounter::STYLE_DISCBULLET:
            text = '*';
            break;
        case KoParagCounter::STYLE_SQUAREBULLET:
            text = '#';
            break;
        case KoParagCounter::STYLE_BOXBULLET:
            text = '=';  // think up a better character
            break;
        case KoParagCounter::STYLE_CIRCLEBULLET:
            text = 'o';
            break;
        case KoParagCounter::STYLE_CUSTOMBULLET:
            text = m_customBulletChar;
            break;
        default: // shut up compiler
            break;
        }
    }
    return text;
}

// Return the full text to be displayed
QString KoParagCounter::text( const KoTextParag *paragraph )
{
    // Return cached value if possible.
    if ( !m_cache.text.isNull() )
        return m_cache.text;

    // If necessary, grab the text of the preceding levels.
    if ( m_displayLevels > 1 && m_numbering != NUM_NONE )
    {
        KoTextParag* p = parent( paragraph );
        int displayLevels = qMin( (int)m_displayLevels, m_depth+1 ); // can't be >depth+1
        for ( int level = 1 ; level < displayLevels ; ++level ) {
            //kDebug() << "additional level=" << level << "/" << displayLevels-1 << endl;
            if ( p )
            {
                KoParagCounter* counter = p->counter();
                QString str = counter->levelText( p );
                // If the preceding level is a bullet, replace it with blanks.
                if ( counter->isBullet() )
                    for ( unsigned i = 0; i < str.length(); i++ )
                        str[i] = ' ';

                str.append('.'); // hardcoded on purpose (like OO) until anyone complains

                // Find the number of missing parents, and add dummy text for them.
                int missingParents = m_depth - level - p->counter()->m_depth;
                //kDebug() << "levelText = " << str << " missingParents=" << missingParents << endl;
                level += missingParents;
                for ( ; missingParents > 0 ; --missingParents )
                    // Each missing level adds a "0"
                    str.append( "0." );

                m_cache.text.prepend( str );
                // Prepare next iteration
                if ( level < displayLevels ) // no need to calc it if we won't use it
                    p = counter->parent( p );
            }
            else // toplevel parents are missing
            {
                // Special case for one-paragraph-documents like preview widgets
                KoTextDocument* textdoc = paragraph->textDocument();
                if ( paragraph == textdoc->firstParag() && paragraph == textdoc->lastParag() )
                    m_cache.text.prepend( "1." );
                else
                    m_cache.text.prepend( "0." );
            }
        }

    }

    //kDebug() << "result: " << m_cache.text << " + " << levelText( paragraph ) << endl;
    // Now add text for this level.
    m_cache.text.append( levelText( paragraph ) );

    // Now apply prefix and suffix
    // We want the '.' to be before the number in a RTL parag,
    // but we can't paint the whole string using QPainter::RTL direction, otherwise
    // '10' becomes '01'.
    m_cache.text.prepend( paragraph->string()->isRightToLeft() ? suffix() : prefix() );
    m_cache.text.append( paragraph->string()->isRightToLeft() ? prefix() : suffix() );
    return m_cache.text;
}

int KoParagCounter::width( const KoTextParag *paragraph )
{
    // Return cached value if possible.
    if ( m_cache.width != -1 && counterFormat( paragraph ) == m_cache.counterFormat )
        return m_cache.width;

    // Ensure paragraph text is valid.
    if ( m_cache.text.isNull() )
        text( paragraph );

    // Now calculate width.
    if ( m_cache.counterFormat )
        m_cache.counterFormat->removeRef();
    m_cache.counterFormat = counterFormat( paragraph );
    m_cache.counterFormat->addRef();
    m_cache.width = 0;
    if ( m_style != STYLE_NONE || m_numbering == NUM_FOOTNOTE)
    {
        QString text = m_cache.text;
        if ( m_style == STYLE_CUSTOMBULLET && !text.isEmpty() )
        {
            text.append( "  " ); // append two trailing spaces, see KoTextParag::drawLabel
        }
        else if ( !text.isEmpty() )
            text.append( ' ' ); // append a trailing space, see KoTextParag::drawLabel
        QFontMetrics fm = m_cache.counterFormat->refFontMetrics();
        for ( unsigned int i = 0; i < text.length(); i++ )
            //m_cache.width += m_cache.counterFormat->width( text, i );
            m_cache.width += fm.width( text[i] );
    }
    // Now go from 100%-zoom to LU
    m_cache.width = KoTextZoomHandler::ptToLayoutUnitPt( m_cache.width );

    //kDebug(32500) << "KoParagCounter::width recalculated parag=" << paragraph << " text='" << text << "' width=" << m_cache.width << endl;
    return m_cache.width;
}

int KoParagCounter::bulletX()
{
    // width() must have been called first
    Q_ASSERT( m_cache.width != -1 );
    Q_ASSERT( m_cache.counterFormat );
    int x = 0;
    QFontMetrics fm = m_cache.counterFormat->refFontMetrics();
    QString text = prefix();
    for (  unsigned int i = 0; i < text.length(); i++ )
        x += fm.width( text[i] );
    // Now go from 100%-zoom to LU
    return KoTextZoomHandler::ptToLayoutUnitPt( x );
}

// Only exists to centralize code. Does no caching.
KoTextFormat* KoParagCounter::counterFormat( const KoTextParag *paragraph )
{
    KoTextFormat* refFormat = paragraph->at( 0 )->format();
    KoTextFormat format( *refFormat );
    format.setVAlign( KoTextFormat::AlignNormal );
    return paragraph->textDocument()->formatCollection()->format( &format );
    /*paragraph->paragFormat()*/
}

///

const QByteArray RNUnits[] = {"", "i", "ii", "iii", "iv", "v", "vi", "vii", "viii", "ix"};
const QByteArray RNTens[] = {"", "x", "xx", "xxx", "xl", "l", "lx", "lxx", "lxxx", "xc"};
const QByteArray RNHundreds[] = {"", "c", "cc", "ccc", "cd", "d", "dc", "dcc", "dccc", "cm"};
const QByteArray RNThousands[] = {"", "m", "mm", "mmm"};

QString KoParagCounter::makeRomanNumber( int n )
{
    if ( n >= 0 )
        return QString::fromLatin1( RNThousands[ ( n / 1000 ) ] +
                                    RNHundreds[ ( n / 100 ) % 10 ] +
                                    RNTens[ ( n / 10 ) % 10 ] +
                                    RNUnits[ ( n ) % 10 ] );
    else { // should never happen, but better not crash if it does
        kWarning(32500) << "makeRomanNumber: n=" << n << endl;
        return QString::number( n );
    }
}

QString KoParagCounter::makeAlphaUpperNumber( int n )
{
    QString tmp;
    char bottomDigit;
    while ( n > 26 )
    {
        bottomDigit = (n-1) % 26;
        n = (n-1) / 26;
        tmp.prepend( QChar( 'A' + bottomDigit  ) );
    }
    tmp.prepend( QChar( 'A' + n -1 ) );
    return tmp;
}

QString KoParagCounter::makeAlphaLowerNumber( int n )
{
    QString tmp;
    char bottomDigit;
    while ( n > 26 )
    {
        bottomDigit = (n-1) % 26;
        n = (n-1) / 26;
        tmp.prepend( QChar( 'a' + bottomDigit  ) );
    }
    tmp.prepend( QChar( 'a' + n - 1 ) );
    return tmp;
}

int KoParagCounter::fromRomanNumber( const QString &string )
{
    int ret = 0;
    int stringStart = 0;
    const int stringLen = string.length();

    for (int base = 1000; base >= 1 && stringStart < stringLen; base /= 10)
    {
        const QByteArray *rn;
        int rnNum;
        switch (base)
        {
            case 1000:
                rn = RNThousands;
                rnNum = sizeof (RNThousands) / sizeof (const QByteArray);
                break;
            case 100:
                rn = RNHundreds;
                rnNum = sizeof (RNHundreds) / sizeof (const QByteArray);
                break;
            case 10:
                rn = RNTens;
                rnNum = sizeof (RNTens) / sizeof (const QByteArray);
                break;
            case 1:
            default:
                rn = RNUnits;
                rnNum = sizeof (RNUnits) / sizeof (const QByteArray);
                break;
        }

        // I _think_ this will work :) - Clarence
        for (int i = rnNum - 1; i >= 1; i--)
        {
            const int rnLength = rn[i].length();
            if (string.mid(stringStart,rnLength) == (const char*)rn[i])
            {
                ret += i * base;
                stringStart += rnLength;
                break;
            }
        }
    }

    return (ret == 0 || stringStart != stringLen) ? -1 /*invalid value*/ : ret;
}

int KoParagCounter::fromAlphaUpperNumber( const QString &string )
{
    int ret = 0;

    const int len = string.length();
    for (int i = 0; i < len; i++)
    {
        const int add = (string[i]).toLatin1() - 'A' + 1;

        if (add >= 1 && add <= 26) // _not_ < 26
            ret = ret * 26 + add;
        else
        {
            ret = -1; // invalid character
            break;
        }
    }

    return (ret == 0) ? -1 /*invalid value*/ : ret;
}

int KoParagCounter::fromAlphaLowerNumber( const QString &string )
{
    int ret = 0;

    const int len = string.length();
    for (int i = 0; i < len; i++)
    {
        const int add = (string[i]).toLatin1() - 'a' + 1;

        if (add >= 1 && add <= 26) // _not_ < 26
            ret = ret * 26 + add;
        else
        {
            ret = -1; // invalid character
            break;
        }
    }

    return (ret == 0) ? -1 /*invalid value*/ : ret;
}

#ifndef NDEBUG
void KoParagCounter::printRTDebug( KoTextParag* parag )
{
    QString additionalInfo;
    if ( restartCounter() )
        additionalInfo = "[restartCounter]";
    if ( m_style == STYLE_CUSTOMBULLET )
        additionalInfo += " [customBullet: " + QString::number( m_customBulletChar.unicode() )
                          + " in font '" + m_customBulletFont + "']";
    static const char * const s_numbering[] = { "List", "Chapter", "None", "Footnote" };
    kDebug(32500) << "  Counter style=" << style()
                   << " numbering=" << s_numbering[ numbering() ]
                   << " depth=" << depth()
                   << " number=" << number( parag )
                   << " text='" << text( parag ) << "'"
                   << " width=" << width( parag )
                   << additionalInfo << endl;
}
#endif
