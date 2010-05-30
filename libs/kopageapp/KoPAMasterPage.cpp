/* This file is part of the KDE project
   Copyright (C) 2006-2009 Thorsten Zachmann <zachmann@kde.org>

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
#include <QPainter>

#include <KoShapePainter.h>
#include <KoGenStyle.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoOdfStylesReader.h>
#include <KoOdfLoadingContext.h>
#include <KoZoomHandler.h>

#include "KoPASavingContext.h"
#include "KoPALoadingContext.h"
#include "KoPAUtil.h"
#include "KoPAPixmapCache.h"

KoPAMasterPage::KoPAMasterPage()
: KoPAPageBase()
{
    setName( "Standard" );
}

KoPAMasterPage::~KoPAMasterPage()
{
}

void KoPAMasterPage::saveOdf( KoShapeSavingContext & context ) const
{
    KoPASavingContext &paContext = static_cast<KoPASavingContext&>( context );

    KoGenStyle pageLayoutStyle = pageLayout().saveOdf();
    pageLayoutStyle.setAutoStyleInStylesDotXml( true );
    pageLayoutStyle.addAttribute( "style:page-usage", "all" );
    QString pageLayoutName( paContext.mainStyles().insert( pageLayoutStyle, "pm" ) );

    KoGenStyle pageMaster( KoGenStyle::MasterPageStyle );
    pageMaster.addAttribute( "style:page-layout-name", pageLayoutName );
    pageMaster.addAttribute( "style:display-name", name() );
    pageMaster.addAttribute( "draw:style-name", saveOdfPageStyle( paContext ) );

    KoXmlWriter &savedWriter = paContext.xmlWriter();

    QBuffer buffer;
    buffer.open( QIODevice::WriteOnly );
    KoXmlWriter xmlWriter( &buffer );

    paContext.setXmlWriter( xmlWriter );

    saveOdfPageContent( paContext );

    paContext.setXmlWriter( savedWriter );

    QString contentElement = QString::fromUtf8( buffer.buffer(), buffer.buffer().size() );
    pageMaster.addChildElement( paContext.masterPageElementName(), contentElement );
    paContext.addMasterPage( this, paContext.mainStyles().insert( pageMaster, "Default" ) );
}

void KoPAMasterPage::loadOdfPageTag( const KoXmlElement &element, KoPALoadingContext &loadingContext )
{
    KoPAPageBase::loadOdfPageTag( element, loadingContext );
    if ( element.hasAttributeNS( KoXmlNS::style, "display-name" ) ) {
        setName( element.attributeNS( KoXmlNS::style, "display-name" ) );
    }
    else {
        setName( element.attributeNS( KoXmlNS::style, "name" ) );
    }
    QString pageLayoutName = element.attributeNS( KoXmlNS::style, "page-layout-name" );
    const KoOdfStylesReader& styles = loadingContext.odfLoadingContext().stylesReader();
    const KoXmlElement* masterPageStyle = styles.findStyle( pageLayoutName );
    KoPageLayout pageLayout;

    if ( masterPageStyle ) {
        pageLayout.loadOdf( *masterPageStyle );
    }

    setPageLayout( pageLayout );
}

bool KoPAMasterPage::displayMasterShapes()
{
    return false;
}

void KoPAMasterPage::setDisplayMasterShapes( bool display )
{
    Q_UNUSED( display );
}

bool KoPAMasterPage::displayMasterBackground()
{
    return false;
}

void KoPAMasterPage::setDisplayMasterBackground( bool display )
{
    Q_UNUSED( display );
}

bool KoPAMasterPage::displayShape(KoShape *shape) const
{
    Q_UNUSED(shape);
    return true;
}

void KoPAMasterPage::pageUpdated()
{
    KoPAPageBase::pageUpdated();
    // TODO that is not the best way as it removes to much from the cache
    KoPAPixmapCache::instance()->clear( false );
}

QPixmap KoPAMasterPage::generateThumbnail( const QSize& size )
{
    // don't paint null pixmap
    if ( size.isEmpty() ) // either width or height is <= 0
        return QPixmap();

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

    paintPage( painter, zoomHandler );
    return pixmap;
}

void KoPAMasterPage::paintPage( QPainter & painter, KoZoomHandler & zoomHandler )
{
    paintBackground( painter, zoomHandler );

    KoShapePainter shapePainter;
    shapePainter.setShapes( shapes() );
    shapePainter.paint(painter, zoomHandler);
}
