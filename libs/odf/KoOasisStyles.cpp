/* This file is part of the KDE project
   Copyright (C) 2004-2006 David Faure <faure@kde.org>
   Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>

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
 * Boston, MA 02110-1301, USA.
*/

#include "KoOasisStyles.h"

#include "KoDom.h"
#include "KoGenStyles.h"
#include "KoXmlNS.h"
#include "KoUnit.h"

#include <QtCore/QBuffer>

#include <kdebug.h>
#include <kglobal.h>

#include <KoXmlReader.h>

class KoOasisStyles::Private
{
public:
    QHash<QString /*family*/, QHash<QString /*name*/, KoXmlElement*> > customStyles;
    // auto-styles in content.xml
    QHash<QString /*family*/, QHash<QString /*name*/, KoXmlElement*> > contentAutoStyles;
    // auto-styles in styles.xml
    QHash<QString /*family*/, QHash<QString /*name*/, KoXmlElement*> > stylesAutoStyles;
    QHash<QString /*family*/, KoXmlElement*> defaultStyles;

    QHash<QString /*name*/, KoXmlElement*> styles; // page-layout, font-face etc.
    QHash<QString /*name*/, KoXmlElement*> masterPages;
    QHash<QString /*name*/, KoXmlElement*> listStyles;
    QHash<QString /*name*/, KoXmlElement*> drawStyles;

    KoXmlElement           officeStyle;
    KoXmlElement           layerSet;

    DataFormatsMap         dataFormats;
};

KoOasisStyles::KoOasisStyles()
    : d( new Private )
{
}

KoOasisStyles::~KoOasisStyles()
{
    foreach ( const QString& family, d->customStyles.keys() )
        qDeleteAll( d->customStyles[family] );
    foreach ( const QString& family, d->contentAutoStyles.keys() )
        qDeleteAll( d->contentAutoStyles[family] );
    foreach ( const QString& family, d->stylesAutoStyles.keys() )
        qDeleteAll( d->stylesAutoStyles[family] );
    qDeleteAll( d->defaultStyles );
    qDeleteAll( d->styles );
    qDeleteAll( d->masterPages );
    qDeleteAll( d->listStyles );
    qDeleteAll( d->drawStyles );
    delete d;
}

void KoOasisStyles::createStyleMap( const KoXmlDocument& doc, bool stylesDotXml )
{
   const KoXmlElement docElement  = doc.documentElement();
    // We used to have the office:version check here, but better let the apps do that
    KoXmlElement fontStyles = KoDom::namedItemNS( docElement, KoXmlNS::office, "font-face-decls" );

    if ( !fontStyles.isNull() ) {
        //kDebug(30003) <<"Starting reading in font-face-decls...";
        insertStyles( fontStyles, stylesDotXml ? AutomaticInStyles : AutomaticInContent );
    }// else
    //   kDebug(30003) <<"No items found";

    //kDebug(30003) <<"Starting reading in office:automatic-styles. stylesDotXml=" << stylesDotXml;

    KoXmlElement autoStyles = KoDom::namedItemNS( docElement, KoXmlNS::office, "automatic-styles" );
    if ( !autoStyles.isNull() ) {
        insertStyles( autoStyles, stylesDotXml ? AutomaticInStyles : AutomaticInContent );
    }// else
    //    kDebug(30003) <<"No items found";


    //kDebug(30003) <<"Reading in master styles";

    KoXmlNode masterStyles = KoDom::namedItemNS( docElement, KoXmlNS::office, "master-styles" );
    if ( !masterStyles.isNull() ) {
        KoXmlElement master;
        forEachElement( master, masterStyles )
        {
            if ( master.localName() == "master-page" &&
                 master.namespaceURI() == KoXmlNS::style ) {
                const QString name = master.attributeNS( KoXmlNS::style, "name", QString() );
                kDebug(30003) <<"Master style: '" << name <<"' loaded";
                d->masterPages.insert( name, new KoXmlElement( master ) );
            } else if( master.localName() == "layer-set" && master.namespaceURI() == KoXmlNS::draw ) {
                kDebug(30003) <<"Master style: layer-set loaded";
                d->layerSet = master;
            } else
                // OASIS docu mentions style:handout-master and draw:layer-set here
                kWarning(30003) << "Unknown tag " << master.tagName() << " in office:master-styles";
        }
    }


    kDebug(30003) <<"Starting reading in office:styles";

    const KoXmlElement officeStyle = KoDom::namedItemNS( docElement, KoXmlNS::office, "styles" );
    if ( !officeStyle.isNull() ) {
        d->officeStyle = officeStyle;
        insertOfficeStyles( officeStyle );
    }

    //kDebug(30003) <<"Styles read in.";
}

