/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include <QCursor>

#include <KoCanvasBase.h>
#include <KoShapeManager.h>
#include <KoTool.h>
#include <KoColor.h>
#include <KoID.h>

#include <kis_image.h>
#include <kis_layer.h>
#include <kis_brush.h>
#include <kis_pattern.h>
#include <kis_gradient.h>

#include "kis_dummy_shape.h"

#include "kis_tool.h"

KisTool::KisTool( KoCanvasBase * canvas, const QCursor & cursor )
    : KoTool( canvas )
    , m_cursor( cursor )
{
}

KisTool::~KisTool()
{
}

void KisTool::activate(bool )
{
    emit sigCursorChanged( m_cursor );

    m_currentFgColor = m_canvas->resourceProvider()->resource( KoCanvasResource::ForegroundColor ).value<KoColor>();
    m_currentBgColor = m_canvas->resourceProvider()->resource( KoCanvasResource::BackgroundColor ).value<KoColor>();
    m_currentBrush = static_cast<KisBrush *>( m_canvas->resourceProvider()->resource( KoCanvasResource::CurrentBrush ).value<void *>() );
    m_currentPattern = static_cast<KisPattern *>( m_canvas->resourceProvider()->resource( KoCanvasResource::CurrentPattern).value<void *>() );
    m_currentGradient = static_cast<KisGradient *>( m_canvas->resourceProvider()->resource( KoCanvasResource::CurrentGradient ).value<void *>() );
    m_currentPaintOp = m_canvas->resourceProvider()->resource( KoCanvasResource::CurrentPaintop ).value<KoID >();
    m_currentPaintOpSettings = static_cast<KisPaintOpSettings*>( m_canvas->resourceProvider()->resource( KoCanvasResource::CurrentPaintopSettings ).value<void *>() );
    m_currentLayer = m_canvas->resourceProvider()->resource( KoCanvasResource::CurrentKritaLayer ).value<KisLayerSP>();
    m_currentExposure = static_cast<float>( m_canvas->resourceProvider()->resource( KoCanvasResource::HdrExposure ).toDouble() );
    m_currentImage = image();
}



void KisTool::deactivate()
{
}

void KisTool::resourceChanged( KoCanvasResource::EnumCanvasResource key, const QVariant & v )
{

    switch ( key ) {
    case ( KoCanvasResource::ForegroundColor ):
        m_currentFgColor = v.value<KoColor>();
        break;
    case ( KoCanvasResource::BackgroundColor ):
        m_currentBgColor = v.value<KoColor>();
        break;
    case ( KoCanvasResource::CurrentBrush ):
        m_currentBrush = static_cast<KisBrush *>( v.value<void *>() );
        break;
    case ( KoCanvasResource::CurrentPattern ):
        m_currentPattern = static_cast<KisPattern *>( v.value<void *>() );
        break;
    case ( KoCanvasResource::CurrentGradient ):
        m_currentGradient = static_cast<KisGradient *>( v.value<void *>() );
        break;
    case ( KoCanvasResource::CurrentPaintop ):
        m_currentPaintOp = v.value<KoID >();
        break;
    case ( KoCanvasResource::CurrentPaintopSettings ):
        m_currentPaintOpSettings = static_cast<KisPaintOpSettings*>( v.value<void *>() );
        break;
    case ( KoCanvasResource::HdrExposure ):
        m_currentExposure = static_cast<float>( v.toDouble() );
    default:
        ;
        // Do nothing
    };
}

KisImageSP KisTool::image() const
{
    KoShapeManager * shapeManager = m_canvas->shapeManager();
    if ( !shapeManager ) return 0;

    KisDummyShape * imageShape = dynamic_cast<KisDummyShape *>( shapeManager->shapeAt(QPointF( 0, 0 )) );
    kDebug() << "Current shape: " << imageShape << endl;

    if ( !imageShape ) return 0;

    KisImageSP img = imageShape->image();
    return img;

}

void KisTool::notifyModified() const
{
    KisImageSP img = image();
    if ( img ) {
        img->setModified();
    }
}

#include "kis_tool.moc"
