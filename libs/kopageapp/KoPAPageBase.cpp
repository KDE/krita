/* This file is part of the KDE project
   Copyright (C) 2006-2010 Thorsten Zachmann <zachmann@kde.org>

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

#include "KoPAPageBase.h"
#include "KoPASavingContext.h"
#include "KoPALoadingContext.h"
#include "KoPAPixmapCache.h"
#include "KoPAPageContainerModel.h"
#include "KoPASavingContext.h"

#include <QPainter>

#include <kdebug.h>

#include <KoXmlNS.h>
#include <KoPageLayout.h>
#include <KoShapeSavingContext.h>
#include <KoOdfLoadingContext.h>
#include <KoShapeLayer.h>
#include <KoShapeRegistry.h>
#include <KoGenStyle.h>
#include <KoGenStyles.h>
#include <KoOdfStylesReader.h>
#include <KoOdfGraphicStyles.h>
#include <KoXmlWriter.h>
#include <KoViewConverter.h>
#include <KoShapeBackground.h>

KoPAPageBase::KoPAPageBase()
: KoShapeContainer( new KoPAPageContainerModel() )
{
    // Add a default layer
    KoShapeLayer* layer = new KoShapeLayer;
    addShape(layer);
}

KoPAPageBase::~KoPAPageBase()
{
}

void KoPAPageBase::paintComponent(QPainter& painter, const KoViewConverter& converter)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
}

void KoPAPageBase::paintBackground( QPainter & painter, const KoViewConverter & converter )
{
    painter.save();
    applyConversion( painter, converter );
    KoPageLayout layout = pageLayout();
    painter.setPen( Qt::black );

    if( background() )
    {
        QPainterPath p;
        p.addRect( QRectF( 0.0, 0.0, layout.width, layout.height ) );
        background()->paint( painter, p );
    }

    painter.restore();
}

void KoPAPageBase::saveOdfPageContent( KoPASavingContext & paContext ) const
{
    saveOdfLayers(paContext);
    saveOdfShapes( paContext );
    saveOdfAnimations( paContext );
    saveOdfPresentationNotes( paContext );
}

void KoPAPageBase::saveOdfLayers(KoPASavingContext &paContext) const
{
    QList<KoShape*> shapes(this->shapes());
    qSort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);
    foreach(KoShape* shape, shapes) {
        KoShapeLayer *layer = dynamic_cast<KoShapeLayer*>(shape);
        if (layer) {
            paContext.addLayerForSaving(layer);
        }
        else {
            Q_ASSERT(layer);
            kWarning(30010) << "Page contains non layer where a layer is expected";
        }
    }
    paContext.saveLayerSet(paContext.xmlWriter());
    paContext.clearLayers();
}

void KoPAPageBase::saveOdfShapes( KoShapeSavingContext &context ) const
{
    QList<KoShape*> shapes(this->shapes());
    QList<KoShape*> tlshapes( shapes );

    qSort( tlshapes.begin(), tlshapes.end(), KoShape::compareShapeZIndex );

    foreach( KoShape *shape, tlshapes ) {
        shape->saveOdf( context );
    }
}

QString KoPAPageBase::saveOdfPageStyle( KoPASavingContext &paContext ) const
{
    KoGenStyle style( KoGenStyle::DrawingPageAutoStyle, "drawing-page" );

    if ( paContext.isSet( KoShapeSavingContext::AutoStyleInStyleXml ) ) {
        style.setAutoStyleInStylesDotXml( true );
    }

    saveOdfPageStyleData( style, paContext );

    return paContext.mainStyles().insert( style, "dp" );
}

void KoPAPageBase::saveOdfPageStyleData( KoGenStyle &style, KoPASavingContext &paContext ) const
{
    KoShapeBackground * bg = background();
    if( bg )
        bg->fillStyle( style, paContext );
}

bool KoPAPageBase::saveOdfAnimations( KoPASavingContext & paContext ) const
{
    Q_UNUSED( paContext );
    return true;
}

bool KoPAPageBase::saveOdfPresentationNotes(KoPASavingContext &paContext) const
{
    Q_UNUSED( paContext );
    return true;
}

bool KoPAPageBase::loadOdf( const KoXmlElement &element, KoShapeLoadingContext & loadingContext )
{
    KoPALoadingContext &paContext = static_cast<KoPALoadingContext&>( loadingContext );

    KoStyleStack& styleStack = loadingContext.odfLoadingContext().styleStack();
    styleStack.save();
    loadingContext.odfLoadingContext().fillStyleStack( element, KoXmlNS::draw, "style-name", "drawing-page" );
    styleStack.setTypeProperties( "drawing-page" );

    loadOdfPageTag(element, paContext);
    styleStack.restore();

    // load layers and shapes 
    const KoXmlElement & pageLayerSet = KoXml::namedItemNS( element, KoXmlNS::draw, "layer-set" );

    const KoXmlElement & usedPageLayerSet = pageLayerSet.isNull() ? loadingContext.odfLoadingContext().stylesReader().layerSet(): pageLayerSet;

    int layerZIndex = 0;
    bool first = true;
    KoXmlElement layerElement;
    forEachElement( layerElement, usedPageLayerSet ) {
        KoShapeLayer * layer = 0;
        if ( first ) {
            first = false;
            layer = dynamic_cast<KoShapeLayer *>( shapes().first() );
            Q_ASSERT( layer );
        }
        else {
            layer = new KoShapeLayer();
            addShape( layer );
        }
        if ( layer ) {
            layer->setZIndex( layerZIndex++ );
            layer->loadOdf( layerElement, loadingContext );
        }
    }

    KoShapeLayer * layer = dynamic_cast<KoShapeLayer *>( shapes().first() );
    if ( layer )
    {
        KoXmlElement child;
        forEachElement( child, element )
        {
            kDebug(30010) <<"loading shape" << child.localName();

            KoShape * shape = KoShapeRegistry::instance()->createShapeFromOdf( child, loadingContext );
            if ( shape ) {
                if( ! shape->parent() ) {
                    layer->addShape( shape );
                }
            }
        }
    }

    loadOdfPageExtra(element, paContext);

    return true;
}

void KoPAPageBase::loadOdfPageTag( const KoXmlElement &element,
                                   KoPALoadingContext &loadingContext )
{
    KoStyleStack& styleStack = loadingContext.odfLoadingContext().styleStack();

    if ( styleStack.hasProperty( KoXmlNS::draw, "fill" ) ) {
        setBackground(loadOdfFill(loadingContext));
    }
}

void KoPAPageBase::loadOdfPageExtra( const KoXmlElement &element, KoPALoadingContext & loadingContext )
{
    Q_UNUSED( element );
    Q_UNUSED( loadingContext );
}

QSizeF KoPAPageBase::size() const
{
    const KoPageLayout layout = pageLayout();
    return QSize( layout.width, layout.height );
}

QRectF KoPAPageBase::boundingRect() const
{
    //return KoShapeContainer::boundingRect();
    return contentRect().united(QRectF(QPointF(0, 0), size() ));
}

QRectF KoPAPageBase::contentRect() const
{
    QRectF bb;
    foreach (KoShape* layer, shapes()) {
        if (bb.isNull()) {
            bb = layer->boundingRect();
        }
        else {
            bb = bb.united(layer->boundingRect());
        }
    }

    return bb;
}

void KoPAPageBase::shapeAdded( KoShape * shape )
{
    Q_UNUSED( shape );
}

void KoPAPageBase::shapeRemoved( KoShape * shape )
{
    Q_UNUSED( shape );
}

KoPageApp::PageType KoPAPageBase::pageType() const
{
    return KoPageApp::Page;
}

QPixmap KoPAPageBase::thumbnail( const QSize& size )
{
#ifdef CACHE_PAGE_THUMBNAILS
    QString key = thumbnailKey();
    QPixmap pm;
    if ( !KoPAPixmapCache::instance()->find( key, size, pm ) ) {
        pm = generateThumbnail( size );
        KoPAPixmapCache::instance()->insert( key, pm );
        kDebug(30010) << "create thumbnail" << this;
    }
    else {
        kDebug(30010) << "thumbnail in cache " << this;
    }
    return pm;
#else
    return generateThumbnail( size );
#endif
}

void KoPAPageBase::pageUpdated()
{
    KoPAPixmapCache::instance()->remove( thumbnailKey() );
}

QString KoPAPageBase::thumbnailKey() const
{
     QString key;
     key.sprintf( "%p", static_cast<const void *>( this ) );
     return key;
}

KoShapeManagerPaintingStrategy * KoPAPageBase::getPaintingStrategy() const
{
    return 0;
}
