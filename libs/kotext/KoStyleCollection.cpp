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

#include "KoStyleCollection.h"
#include "KoOasisContext.h"
#include "KoParagCounter.h"

#include <KoOasisStyles.h>
#include <KoGenStyles.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>

#include <kdebug.h>
#include <klocale.h>

#include <QList>

KoStyleCollection::KoStyleCollection()
    : KoUserStyleCollection( "paragsty" )
{
}

KoStyleCollection::~KoStyleCollection()
{
}

int KoStyleCollection::loadOasisStyles( KoOasisContext& context )
{
    QStringList followingStyles;
    Q3ValueVector<QDomElement> userStyles = context.oasisStyles().userStyles();
    bool defaultStyleDeleted = false;
    int stylesLoaded = 0;
    const unsigned int nStyles = userStyles.count();
    for (unsigned int item = 0; item < nStyles; item++) {
        QDomElement styleElem = userStyles[item];
	Q_ASSERT( !styleElem.isNull() );

        if ( styleElem.attributeNS( KoXmlNS::style, "family", QString::null ) != "paragraph" )
            continue;

        if( !defaultStyleDeleted ) { // we are going to import at least one style.
            KoParagStyle *s = defaultStyle();
            //kDebug() << "loadOasisStyles looking for Standard, to delete it. Found " << s << endl;
            if(s) // delete the standard style.
                removeStyle(s);
            defaultStyleDeleted = true;
        }

        KoParagStyle *sty = new KoParagStyle( QString::null );
        // Load the style
        sty->loadStyle( styleElem, context );
        // Style created, now let's try to add it
        const int oldStyleCount = count();
        sty = addStyle( sty );
        // the real value of followingStyle is set below after loading all styles
        sty->setFollowingStyle( sty );

        kDebug() << " Loaded style " << sty->name() << endl;

        if ( count() > oldStyleCount )
        {
            const QString following = styleElem.attributeNS( KoXmlNS::style, "next-style-name", QString::null );
            followingStyles.append( following );
            ++stylesLoaded;
        }
        else
            kWarning() << "Found duplicate style declaration, overwriting former " << sty->name() << endl;
    }

    if( followingStyles.count() != styleList().count() ) {
        kDebug() << "Ouch, " << followingStyles.count() << " following-styles, but "
                       << styleList().count() << " styles in styleList" << endl;
    }

    unsigned int i = 0;
    QString tmpString;
    foreach( tmpString, followingStyles )
    {
        const QString followingStyleName = tmpString;
	if ( !followingStyleName.isEmpty() ) {
            KoParagStyle * style = findStyle( followingStyleName );
	    if ( style )
                styleAt(i)->setFollowingStyle( style );
	}
    }

    // TODO the same thing for style inheritance (style:parent-style-name) and setParentStyle()

    Q_ASSERT( defaultStyle() );
    return stylesLoaded;
}

void KoStyleCollection::saveOasis( KoGenStyles& styles, int styleType, KoSavingContext& context ) const
{
    // In order to reduce the bloat, we define that the first style (usually Standard)
    // is the "parent" (reference) for the others.
    // ## This is mostly a hack due to lack of proper style inheritance.
    // Once that's implemented, default to 'styles derive from Standard', but save normally.
    QString refStyleName;
    
    KoUserStyle* tmpStyle = 0;
    foreach( tmpStyle, m_styleList )
    {
        KoParagStyle* style = static_cast<KoParagStyle *>( tmpStyle );
        style->saveStyle( styles, styleType, refStyleName, context );
        kDebug() << k_funcinfo << "Saved style " << style->displayName() << " to OASIS format as " << style->name() << endl;
        if ( refStyleName.isEmpty() ) // i.e. first style
            refStyleName = style->name();
    }
    tmpStyle = 0;
    // Now edit the kogenstyle and set the next-style-name. This works here
    // because the style's m_name is already unique so there's no risk of
    // "two styles being only different due to their following-style"; the
    // display-name will also be different, and will ensure they get two kogenstyles.
    foreach( tmpStyle, m_styleList )
    {
        KoParagStyle* style = static_cast<KoParagStyle *>( tmpStyle );
        if ( style->followingStyle() && style->followingStyle() != style ) {
            const QString fsname = style->followingStyle()->name();
            KoGenStyle* gs = styles.styleForModification( style->name() );
            Q_ASSERT( gs );
            if ( gs )
                gs->addAttribute( "style:next-style-name", fsname );
        }
    }
}

