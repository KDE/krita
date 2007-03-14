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

#include <QDebug>

#include <KoShapeLayer.h>
#include <KoShapeSavingContext.h>
#include <KoGenStyle.h>
#include <KoGenStyles.h>
#include <KoOasisStyles.h>
#include <KoXmlWriter.h>
#include <KoViewConverter.h>

#include "KoPASavingContext.h"
#include "KoPAStyles.h"

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

bool KoPAPageBase::saveOdf( KoPASavingContext &paContext ) const
{
    createOdfPageTag( paContext );

    paContext.xmlWriter().addAttribute( "draw:style-name", saveOdfPageStyle( paContext ) );

    saveOdfShapes( paContext );
    saveOdfAnimations( paContext );
    saveOdfPresentationNotes();        

    paContext.xmlWriter().endElement(); //draw:page
    return true;
}

bool KoPAPageBase::saveOdfShapes( KoPASavingContext &paContext ) const
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
        shape->saveOdf( paContext );
    }

    return true;
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

bool KoPAPageBase::loadOdf( const KoXmlElement &element, KoOasisLoadingContext & loadingContext )
{
    // load shapes, this is only for testing
    KoXmlNode n = element.firstChild();
    for ( ; !n.isNull(); n = n.nextSibling() )
    {
        if ( n.isElement() )
        {
            KoXmlElement child = n.toElement();
            qDebug() << "Element:" << child.tagName();
        }
    }
    return true;
}
