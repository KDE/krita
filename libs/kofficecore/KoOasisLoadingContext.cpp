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

#include "KoOasisLoadingContext.h"
#include <KoOasisStore.h>
#include <KoOasisStyles.h>
#include <KoStore.h>
#include <KoXmlNS.h>
#include <kdebug.h>
#include <KoDom.h>

KoOasisLoadingContext::KoOasisLoadingContext( KoDocument* doc,
                                              KoOasisStyles& styles, KoStore* store )
    : m_doc( doc ), m_store( store ), m_styles( styles ),
      m_metaXmlParsed( false ), m_useStylesAutoStyles( false )
{
    // Ideally this should be done by KoDocument and passed as argument here...
    KoOasisStore oasisStore( store );
    QString dummy;
    (void)oasisStore.loadAndParse( "tar:/META-INF/manifest.xml", m_manifestDoc, dummy );
}


KoOasisLoadingContext::~KoOasisLoadingContext()
{

}

void KoOasisLoadingContext::fillStyleStack( const KoXmlElement& object, const char* nsURI, const char* attrName, const char* family )
{
    // find all styles associated with an object and push them on the stack
    if ( object.hasAttributeNS( nsURI, attrName ) ) {
        const QString styleName = object.attributeNS( nsURI, attrName, QString::null );
        const KoXmlElement* style = 0;
        bool isStyleAutoStyle = false;
        if ( m_useStylesAutoStyles ) {
            // When loading something from styles.xml, look into the styles.xml auto styles first
            style = m_styles.findStyleAutoStyle( styleName, family );
            // and fallback to looking at styles(), which includes the user styles from styles.xml
            if ( style )
                isStyleAutoStyle = true;
        }
        if ( !style )
            style = m_styles.findStyle( styleName, family );
        if ( style )
            addStyles( style, family, isStyleAutoStyle );
        else
            kWarning(32500) << "fillStyleStack: no style named " << styleName << " found." << endl;
    }
}

void KoOasisLoadingContext::addStyles( const KoXmlElement* style, const char* family, bool usingStylesAutoStyles )
{
    Q_ASSERT( style );
    if ( !style ) return;
    // this recursive function is necessary as parent styles can have parents themselves
    if ( style->hasAttributeNS( KoXmlNS::style, "parent-style-name" ) ) {
        const QString parentStyleName = style->attributeNS( KoXmlNS::style, "parent-style-name", QString::null );
        const KoXmlElement* parentStyle = 0;
        if ( usingStylesAutoStyles ) {
            // When loading something from styles.xml, look into the styles.xml auto styles first
            parentStyle = m_styles.findStyleAutoStyle( parentStyleName, family );
            // and fallback to looking at styles(), which includes the user styles from styles.xml
        }
        if ( !parentStyle )
            parentStyle = m_styles.findStyle( parentStyleName, family );
        if ( parentStyle )
            addStyles( parentStyle, family, usingStylesAutoStyles );
        else
            kWarning(32500) << "Parent style not found: " << parentStyleName << endl;
    }
    else if ( family ) {
        const KoXmlElement* def = m_styles.defaultStyle( family );
        if ( def ) { // on top of all, the default style for this family
            //kDebug(32500) << "pushing default style " << style->attributeNS( KoXmlNS::style, "name", QString::null ) << endl;
            m_styleStack.push( *def );
        }
    }

    //kDebug(32500) << "pushing style " << style->attributeNS( KoXmlNS::style, "name", QString::null ) << endl;
    m_styleStack.push( *style );
}

QString KoOasisLoadingContext::generator() const
{
    parseMeta();
    return m_generator;
}

void KoOasisLoadingContext::parseMeta() const
{
    if ( !m_metaXmlParsed && m_store )
    {
        if ( m_store->hasFile( "meta.xml" ) )
        {
            KoXmlDocument metaDoc;
            KoOasisStore oasisStore( m_store );
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