QHash<QString, KoXmlElement*> KoOasisStyles::customStyles(const QString& family) const
{
    if ( family.isNull() )
        return QHash<QString, KoXmlElement*>();
    return d->customStyles.value( family );
}

QHash<QString, KoXmlElement*> KoOasisStyles::autoStyles(const QString& family, bool stylesDotXml ) const
{
    if ( family.isNull() )
        return QHash<QString, KoXmlElement*>();
    return stylesDotXml ? d->stylesAutoStyles.value( family ) : d->contentAutoStyles.value( family );
}

const KoOasisStyles::DataFormatsMap& KoOasisStyles::dataFormats() const
{
    return d->dataFormats;
}

void KoOasisStyles::insertOfficeStyles( const KoXmlElement& styles )
{
    KoXmlElement e;
    forEachElement( e, styles )
    {
        const QString localName = e.localName();
        const QString ns = e.namespaceURI();
        if ( ( ns == KoXmlNS::svg && (
                   localName == "linearGradient"
                   || localName == "radialGradient" ) )
             || ( ns == KoXmlNS::draw && (
                      localName == "gradient"
                      || localName == "hatch"
                      || localName == "fill-image"
                      || localName == "marker"
                      || localName == "stroke-dash"
                      || localName == "opacity" ) )
             )
        {
            const QString name = e.attributeNS( KoXmlNS::draw, "name", QString() );
            Q_ASSERT( !name.isEmpty() );
            KoXmlElement* ep = new KoXmlElement( e );
            d->drawStyles.insert( name, ep );
        }
        else
            insertStyle( e, CustomInStyles );
    }
}


void KoOasisStyles::insertStyles( const KoXmlElement& styles, TypeAndLocation typeAndLocation )
{
    //kDebug(30003) <<"Inserting styles from" << styles.tagName();
    KoXmlElement e;
    forEachElement( e, styles )
        insertStyle( e, typeAndLocation );
}

void KoOasisStyles::insertStyle( const KoXmlElement& e, TypeAndLocation typeAndLocation )
{
    const QString localName = e.localName();
    const QString ns = e.namespaceURI();

    const QString name = e.attributeNS( KoXmlNS::style, "name", QString() );
    if ( ns == KoXmlNS::style && localName == "style" ) {
        const QString family = e.attributeNS( KoXmlNS::style, "family", QString() );

        if ( typeAndLocation == AutomaticInContent ) {
            QHash<QString, KoXmlElement*>& dict = d->contentAutoStyles[ family ];
            if ( dict.contains( name ) )
            {
                kDebug(30003) <<"Auto-style: '" << name <<"' already exists";
                delete dict.take( name );
            }
            dict.insert( name, new KoXmlElement( e ) );
            //kDebug(30003) <<"Style: '" << name <<"' loaded as a style auto style";
        } else if ( typeAndLocation == AutomaticInStyles ) {
            QHash<QString, KoXmlElement*>& dict = d->stylesAutoStyles[ family ];
            if ( dict.contains( name ) )
            {
                kDebug(30003) <<"Auto-style: '" << name <<"' already exists";
                delete dict.take( name );
            }
            dict.insert( name, new KoXmlElement( e ) );
            //kDebug(30003) <<"Style: '" << name <<"' loaded as a style auto style";
        } else {
            QHash<QString, KoXmlElement*>& dict = d->customStyles[ family ];
            if ( dict.contains( name ) )
            {
                kDebug(30003) <<"Style: '" << name <<"' already exists";
                delete dict.take( name );
            }
            dict.insert( name, new KoXmlElement( e ) );
            //kDebug(30003) <<"Style: '" << name <<"' loaded";
        }
    } else if ( ns == KoXmlNS::style && (
                localName == "page-layout"
             || localName == "font-face"
             || localName == "presentation-page-layout" ) )
    {
        if ( d->styles.contains( name ) )
        {
            kDebug(30003) <<"Style: '" << name <<"' already exists";
            delete d->styles.take( name );
        }
        d->styles.insert( name, new KoXmlElement( e ) );
    } else if ( localName == "default-style" && ns == KoXmlNS::style ) {
        const QString family = e.attributeNS( KoXmlNS::style, "family", QString() );
        if ( !family.isEmpty() )
            d->defaultStyles.insert( family, new KoXmlElement( e ) );
    } else if ( localName == "list-style" && ns == KoXmlNS::text ) {
        d->listStyles.insert( name, new KoXmlElement( e ) );
        //kDebug(30003) <<"List style: '" << name <<"' loaded";
    } else if ( ns == KoXmlNS::number && (
                   localName == "number-style"
                || localName == "currency-style"
                || localName == "percentage-style"
                || localName == "boolean-style"
                || localName == "text-style"
                || localName == "date-style"
                || localName == "time-style" ) ) {
        QPair<QString, KoOdfNumberStyles::NumericStyleFormat> numberStyle = KoOdfNumberStyles::loadOdfNumberStyle( e );
        d->dataFormats.insert( numberStyle.first, numberStyle.second );
    }
    // The rest (text:*-configuration and text:outline-style) is to be done by the apps.
}

