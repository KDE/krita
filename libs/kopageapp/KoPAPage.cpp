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

#include "KoPAPage.h"

#include <KoShapeSavingContext.h>
#include <KoShapeLayer.h>
#include <KoOdfLoadingContext.h>
#include <KoStyleStack.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>

#include "KoPAMasterPage.h"
#include "KoPASavingContext.h"
#include "KoPALoadingContext.h"

KoPAPage::KoPAPage( KoPAMasterPage * masterPage )
: KoPAPageBase()
, m_masterPage( masterPage )
, m_pageProperties( UseMasterBackground | DisplayMasterBackground | DisplayMasterShapes )
{
}

KoPAPage::~KoPAPage()
{
}

KoShape * KoPAPage::cloneShape() const
{
    // TODO implement cloning
    return 0;
}

void KoPAPage::saveOdf( KoShapeSavingContext & context ) const
{
    KoPASavingContext &paContext = static_cast<KoPASavingContext&>( context );

    paContext.xmlWriter().startElement( "draw:page" );
    paContext.xmlWriter().addAttribute( "draw:name", name () );
    paContext.xmlWriter().addAttribute( "draw:id", "page" + QString::number( paContext.page() ) );
    paContext.xmlWriter().addAttribute( "draw:master-page-name", paContext.masterPageName( m_masterPage ) );
    paContext.xmlWriter().addAttribute( "draw:style-name", saveOdfPageStyle( paContext ) );

    saveOdfPageContent( paContext );

    paContext.xmlWriter().endElement();
}

KoPageLayout & KoPAPage::pageLayout()
{
    Q_ASSERT( m_masterPage );

    return m_masterPage->pageLayout();
}

void KoPAPage::loadOdfPageTag( const KoXmlElement &element, KoPALoadingContext &loadingContext )
{
    KoStyleStack& styleStack = loadingContext.odfLoadingContext().styleStack();
    int pageProperties = UseMasterBackground | DisplayMasterShapes | DisplayMasterBackground;
    if ( styleStack.hasProperty( KoXmlNS::draw, "fill" ) ) {
        KoPAPageBase::loadOdfPageTag( element, loadingContext );
        pageProperties = DisplayMasterShapes;
    }
    m_pageProperties = pageProperties;
    setName( element.attributeNS( KoXmlNS::draw, "name" ) );
    QString master = element.attributeNS (KoXmlNS::draw, "master-page-name" );
    setMasterPage( loadingContext.masterPageFromName( master ) );
}

void KoPAPage::setMasterPage( KoPAMasterPage * masterPage )
{
    m_masterPage = masterPage;
}

void KoPAPage::paintBackground( QPainter & painter, const KoViewConverter & converter )
{
    if ( m_pageProperties & UseMasterBackground ) {
        if ( m_pageProperties & DisplayMasterBackground ) {
            Q_ASSERT( m_masterPage );
            m_masterPage->paintBackground( painter, converter );
        }
    }
    else {
        KoPAPageBase::paintBackground( painter, converter );
    }
}
