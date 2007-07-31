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

#include "KoPAPageBase.h"
#include "KoPASavingContext.h"
#include "KoPAStyles.h"
#include "KoPALoadingContext.h"

#include <QDebug>
#include <QPainter>

#include <kdebug.h>

#include <KoShapeSavingContext.h>
#include <KoOasisLoadingContext.h>
#include <KoShapeLayer.h>
#include <KoShapeRegistry.h>
#include <KoGenStyle.h>
#include <KoGenStyles.h>
#include <KoOasisStyles.h>
#include <KoXmlWriter.h>
#include <KoViewConverter.h>

KoPAPageBase::KoPAPageBase()
: KoShapeContainer()
{
    // Add a default layer
    KoShapeLayer* layer = new KoShapeLayer;
    addChild(layer);
}

KoPAPageBase::~KoPAPageBase()
{
}

QString KoPAPageBase::pageTitle() const
{
    return m_pageTitle;
}

void KoPAPageBase::setPageTitle( const QString &title )
{
    m_pageTitle = title;
}

void KoPAPageBase::paintComponent(QPainter& painter, const KoViewConverter& converter)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
}

void KoPAPageBase::saveOdf( KoShapeSavingContext & context ) const
{
    KoPASavingContext &paContext = static_cast<KoPASavingContext&>( context );
    createOdfPageTag( paContext );

    context.xmlWriter().addAttribute( "draw:style-name", saveOdfPageStyle( paContext ) );

    saveOdfShapes( context );
    saveOdfAnimations( paContext );
    saveOdfPresentationNotes();        

    context.xmlWriter().endElement(); //draw:page
}

void KoPAPageBase::saveOdfShapes( KoShapeSavingContext &context ) const
{
    QList<KoShape*> shapes( iterator() );
    QList<KoShape*> tlshapes;

    foreach( KoShape *shape, shapes ) {
        KoShapeLayer *layer = dynamic_cast<KoShapeLayer *>( shape );

        Q_ASSERT( layer );
        if ( layer ) {
            QList<KoShape*> layerShapes( layer->iterator() ); 
            foreach( KoShape *layerShape, layerShapes ) {
                tlshapes.append( layerShape );
            }
        }
    }

    qSort( tlshapes.begin(), tlshapes.end(), KoShape::compareShapeZIndex );

    foreach( KoShape *shape, tlshapes ) {
        shape->saveOdf( context );
    }
}

QString KoPAPageBase::saveOdfPageStyle( KoPASavingContext &paContext ) const
{
    KoGenStyle style( KoPAStyles::STYLE_PAGE, "drawing-page" );

    if ( paContext.isSet( KoShapeSavingContext::AutoStyleInStyleXml ) ) {
        style.setAutoStyleInStylesDotXml( true );
    }

    saveOdfPageStyleData( style, paContext );

    return paContext.mainStyles().lookup( style, "dp" );
}

void KoPAPageBase::saveOdfPageStyleData( KoGenStyle &style, KoPASavingContext &paContext ) const
{
    //TODO
    QBrush background( Qt::white );
    KoOasisStyles::saveOasisFillStyle( style, paContext.mainStyles(), background );
}

bool KoPAPageBase::loadOdf( const KoXmlElement &element, KoShapeLoadingContext & loadingContext )
{
    KoPALoadingContext &paContext = static_cast<KoPALoadingContext&>( loadingContext );

    loadOdfPageTag(element, paContext);

    // load layers and shapes 
    // This needs some work as this is only for layers which are the same for all pages
    KoXmlElement layerElement;
    forEachElement( layerElement, loadingContext.koLoadingContext().oasisStyles().layerSet() )
    {
        KoShapeLayer * layer = new KoShapeLayer();
        if ( layer->loadOdf( layerElement, loadingContext ) ) {
            addChild( layer );
        }
    }

    KoShapeLayer * layer = dynamic_cast<KoShapeLayer *>( iterator().first() );
    if ( layer )
    {
        KoXmlElement child;
        forEachElement( child, element )
        {
            kDebug() <<"loading shape" << child.localName();

            KoShape * shape = KoShapeRegistry::instance()->createShapeFromOdf( child, loadingContext );
            if ( shape ) {
                if( ! shape->parent() ) {
                    layer->addChild( shape );
                }
            }
        }
    }

    return true;
}

void KoPAPageBase::loadOdfPageTag( const KoXmlElement &element, KoPALoadingContext &loadingContext )
{
}