void KoStyleCollection::importStyles( const KoStyleCollection& styleCollection )
{
    const Q3ValueList<KoUserStyle *> styles = styleCollection.styleList();
    QMap<QString, QString> followStyle;
    for ( Q3ValueList<KoUserStyle *>::const_iterator styleIt = styles.begin(), styleEnd = styles.end() ; styleIt != styleEnd ; ++styleIt ) {
        KoParagStyle* p = static_cast<KoParagStyle *>( *styleIt );
        KoParagStyle* style = new KoParagStyle( *p );
        if ( style->followingStyle() ) {
            followStyle.insert( style->name(), style->followingStyle()->name() );
        }
        style = addStyle( style );
    }

    QMap<QString, QString>::iterator itFollow = followStyle.begin();
    for ( ; itFollow != followStyle.end(); ++itFollow )
    {
        KoParagStyle * style = findStyle(itFollow.key());
        const QString followingStyleName = followStyle[ itFollow.key() ];
        KoParagStyle * styleFollow = findStyle(followingStyleName);
        //kDebug() << "    " << style << "  " << itFollow.key() << ": followed by " << styleFollow << " (" << followingStyleName << ")" << endl;
        Q_ASSERT(styleFollow);
        if ( styleFollow )
            style->setFollowingStyle( styleFollow );
        else
            style->setFollowingStyle( style );
    }
}

void KoStyleCollection::saveOasisOutlineStyles( KoXmlWriter& writer ) const
{
    bool first = true;
    Q3ValueVector<KoParagStyle *> styles = outlineStyles();
    for ( int i = 0 ; i < 10 ; ++i ) {
        if ( styles[i] ) {
            if ( first ) {
                writer.startElement( "text:outline-style" );
                first = false;
            }
            writer.startElement( "text:outline-level-style" );
            styles[i]->paragLayout().counter->saveOasisListLevel( writer, true, true );
            writer.endElement();
        }
    }
    if ( !first )
        writer.endElement(); // text:outline-style
}

Q3ValueVector<KoParagStyle *> KoStyleCollection::outlineStyles() const
{
    Q3ValueVector<KoParagStyle *> lst( 10, 0 );
    for ( int i = 0 ; i < 10 ; ++i ) {
        KoParagStyle* style = outlineStyleForLevel( i );
        if ( style )
            lst[i] = style;
    }
    return lst;
}


KoParagStyle* KoStyleCollection::outlineStyleForLevel( int level ) const
{
    KoUserStyle* tmpStyle = 0;	
    foreach( tmpStyle, m_styleList )
    {
        KoParagStyle* style = static_cast<KoParagStyle *>( tmpStyle );
        if ( style->isOutline() && style->paragLayout().counter )
        {
            int styleLevel = style->paragLayout().counter->depth();
            if ( styleLevel == level )
                return style;
        }
    }
    return 0;
}

KoParagStyle* KoStyleCollection::numberedStyleForLevel( int level ) const
{
    KoUserStyle* tmpStyle = 0;	
    foreach( tmpStyle, m_styleList )
    {
        KoParagStyle* style = static_cast<KoParagStyle *>( tmpStyle );
        KoParagCounter* counter = style->paragLayout().counter;
        if ( !style->isOutline() && counter
             && counter->numbering() != KoParagCounter::NUM_NONE
             && !counter->isBullet() )
        {
            int styleLevel = counter->depth();
            if ( styleLevel == level )
                return style;
        }
    }
    return 0;
}

KoParagStyle* KoStyleCollection::defaultStyle() const
{
    return findStyle( "Standard" ); // includes the fallback to first style
}

#ifndef NDEBUG
void KoStyleCollection::printDebug() const
{
    KoParagStyle* style = 0;	
    KoUserStyle* tmpStyle = 0;	
    foreach( tmpStyle, m_styleList )
    {
	style = static_cast<KoParagStyle *>( tmpStyle );

        // short version:
        // kDebug() << style << "  " << style->name() << "    " << style->displayName() << "  followingStyle=" << style->followingStyle() << endl;

        kDebug() << "Style " << style << "  " << style->name() << "  isOutline=" << style->isOutline() << endl;
        kDebug() << "   format: " << style->format().key() <<endl;
        static const char * const s_align[] = { "Auto", "Left", "Right", "ERROR", "HCenter", "ERR", "ERR", "ERR", "Justify", };
        kDebug() << "  align: " << s_align[(Qt::Alignment)style->paragLayout().alignment] << endl;
        if ( style->paragLayout().counter )
            kDebug() << "  counter level=" << style->paragLayout().counter->depth() << endl;

        kDebug() << "   following style: " << style->followingStyle() << " "
                  << ( style->followingStyle() ? style->followingStyle()->name() : QString::null ) << endl;

    }
}
#endif
