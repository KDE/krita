/* This file is part of the KDE project
   Copyright (C) 2005 David Faure <faure@kde.org>

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

#include "KoOdfLoadingContext.h"
#include <KoOdfReadStore.h>
#include <KoOdfStylesReader.h>
#include <KoStore.h>
#include <KoXmlNS.h>
#include <kdebug.h>
#include <KoDom.h>

class KoOdfLoadingContext::Private
{
};

KoOdfLoadingContext::KoOdfLoadingContext( KoOdfStylesReader& stylesReader, KoStore* store )
: m_store( store )
, m_stylesReader( stylesReader )
, m_metaXmlParsed( false )
, m_useStylesAutoStyles( false )
, d( 0 )
{
    // Ideally this should be done by KoDocument and passed as argument here...
    KoOdfReadStore oasisStore( store );
    QString dummy;
    (void)oasisStore.loadAndParse( "tar:/META-INF/manifest.xml", m_manifestDoc, dummy );
}


KoOdfLoadingContext::~KoOdfLoadingContext()
{
    delete d;
}

void KoOdfLoadingContext::fillStyleStack( const KoXmlElement& object, const char* nsURI, const char* attrName, const char* family )
{
    // find all styles associated with an object and push them on the stack
    if ( object.hasAttributeNS( nsURI, attrName ) ) {
        const QString styleName = object.attributeNS( nsURI, attrName, QString() );
        const KoXmlElement * style = m_stylesReader.findStyle( styleName, family, m_useStylesAutoStyles );

        if ( style )
            addStyles( style, family, m_useStylesAutoStyles );
        else
            kWarning(32500) << "style" << styleName << "not found in" << ( m_useStylesAutoStyles ? "styles.xml" : "content.xml" );
    }
}

void KoOdfLoadingContext::addStyles( const KoXmlElement* style, const char* family, bool usingStylesAutoStyles )
{
    Q_ASSERT( style );
    if ( !style ) return;

    // this recursive function is necessary as parent styles can have parents themselves
    if ( style->hasAttributeNS( KoXmlNS::style, "parent-style-name" ) ) {
        const QString parentStyleName = style->attributeNS( KoXmlNS::style, "parent-style-name", QString() );
        const KoXmlElement* parentStyle = m_stylesReader.findStyle( parentStyleName, family, usingStylesAutoStyles );

        if ( parentStyle )
            addStyles( parentStyle, family, usingStylesAutoStyles );
        else
            kWarning(32500) << "Parent style not found: " << parentStyleName;
    }
    else if ( family ) {
        const KoXmlElement* def = m_stylesReader.defaultStyle( family );
        if ( def ) { // on top of all, the default style for this family
            //kDebug(32500) <<"pushing default style" << style->attributeNS( KoXmlNS::style,"name", QString() );
            m_styleStack.push( *def );
        }
    }

    //kDebug(32500) <<"pushing style" << style->attributeNS( KoXmlNS::style,"name", QString() );
    m_styleStack.push( *style );
}

QString KoOdfLoadingContext::generator() const
{
    parseMeta();
    return m_generator;
}

void KoOdfLoadingContext::parseMeta() const
{
    if ( !m_metaXmlParsed && m_store )
    {
        if ( m_store->hasFile( "meta.xml" ) )
        {
            KoXmlDocument metaDoc;
            KoOdfReadStore oasisStore( m_store );
            QString errorMsg;
            if ( oasisStore.loadAndParse( "meta.xml", metaDoc, errorMsg ) ) {
                KoXmlNode meta   = KoDom::namedItemNS( metaDoc, KoXmlNS::office, "document-meta" );
                KoXmlNode office = KoDom::namedItemNS( meta, KoXmlNS::office, "meta" );
                KoXmlElement generator = KoDom::namedItemNS( office, KoXmlNS::meta, "generator" );
                if ( !generator.isNull() )
                    m_generator = generator.text();
            }
        }
        m_metaXmlParsed = true;
    }
}
