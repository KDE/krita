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

#include "KoParagStyle.h"
#include "KoOasisContext.h"
#include "KoParagCounter.h"

#include <KoGenStyles.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>

#include <kdebug.h>
#include <klocale.h>


KoCharStyle::KoCharStyle( const QString & name )
    : KoUserStyle( name )
{
}

const KoTextFormat & KoCharStyle::format() const
{
    return m_format;
}

KoTextFormat & KoCharStyle::format()
{
    return m_format;
}

///////////////////////////

KoParagStyle::KoParagStyle( const QString & name )
    : KoCharStyle( name )
{
    m_followingStyle = this;

    // This way, KoTextParag::setParagLayout also sets the style pointer, to this style
    m_paragLayout.style = this;
    m_parentStyle = 0L;
    m_inheritedParagLayoutFlag = 0;
    m_inheritedFormatFlag = 0;
    m_bOutline = false;
}

KoParagStyle::KoParagStyle( const KoParagStyle & rhs )
    : KoCharStyle( rhs)
{
    *this = rhs;
}

KoParagStyle::~KoParagStyle()
{
}

void KoParagStyle::operator=( const KoParagStyle &rhs )
{
    KoCharStyle::operator=( rhs );
    m_paragLayout = rhs.m_paragLayout;
    m_followingStyle = rhs.m_followingStyle;
    m_paragLayout.style = this; // must always be "this"
    m_parentStyle = rhs.m_parentStyle;
    m_inheritedParagLayoutFlag = rhs.m_inheritedParagLayoutFlag;
    m_inheritedFormatFlag = rhs.m_inheritedFormatFlag;
    m_bOutline = rhs.m_bOutline;
}

void KoParagStyle::setFollowingStyle( KoParagStyle *fst )
{
  m_followingStyle = fst;
}

void KoParagStyle::saveStyle( QDomElement & parentElem )
{
    m_paragLayout.saveParagLayout( parentElem, m_paragLayout.alignment );

    if ( followingStyle() )
    {
        QDomElement element = parentElem.ownerDocument().createElement( "FOLLOWING" );
        parentElem.appendChild( element );
        element.setAttribute( "name", followingStyle()->displayName() );
    }
    // TODO save parent style, and inherited flags.

    parentElem.setAttribute( "outline", m_bOutline ? "true" : "false" );
}

void KoParagStyle::loadStyle( QDomElement & parentElem, int docVersion )
{
    KoParagLayout layout;
    KoParagLayout::loadParagLayout( layout, parentElem, docVersion );

    // This way, KoTextParag::setParagLayout also sets the style pointer, to this style
    layout.style = this;
    m_paragLayout = layout;

    // Load name
    QDomElement nameElem = parentElem.namedItem("NAME").toElement();
    if ( !nameElem.isNull() ) {
        m_name = nameElem.attribute("value");
        m_displayName = i18n( "Style name", m_name );
    } else
        kWarning() << "No NAME tag in LAYOUT -> no name for this style!" << endl;

    // The followingStyle stuff has to be done after loading all styles.

    m_bOutline = parentElem.attribute( "outline" ) == "true";
}

void KoParagStyle::loadStyle( QDomElement & styleElem, KoOasisContext& context )
{
    // Load name
    m_name = styleElem.attributeNS( KoXmlNS::style, "name", QString::null );
    m_displayName = styleElem.attributeNS( KoXmlNS::style, "display-name", QString::null );
    if ( m_displayName.isEmpty() )
        m_displayName = m_name;

    // OOo hack
    //m_bOutline = m_name.startsWith( "Heading" );
    // real OASIS solution:
    m_bOutline = styleElem.hasAttributeNS( KoXmlNS::style, "default-outline-level" );

    context.styleStack().save();
    context.addStyles( &styleElem, "paragraph" ); // Load all parents - only because we don't support inheritance.
    KoParagLayout layout;
    KoParagLayout::loadOasisParagLayout( layout, context );

    // loadOasisParagLayout doesn't load the counter. It's modelled differently for parags and for styles.
    int level = 0;
    bool listOK = false;
    const QString listStyleName = styleElem.attributeNS( KoXmlNS::style, "list-style-name", QString::null );
    if ( m_bOutline ) {
        level = styleElem.attributeNS( KoXmlNS::style, "default-outline-level", QString::null ).toInt(); // 1-based
        listOK = context.pushOutlineListLevelStyle( level );
        // allow overriding the outline numbering, see http://lists.oasis-open.org/archives/office/200310/msg00033.html
        if ( !listStyleName.isEmpty() )
            context.pushListLevelStyle( listStyleName, level );
    }
    else {
        // ######## BIG difference here. In the OOo/OASIS format, one list style has infos for 10 list levels...
        // ###### so we can't know a level at this point...

        // The only solution I can think of, to preserve document content when importing OO but
        // not necessarily the styles used when editing, is:
        // 1) when importing from OOo, convert each non-heading style with numbering
        // into 10 kotext styles (at least those used by the document) [TODO]
        // 2) for KWord's own loading/saving, to add a hack into the file format, say
        // style:default-level.
        // Note that default-level defaults to "1", i.e. works for non-nested OOo lists too.
        level = styleElem.attributeNS( KoXmlNS::style, "default-level", "1" ).toInt(); // 1-based
        listOK = !listStyleName.isEmpty();
        if ( listOK )
            listOK = context.pushListLevelStyle( listStyleName, level );
    }
    if ( listOK ) {
        const QDomElement listStyle = context.listStyleStack().currentListStyle();
        // The tag is either text:list-level-style-number or text:list-level-style-bullet
        const bool ordered = listStyle.localName() == "list-level-style-number";
        Q_ASSERT( !layout.counter );
        layout.counter = new KoParagCounter;
        layout.counter->loadOasis( context, -1, ordered, m_bOutline, level, true );
        context.listStyleStack().pop();
    }

    // This way, KoTextParag::setParagLayout also sets the style pointer, to this style
    layout.style = this;
    m_paragLayout = layout;

    m_format.load( context );

    context.styleStack().restore();
}

