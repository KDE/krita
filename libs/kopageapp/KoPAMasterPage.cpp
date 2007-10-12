/* This file is part of the KDE project
   Copyright (C) 2006-2007 Thorsten Zachmann <zachmann@kde.org>

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

#include "KoPAMasterPage.h"

#include <QBuffer>
#include <KoGenStyle.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoOasisStyles.h>
#include <KoOasisLoadingContext.h>

#include "KoPASavingContext.h"
#include "KoPALoadingContext.h"

KoPAMasterPage::KoPAMasterPage()
: KoPAPageBase()
{
    m_pageLayout = KoPageLayout::standardLayout();
    setPageTitle ( "Standard" );
}

KoPAMasterPage::~KoPAMasterPage()
{
}

void KoPAMasterPage::saveOdf( KoShapeSavingContext & context ) const
{
    KoPASavingContext &paContext = static_cast<KoPASavingContext&>( context );

    KoGenStyle pageLayoutStyle = pageLayout().saveOasis();
    pageLayoutStyle.setAutoStyleInStylesDotXml( true );
    pageLayoutStyle.addAttribute( "style:page-usage", "all" );
    QString pageLayoutName( paContext.mainStyles().lookup( pageLayoutStyle, "pm" ) );

    KoGenStyle pageMaster( KoGenStyle::StyleMaster );
    pageMaster.addAttribute( "style:page-layout-name", pageLayoutName );
    pageMaster.addAttribute( "style:display-name", pageTitle() );

    KoXmlWriter &savedWriter = paContext.xmlWriter();

    QBuffer buffer;
    buffer.open( QIODevice::WriteOnly );
    KoXmlWriter xmlWriter( &buffer );

    paContext.setXmlWriter( xmlWriter );

    saveOdfPageContent( paContext );

    paContext.setXmlWriter( savedWriter );

    QString contentElement = QString::fromUtf8( buffer.buffer(), buffer.buffer().size() );
    pageMaster.addChildElement( "content", contentElement );
    paContext.addMasterPage( this, paContext.mainStyles().lookup( pageMaster, "Default" ) );
}

void KoPAMasterPage::loadOdfPageTag( const KoXmlElement &element, KoPALoadingContext &loadingContext )
{
    if ( element.hasAttributeNS( KoXmlNS::style, "display-name" ) ) {
        setPageTitle( element.attributeNS( KoXmlNS::style, "display-name" ) );
    }
    else {
        setPageTitle( element.attributeNS( KoXmlNS::style, "name" ) );
    }
    QString pageLayoutName = element.attributeNS( KoXmlNS::style, "page-layout-name" );
    const KoOasisStyles& styles = loadingContext.koLoadingContext().oasisStyles();
    const KoXmlElement* masterPageStyle = styles.findStyle( pageLayoutName );
    KoPageLayout pageLayout = KoPageLayout::standardLayout();

    if ( masterPageStyle ) {
        pageLayout.loadOasis( *masterPageStyle );
    }

    setPageLayout( pageLayout );
}
