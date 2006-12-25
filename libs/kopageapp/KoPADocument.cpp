/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>

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

#include "KoPADocument.h"

#include <kcommand.h>

#include <KoShapeManager.h>

#include "KoPACanvas.h"
#include "KoPAView.h"
#include "KoPAPage.h"
#include "KoPAMasterPage.h"
#include "KoPAShapeAddRemoveData.h"

KoPADocument::KoPADocument( QWidget* parentWidget, QObject* parent, bool singleViewMode )
: KoDocument( parentWidget, parent, singleViewMode )
{
    KoPAMasterPage * masterPage = new KoPAMasterPage();
    m_masterPages.append( masterPage );
    m_pages.append( new KoPAPage( masterPage ) );
}

KoPADocument::~KoPADocument()
{
    qDeleteAll( m_pages );
    qDeleteAll( m_masterPages );
    m_pages.clear();
}

void KoPADocument::paintContent( QPainter &painter, const QRect &rect, bool transparent,
                                double zoomX, double zoomY )
{
}

bool KoPADocument::loadXML( QIODevice *, const KoXmlDocument & doc )
{
    //Perhaps not necessary if we use filter import/export for old file format
    //only needed as it is in the base class will be removed.
    return true;
}

bool KoPADocument::loadOasis( const KoXmlDocument & doc, KoOasisStyles& oasisStyles,
                             const KoXmlDocument & settings, KoStore* store )
{
    return true;
}

bool KoPADocument::saveOasis( KoStore* store, KoXmlWriter* manifestWriter )
{
    return true;
}

KoPAPage* KoPADocument::pageByIndex(int index)
{
    return m_pages.at(index);
}

void KoPADocument::addShape( KoShape * shape, KoShapeAddRemoveData * addRemoveData )
{
    KoPAShapeAddRemoveData * arData = dynamic_cast<KoPAShapeAddRemoveData*>( addRemoveData );
    if ( arData )
    {
        arData->page()->addShape( shape );
        foreach( KoView *view, views() ) 
        {
            KoPAView * kogaView = static_cast<KoPAView*>( view );
            if ( kogaView->activePage() && kogaView->activePage() == arData->page() )
            {
                KoPACanvas *canvas = kogaView->kogaCanvas();
                canvas->shapeManager()->add( shape );
                //TODO repainting kprView->canvasWidget()->update();
            }
        }
    }
}


void KoPADocument::removeShape( KoShape *shape, KoShapeAddRemoveData * addRemoveData )
{
    pageByIndex( 0 )->removeShape( shape );
    foreach( KoView *view, views() ) 
    {
        KoPAView * kogaView = static_cast<KoPAView*>( view );
        if ( kogaView->activePage() /*TODO*/ )
        {
            KoPACanvas *canvas = kogaView->kogaCanvas();
            canvas->shapeManager()->remove( shape );
            //TODO repainting kView->canvasWidget()->update();
        }
    }
}

#include "KoPADocument.moc"