QString KoParagStyle::saveStyle( KoGenStyles& genStyles, int styleType, const QString& parentStyleName, KoSavingContext& context ) const
{
    KoGenStyle gs( styleType, "paragraph", parentStyleName );

    gs.addAttribute( "style:display-name", m_displayName );
    if ( m_paragLayout.counter ) {
        if ( m_bOutline )
            gs.addAttribute( "style:default-outline-level", (int)m_paragLayout.counter->depth() + 1 );
        else if ( m_paragLayout.counter->depth() )
            // ### kword-specific attribute, see loadOasis
            gs.addAttribute( "style:default-level", (int)m_paragLayout.counter->depth() + 1 );

        if ( m_paragLayout.counter->numbering() != KoParagCounter::NUM_NONE &&
             m_paragLayout.counter->style() != KoParagCounter::STYLE_NONE )
        {
            KoGenStyle listStyle( KoGenStyle::STYLE_LIST /*, no family*/ );
            m_paragLayout.counter->saveOasis( listStyle, true );
            // This display-name will probably look nicer in OO, but this also means
            // no re-use possible between list styles...
            listStyle.addAttribute( "style:display-name",
                                    i18n( "Numbering Style for %1" , m_displayName ) );

            QString autoListStyleName = genStyles.lookup( listStyle, "L", KoGenStyles::ForceNumbering );
            gs.addAttribute( "style:list-style-name", autoListStyleName );
        }
    }

    m_paragLayout.saveOasis( gs, context, true );

    m_format.save( gs, context );

    // try to preserve existing internal name, if it looks adequate (no spaces)
    // ## TODO: check XML-Schemacs NCName conformity
    bool nameIsConform = !m_name.isEmpty() && !m_name.contains( ' ' );
    QString newName;
    if ( nameIsConform )
        newName = genStyles.lookup( gs, m_name, KoGenStyles::DontForceNumbering );
    else
        newName = genStyles.lookup( gs, "U", KoGenStyles::ForceNumbering );
    const_cast<KoParagStyle*>( this )->m_name = newName;
    return m_name;
}

const KoParagLayout & KoParagStyle::paragLayout() const
{
    return m_paragLayout;
}

KoParagLayout & KoParagStyle::paragLayout()
{
    return m_paragLayout;
}

void KoParagStyle::propagateChanges( int paragLayoutFlag, int /*formatFlag*/ )
{
    if ( !m_parentStyle )
        return;
    if ( !(paragLayoutFlag & KoParagLayout::Alignment) )
        m_paragLayout.alignment = m_parentStyle->paragLayout().alignment;
    if ( !(paragLayoutFlag & KoParagLayout::Margins) )
        for ( int i = 0 ; i < 5 ; ++i )
            m_paragLayout.margins[i] = m_parentStyle->paragLayout().margins[i];
    if ( !(paragLayoutFlag & KoParagLayout::LineSpacing) )
    {
        m_paragLayout.setLineSpacingValue(m_parentStyle->paragLayout().lineSpacingValue());
        m_paragLayout.lineSpacingType = m_parentStyle->paragLayout().lineSpacingType;
    }
    if ( !(paragLayoutFlag & KoParagLayout::Borders) )
    {
        m_paragLayout.leftBorder = m_parentStyle->paragLayout().leftBorder;
        m_paragLayout.rightBorder = m_parentStyle->paragLayout().rightBorder;
        m_paragLayout.topBorder = m_parentStyle->paragLayout().topBorder;
        m_paragLayout.bottomBorder = m_parentStyle->paragLayout().bottomBorder;
        m_paragLayout.joinBorder = m_parentStyle->paragLayout().joinBorder;
    }
    if ( !(paragLayoutFlag & KoParagLayout::BulletNumber) )
        m_paragLayout.counter = m_parentStyle->paragLayout().counter;
    if ( !(paragLayoutFlag & KoParagLayout::Tabulator) )
        m_paragLayout.setTabList(m_parentStyle->paragLayout().tabList());
#if 0
    if ( paragLayoutFlag == KoParagLayout::All )
    {
        setDirection( static_cast<QChar::Direction>(layout.direction) );
        // Don't call applyStyle from here, it would overwrite any paragraph-specific settings
        setStyle( layout.style );
    }
#endif
    // TODO a flag for the "is outline" bool? Otherwise we have no way to inherit
    // that property (and possibly reset it).
}

void KoParagStyle::setOutline( bool b )
{
    m_bOutline = b;
}