const KoXmlElement* KoOasisStyles::defaultStyle( const QString& family ) const
{
    return d->defaultStyles[family];
}

const KoXmlElement& KoOasisStyles::officeStyle() const
{
    return d->officeStyle;
}

const KoXmlElement& KoOasisStyles::layerSet() const
{
    return d->layerSet;
}

const QHash<QString, KoXmlElement*>& KoOasisStyles::listStyles() const
{
    return d->listStyles;
}

const QHash<QString, KoXmlElement*>& KoOasisStyles::masterPages() const
{
    return d->masterPages;
}

const QHash<QString, KoXmlElement*>& KoOasisStyles::drawStyles() const
{
    return d->drawStyles;
}

const KoXmlElement* KoOasisStyles::findStyle( const QString& name ) const
{
    return d->styles[ name ];
}

const KoXmlElement* KoOasisStyles::findStyle( const QString& styleName, const QString& family ) const
{
    const KoXmlElement* style = findStyleCustomStyle( styleName, family );
    if ( !style )
        style = findStyleAutoStyle( styleName, family );
    if ( !style )
        style = findContentAutoStyle( styleName, family );
    return style;
}

const KoXmlElement* KoOasisStyles::findStyleCustomStyle( const QString& styleName, const QString& family ) const
{
    const KoXmlElement* style = d->customStyles.value( family ).value( styleName );
    if ( style && !family.isEmpty() ) {
        const QString styleFamily = style->attributeNS( KoXmlNS::style, "family", QString() );
        if ( styleFamily != family ) {
            kWarning() << "KoOasisStyles: was looking for style " << styleName
                        << " in family " << family << " but got " << styleFamily << endl;
        }
    }
    return style;
}

const KoXmlElement* KoOasisStyles::findStyleAutoStyle( const QString& styleName, const QString& family ) const
{
    const KoXmlElement* style = d->stylesAutoStyles.value( family ).value( styleName );
    if ( style ) {
        const QString styleFamily = style->attributeNS( KoXmlNS::style, "family", QString() );
        if ( styleFamily != family ) {
            kWarning() << "KoOasisStyles: was looking for style " << styleName
                        << " in family " << family << " but got " << styleFamily << endl;
        }
    }
    return style;
}

const KoXmlElement* KoOasisStyles::findContentAutoStyle( const QString& styleName, const QString& family ) const
{
    const KoXmlElement* style = d->contentAutoStyles.value( family ).value( styleName );
    if ( style ) {
        const QString styleFamily = style->attributeNS( KoXmlNS::style, "family", QString() );
        if ( styleFamily != family ) {
            kWarning() << "KoOasisStyles: was looking for style " << styleName
                        << " in family " << family << " but got " << styleFamily << endl;
        }
    }
    return style;
}
