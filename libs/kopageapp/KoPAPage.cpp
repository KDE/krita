/* This file is part of the KDE project
   Copyright (C) 2006-2008 Thorsten Zachmann <zachmann@kde.org>

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

#include <QPainter>

#include <KoShapePainter.h>
#include <KoShapeSavingContext.h>
#include <KoShapeLayer.h>
#include <KoOdfLoadingContext.h>
#include <KoStyleStack.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoZoomHandler.h>

#include "KoPAMasterPage.h"
#include "KoPASavingContext.h"
#include "KoPALoadingContext.h"
#include "KoPAUtil.h"

KoPAPage::KoPAPage( KoPAMasterPage * masterPage )
: KoPAPageBase()
, m_masterPage( masterPage )
, m_pageProperties( UseMasterBackground | DisplayMasterBackground | DisplayMasterShapes )
{
}

KoPAPage::~KoPAPage()
{
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

bool KoPAPage::displayMasterShapes()
{
    return m_pageProperties & DisplayMasterShapes;
}

QPixmap KoPAPage::thumbnail()
{
    QSize size( 512, 512 );

    KoZoomHandler zoomHandler;
    const KoPageLayout & layout = pageLayout();
    KoPAUtil::setZoom( layout, size, zoomHandler );
    QRect pageRect( KoPAUtil::pageRect( layout, size, zoomHandler ) );

    QPixmap pixmap( size.width(), size.height() );
    // should it be transparent at the places where it is to big?
    pixmap.fill( Qt::white );
    QPainter painter( &pixmap );
    painter.setClipRect( pageRect );
    painter.setRenderHint( QPainter::Antialiasing );
    painter.translate( pageRect.topLeft() );

    paintBackground( painter, zoomHandler );

    KoShapePainter shapePainter;
    if ( displayMasterShapes() ) {
        shapePainter.setShapes( masterPage()->iterator() );
        shapePainter.paintShapes( painter, zoomHandler );
    }
    shapePainter.setShapes( iterator() );
    shapePainter.paintShapes( painter, zoomHandler );
    return pixmap;
}

void KoPAPage::saveOdfPageStyleData( KoGenStyle &style, KoPASavingContext &paContext ) const
{
    if ( ( m_pageProperties & UseMasterBackground ) == 0 ) {
        KoPAPageBase::saveOdfPageStyleData( style, paContext );
    }
}
