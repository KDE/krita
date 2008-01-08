/* This file is part of the KDE project
 * Copyright (C) 2004-2006 David Faure <faure@kde.org>
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2007 Pierre Ducroquet <pinaraf@gmail.com>
 * Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoTextSharedLoadingData.h"

#include <QString>
#include <QHash>

#include <kdebug.h>

#include <KoDom.h>
#include <KoXmlNS.h>
#include <KoOdfStylesReader.h>
#include <KoOasisLoadingContext.h>

#include "styles/KoStyleManager.h"
#include "styles/KoParagraphStyle.h"
#include "styles/KoCharacterStyle.h"
#include "styles/KoListStyle.h"
#include "styles/KoListLevelProperties.h"

class KoTextSharedLoadingData::Private
{
public:
    ~Private()
    {
        qDeleteAll( paragraphStylesToDelete );
        qDeleteAll( characterStylesToDelete );
        qDeleteAll( listStyles );
    }

    // It is possible that automatic-styles in content.xml and styles.xml have the same name
    // within the same family. Therefore we have to keep them separate. The office:styles are
    // added to the autostyles so that only one lookup is needed to get the style. This is 
    // about 30% faster than having a special data structure for office:styles.
    QHash<QString, KoParagraphStyle *> paragraphContentDotXmlStyles;
    QHash<QString, KoCharacterStyle *> characterContentDotXmlStyles;
    QHash<QString, KoParagraphStyle *> paragraphStylesDotXmlStyles;
    QHash<QString, KoCharacterStyle *> characterStylesDotXmlStyles;

    QHash<QString, KoListStyle *> listStyles;
    KoListStyle outlineStyles;

    QList<KoParagraphStyle *> paragraphStylesToDelete;
    QList<KoCharacterStyle *> characterStylesToDelete;
};

KoTextSharedLoadingData::KoTextSharedLoadingData()
: d( new Private() )
{
}

KoTextSharedLoadingData::~KoTextSharedLoadingData()
{
    delete d;
}

void KoTextSharedLoadingData::loadOdfStyles( KoOasisLoadingContext & context, KoStyleManager * styleManager, bool insertOfficeStyles )
{
    // add paragraph styles
    addParagraphStyles( context, context.stylesReader().autoStyles( "paragraph" ).values(), ContextDotXml, styleManager );
    addParagraphStyles( context, context.stylesReader().autoStyles( "paragraph", true ).values(), StylesDotXml, styleManager );
    // only add styles of office:styles to the style manager
    addParagraphStyles( context, context.stylesReader().customStyles( "paragraph" ).values(), ContextDotXml | StylesDotXml, styleManager, insertOfficeStyles );

    addCharacterStyles( context, context.stylesReader().autoStyles( "text" ).values(), ContextDotXml, styleManager );
    addCharacterStyles( context, context.stylesReader().autoStyles( "text", true ).values(), StylesDotXml, styleManager );
    // only add styles of office:styles to the style manager
    addCharacterStyles( context, context.stylesReader().customStyles( "text" ).values(), ContextDotXml | StylesDotXml, styleManager, insertOfficeStyles );

    addListStyles( context );
    addOutlineStyles( context );

    kDebug(32500) << "content.xml: paragraph styles" << d->paragraphContentDotXmlStyles.count() << "character styles" << d->characterContentDotXmlStyles.count();
    kDebug(32500) << "styles.xml:  paragraph styles" << d->paragraphStylesDotXmlStyles.count() << "character styles" << d->characterStylesDotXmlStyles.count();
}

void KoTextSharedLoadingData::addParagraphStyles( KoOasisLoadingContext & context, QList<KoXmlElement*> styleElements,
                                                  int styleTypes, KoStyleManager * styleManager, bool insertOfficeStyles )
{
    QList<QPair<QString, KoParagraphStyle *> > paragraphStyles( loadParagraphStyles( context, styleElements ) );

    QList<QPair<QString, KoParagraphStyle *> >::iterator it( paragraphStyles.begin() );
    for ( ; it != paragraphStyles.end(); ++it )
    {
        if ( styleTypes & ContextDotXml ) {
            d->paragraphContentDotXmlStyles.insert( it->first, it->second );
        }
        if ( styleTypes & StylesDotXml ) {
            d->paragraphStylesDotXmlStyles.insert( it->first, it->second );
        }

        // TODO check if it a know style set the styleid so that the custom styles are kept during copy and paste
        // in case styles are not added to the style manager they have to be deleted after loading to avoid leaking memeory
        if ( styleManager && insertOfficeStyles ) {
            styleManager->add( it->second );
        }
        else {
            d->paragraphStylesToDelete.append( it->second );
        }
    }
}

QList<QPair<QString, KoParagraphStyle *> > KoTextSharedLoadingData::loadParagraphStyles( KoOasisLoadingContext & context, QList<KoXmlElement*> styleElements )
{
    QList<QPair<QString, KoParagraphStyle *> > paragraphStyles;

    foreach( KoXmlElement* styleElem, styleElements ) {
        Q_ASSERT( styleElem );
        Q_ASSERT( !styleElem->isNull() );

        //1.6: KoParagStyle::loadStyle
        QString name = styleElem->attributeNS( KoXmlNS::style, "name", QString() );
        QString displayName = styleElem->attributeNS( KoXmlNS::style, "display-name", QString() );
        if ( displayName.isEmpty() ) {
            displayName = name;
        }

        kDebug(32500) << "styleName =" << name << "styleDisplayName =" << displayName;

#if 0 //1.6:
        // OOo hack:
        //m_bOutline = name.startsWith( "Heading" );
        // real OASIS solution:
        bool m_bOutline = styleElem->hasAttributeNS( KoXmlNS::style, "default-outline-level" );
#endif
        context.styleStack().save();
        context.addStyles( styleElem, "paragraph" ); // Load all parents - only because we don't support inheritance.

        KoParagraphStyle *parastyle = new KoParagraphStyle();
        parastyle->setName( displayName );
        //parastyle->setParent( d->stylemanager->defaultParagraphStyle() );

        //1.6: KoTextParag::loadOasis => KoParagLayout::loadOasisParagLayout
        context.styleStack().setTypeProperties( "paragraph" ); // load all style attributes from "style:paragraph-properties"
        parastyle->loadOasis( context.styleStack() ); // load the KoParagraphStyle from the stylestack

        //1.6: KoTextFormat::load
        KoCharacterStyle *charstyle = parastyle->characterStyle();
        context.styleStack().setTypeProperties( "text" ); // load all style attributes from "style:text-properties"
        charstyle->loadOasis( context ); // load the KoCharacterStyle from the stylestack

        context.styleStack().restore();

        paragraphStyles.append( QPair<QString, KoParagraphStyle *>( name, parastyle ) );
    }
    return paragraphStyles;
}

void KoTextSharedLoadingData::addCharacterStyles( KoOasisLoadingContext & context, QList<KoXmlElement*> styleElements,
                                                  int styleTypes, KoStyleManager *styleManager, bool insertOfficeStyles )
{
    QList<QPair<QString, KoCharacterStyle *> > characterStyles( loadCharacterStyles( context, styleElements ) );

    QList<QPair<QString, KoCharacterStyle *> >::iterator it( characterStyles.begin() );
    for ( ; it != characterStyles.end(); ++it )
    {
        if ( styleTypes & ContextDotXml ) {
            d->characterContentDotXmlStyles.insert( it->first, it->second );
        }
        if ( styleTypes & StylesDotXml ) {
            d->characterStylesDotXmlStyles.insert( it->first, it->second );
        }

        // TODO check if it a know style set the styleid so that the custom styles are kept during copy and paste
        // in case styles are not added to the style manager they have to be deleted after loading to avoid leaking memeory
        if ( styleManager && insertOfficeStyles ) {
            styleManager->add( it->second );
        }
        else {
            d->characterStylesToDelete.append( it->second );
        }
    }
}

QList<QPair<QString, KoCharacterStyle *> > KoTextSharedLoadingData::loadCharacterStyles( KoOasisLoadingContext & context, QList<KoXmlElement*> styleElements )
{
    QList<QPair<QString, KoCharacterStyle *> > characterStyles;

    foreach( KoXmlElement* styleElem, styleElements ) {
        Q_ASSERT( styleElem );
        Q_ASSERT( !styleElem->isNull() );

        QString name = styleElem->attributeNS( KoXmlNS::style, "name", QString() );
        QString displayName = styleElem->attributeNS( KoXmlNS::style, "display-name", QString() );
        if ( displayName.isEmpty() ) {
            displayName = name;
        }

        kDebug(32500) << "styleName =" << name << "styleDisplayName =" << displayName;

        context.styleStack().save();
        context.addStyles( styleElem, "text" ); // Load all parents - only because we don't support inheritance.

        context.styleStack().setTypeProperties( "text" );

        KoCharacterStyle *characterStyle = new KoCharacterStyle();
        characterStyle->setName( displayName );
        characterStyle->loadOasis( context );

        context.styleStack().restore();

        characterStyles.append( QPair<QString, KoCharacterStyle *>( name, characterStyle ) );
    }
    return characterStyles;
}

void KoTextSharedLoadingData::addListStyles( KoOasisLoadingContext & context )
{
    QHash<QString, KoXmlElement *> listStyles = context.stylesReader().listStyles();
    QHash<QString, KoXmlElement*>::iterator it( listStyles.begin() );
    for ( ; it != listStyles.end(); ++it )
    {
        kDebug(32500) << "listStyle =" << it.key();
        KoListStyle *listStyle = new KoListStyle();
        listStyle->setName( it.key() );
        listStyle->loadOasis( context, *it.value() );
        d->listStyles.insert( it.key(), listStyle);
    }
}

void KoTextSharedLoadingData::addOutlineStyles( KoOasisLoadingContext & context )
{
    // outline-styles used e.g. for headers
    KoXmlElement outlineStyle = KoDom::namedItemNS( context.stylesReader().officeStyle(), KoXmlNS::text, "outline-style" );
    KoXmlElement tag;
    forEachElement( tag, outlineStyle )
    {
        kDebug(32500) << "outline-listStyle =" << tag.localName();
        KoListLevelProperties properties;
        properties.loadOasis( context, tag );
        d->outlineStyles.setLevel( properties );
    }
}

KoParagraphStyle * KoTextSharedLoadingData::paragraphStyle( const QString &name, bool stylesDotXml )
{
    return stylesDotXml ? d->paragraphStylesDotXmlStyles.value( name ) : d->paragraphContentDotXmlStyles.value( name );
}

KoCharacterStyle * KoTextSharedLoadingData::characterStyle( const QString &name, bool stylesDotXml )
{
    return stylesDotXml ? d->characterStylesDotXmlStyles.value( name ) : d->characterContentDotXmlStyles.value( name );
}

KoListStyle * KoTextSharedLoadingData::listStyle( const QString &name )
{
    return d->listStyles.value( name );
}

KoListLevelProperties KoTextSharedLoadingData::outlineLevel( int level, const KoListLevelProperties& defaultprops )
{
    return d->outlineStyles.hasPropertiesForLevel( level ) ? d->outlineStyles.level( level ) : defaultprops;
}
