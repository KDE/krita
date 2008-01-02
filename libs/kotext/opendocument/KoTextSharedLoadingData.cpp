/* This file is part of the KDE project
 * Copyright (C) 2004-2006 David Faure <faure@kde.org>
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2007 Pierre Ducroquet <pinaraf@gmail.com>
 * Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
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
        // TODO check what to do with list styles
    }

    QHash<QString, KoParagraphStyle *> paragraphStyles;
    QHash<QString, KoCharacterStyle *> characterStyles;

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

bool KoTextSharedLoadingData::loadOdfStyles( KoOasisLoadingContext & context, KoStyleManager * styleManager, bool insertOfficeStyles )
{
    // add paragraph styles
    addParagraphStyles( context, context.stylesReader().autoStyles( "paragraph" ).values(), styleManager );
    addParagraphStyles( context, context.stylesReader().autoStyles( "paragraph", true ).values(), styleManager );
    // only add styles of office:styles to the style manager
    addParagraphStyles( context, context.stylesReader().customStyles( "paragraph" ).values(), styleManager, insertOfficeStyles );

    addCharacterStyles( context, context.stylesReader().autoStyles( "text" ).values(), styleManager );
    addCharacterStyles( context, context.stylesReader().autoStyles( "text", true ).values(), styleManager );
    // only add styles of office:styles to the style manager
    addCharacterStyles( context, context.stylesReader().customStyles( "text" ).values(), styleManager, insertOfficeStyles );
}

void KoTextSharedLoadingData::addParagraphStyles( KoOasisLoadingContext & context, QList<KoXmlElement*> styleElements,
                                                  KoStyleManager * styleManager, bool insertOfficeStyles )
{
    QList<QPair<QString, KoParagraphStyle *> > paragraphStyles( loadParagraphStyles( context, styleElements ) );

    QList<QPair<QString, KoParagraphStyle *> >::iterator it( paragraphStyles.begin() );
    for ( ; it != paragraphStyles.end(); ++it )
    {
        d->paragraphStyles.insert( it->first, it->second );

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
                                                  KoStyleManager *styleManager, bool insertOfficeStyles )
{
    QList<QPair<QString, KoCharacterStyle *> > characterStyles( loadCharacterStyles( context, styleElements ) );

    QList<QPair<QString, KoCharacterStyle *> >::iterator it( characterStyles.begin() );
    for ( ; it != characterStyles.end(); ++it )
    {
        d->characterStyles.insert( it->first, it->second );

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

KoParagraphStyle * KoTextSharedLoadingData::paragraphStyle( const QString &name )
{
    return d->paragraphStyles.value( name );
}

KoCharacterStyle * KoTextSharedLoadingData::characterStyle( const QString &name )
{
    return d->characterStyles.value( name );
}

KoListStyle * KoTextSharedLoadingData::listStyle( const QString &name )
{
    return d->listStyles.value( name );
}

KoListLevelProperties KoTextSharedLoadingData::outlineLevel( int level, const KoListLevelProperties& defaultprops )
{
}
