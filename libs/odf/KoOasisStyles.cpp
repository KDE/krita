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
#include <KoXmlWriter.h>

#include <math.h>

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

QMatrix KoOasisStyles::loadTransformation( const QString &transformation )
{
    QMatrix matrix;

    // Split string for handling 1 transform statement at a time
    QStringList subtransforms = transformation.split(')', QString::SkipEmptyParts);
    QStringList::ConstIterator it = subtransforms.begin();
    QStringList::ConstIterator end = subtransforms.end();
    for(; it != end; ++it)
    {
        QStringList subtransform = (*it).split('(', QString::SkipEmptyParts);

        subtransform[0] = subtransform[0].trimmed().toLower();
        subtransform[1] = subtransform[1].simplified();
        QRegExp reg("[,( ]");
        QStringList params = subtransform[1].split(reg, QString::SkipEmptyParts);

        if(subtransform[0].startsWith(';') || subtransform[0].startsWith(','))
            subtransform[0] = subtransform[0].right(subtransform[0].length() - 1);

        if(subtransform[0] == "rotate")
        {
            // TODO find out what oo2 really does when rotating, it seems severly broken
            if(params.count() == 3)
            {
                double x = KoUnit::parseValue( params[1] );
                double y = KoUnit::parseValue( params[2] );

                matrix.translate(x, y);
                // oo2 rotates by radians
                matrix.rotate( params[0].toDouble()*180.0/M_PI );
                matrix.translate(-x, -y);
            }
            else
            {
                // oo2 rotates by radians
                matrix.rotate( params[0].toDouble()*180.0/M_PI );
            }
        }
        else if(subtransform[0] == "translate")
        {
            if(params.count() == 2)
            {
                double x = KoUnit::parseValue( params[0] );
                double y = KoUnit::parseValue( params[1] );
                matrix.translate(x, y);
            }
            else    // Spec : if only one param given, assume 2nd param to be 0
                matrix.translate( KoUnit::parseValue( params[0] ) , 0);
        }
        else if(subtransform[0] == "scale")
        {
            if(params.count() == 2)
                matrix.scale(params[0].toDouble(), params[1].toDouble());
            else    // Spec : if only one param given, assume uniform scaling
                matrix.scale(params[0].toDouble(), params[0].toDouble());
        }
        else if(subtransform[0] == "skewx")
            matrix.shear(tan(params[0].toDouble()), 0.0F);
        else if(subtransform[0] == "skewy")
            matrix.shear(tan(params[0].toDouble()), 0.0F);
        else if(subtransform[0] == "skewy")
            matrix.shear(0.0F, tan(params[0].toDouble()));
        else if(subtransform[0] == "matrix")
        {
            if(params.count() >= 6)
                matrix.setMatrix(params[0].toDouble(), params[1].toDouble(), params[2].toDouble(), params[3].toDouble(), KoUnit::parseValue( params[4] ), KoUnit::parseValue( params[5] ) );
        }
    }

    return matrix;
}

QString KoOasisStyles::saveTransformation( const QMatrix &transformation, bool appendTranslateUnit )
{
    QString transform;
    if( appendTranslateUnit )
        transform = QString( "matrix(%1 %2 %3 %4 %5pt %6pt)" )
            .arg( transformation.m11() ).arg( transformation.m12() )
            .arg( transformation.m21() ).arg( transformation.m22() )
            .arg( transformation.dx() ) .arg( transformation.dy() );
    else
        transform = QString( "matrix(%1 %2 %3 %4 %5 %6)" )
            .arg( transformation.m11() ).arg( transformation.m12() )
            .arg( transformation.m21() ).arg( transformation.m22() )
            .arg( transformation.dx() ) .arg( transformation.dy() );

    return transform;
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
