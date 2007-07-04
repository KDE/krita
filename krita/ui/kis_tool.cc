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
#include <KoPointerEvent.h>
#include <KoViewConverter.h>
#include <KoSelection.h>

#include "kis_layer_shape.h"
#include "kis_layer_container_shape.h"
#include "kis_mask_shape.h"
#include "kis_shape_layer.h"

#include <kis_image.h>
#include <kis_layer.h>
#include <kis_group_layer.h>
#include <kis_adjustment_layer.h>
#include <kis_mask.h>
#include <kis_paint_layer.h>
#include <kis_brush.h>
#include <kis_pattern.h>
#include <kis_gradient.h>
#include "kis_resource_provider.h"

#include "kis_canvas2.h"
#include "kis_tool.h"

struct KisTool::Private {
    Private() : currentBrush(0),
            currentPattern(0),
            currentGradient(0),
            currentPaintOpSettings(0)
    { }
    QCursor cursor; // the cursor that should be shown on tool activation.

    // From the canvas resources
    KisBrush * currentBrush;
    KisPattern * currentPattern;
    KisGradient * currentGradient;
    KoColor currentFgColor;
    KoColor currentBgColor;
    QString currentPaintOp;
    KisPaintOpSettings * currentPaintOpSettings;
    KisLayerSP currentLayer;
    float currentExposure;
    KisImageSP currentImage;
};

KisTool::KisTool( KoCanvasBase * canvas, const QCursor & cursor )
    : KoTool( canvas )
    , d(new Private)
{
    d->cursor = cursor;
}

KisTool::~KisTool()
{
}

void KisTool::activate(bool )
{
    useCursor(d->cursor, true);

    d->currentFgColor = m_canvas->resourceProvider()->resource( KoCanvasResource::ForegroundColor ).value<KoColor>();
    d->currentBgColor = m_canvas->resourceProvider()->resource( KoCanvasResource::BackgroundColor ).value<KoColor>();
    d->currentBrush = static_cast<KisBrush *>( m_canvas->resourceProvider()->resource( KisResourceProvider::CurrentBrush ).value<void *>() );
    d->currentPattern = static_cast<KisPattern *>( m_canvas->resourceProvider()->resource( KisResourceProvider::CurrentPattern).value<void *>() );
    d->currentGradient = static_cast<KisGradient *>( m_canvas->resourceProvider()->resource( KisResourceProvider::CurrentGradient ).value<void *>() );
    d->currentPaintOp = m_canvas->resourceProvider()->resource( KisResourceProvider::CurrentPaintop ).value<KoID >().id();
    d->currentPaintOpSettings = static_cast<KisPaintOpSettings*>( m_canvas->resourceProvider()->resource( KisResourceProvider::CurrentPaintopSettings ).value<void *>() );
    d->currentLayer = m_canvas->resourceProvider()->resource( KisResourceProvider::CurrentKritaLayer ).value<KisLayerSP>();
    d->currentExposure = static_cast<float>( m_canvas->resourceProvider()->resource( KisResourceProvider::HdrExposure ).toDouble() );
    d->currentImage = image();
}



void KisTool::deactivate()
{
}

void KisTool::resourceChanged( int key, const QVariant & v )
{
    switch ( key ) {
    case ( KoCanvasResource::ForegroundColor ):
        d->currentFgColor = v.value<KoColor>();
        break;
    case ( KoCanvasResource::BackgroundColor ):
        d->currentBgColor = v.value<KoColor>();
        break;
    case ( KisResourceProvider::CurrentBrush ):
        d->currentBrush = static_cast<KisBrush *>( v.value<void *>() );
        break;
    case ( KisResourceProvider::CurrentPattern ):
        d->currentPattern = static_cast<KisPattern *>( v.value<void *>() );
        break;
    case ( KisResourceProvider::CurrentGradient ):
        d->currentGradient = static_cast<KisGradient *>( v.value<void *>() );
        break;
    case ( KisResourceProvider::CurrentPaintop ):
        d->currentPaintOp = v.value<KoID >().id();
        break;
    case ( KisResourceProvider::CurrentPaintopSettings ):
        d->currentPaintOpSettings = static_cast<KisPaintOpSettings*>( v.value<void *>() );
        break;
    case ( KisResourceProvider::HdrExposure ):
        d->currentExposure = static_cast<float>( v.toDouble() );
    case ( KisResourceProvider::CurrentKritaLayer ):
        d->currentLayer = v.value<KisLayerSP>();
    default:
        ;
        // Do nothing
    };
}

QPointF KisTool::convertToPixelCoord( KoPointerEvent *e )
{
    return image()->documentToPixel(e->point);
}

QPoint KisTool::convertToIntPixelCoord( KoPointerEvent *e )
{
    return image()->documentToIntPixel(e->point);
}

QRectF KisTool::convertToPt( const QRectF &rect )
{
    QRectF r;
    //We add 1 in the following to the extreme coords because a pixel always has size
    r.setCoords(int(rect.left()) / image()->xRes(), int(rect.top()) / image()->yRes(),
             int(1 + rect.right()) / image()->xRes(), int(1 + rect.bottom()) / image()->yRes());
    return r;
}

QPointF KisTool::pixelToView(const QPoint &pixelCoord)
{
    QPointF documentCoord = image()->pixelToDocument(pixelCoord);
    return m_canvas->viewConverter()->documentToView(documentCoord);
}

QPointF KisTool::pixelToView(const QPointF &pixelCoord)
{
    QPointF documentCoord = image()->pixelToDocument(pixelCoord);
    return m_canvas->viewConverter()->documentToView(documentCoord);
}

QRectF KisTool::pixelToView(const QRectF &pixelRect)
{
    QPointF topLeft = pixelToView(pixelRect.topLeft());
    QPointF bottomRight = pixelToView(pixelRect.bottomRight());
    return QRectF(topLeft, bottomRight);
}

void KisTool::updateCanvasPixelRect(const QRectF &pixelRect)
{
    m_canvas->updateCanvas(convertToPt(pixelRect));
}

void KisTool::updateCanvasViewRect(const QRectF &viewRect)
{
    m_canvas->updateCanvas(m_canvas->viewConverter()->viewToDocument(viewRect));
}

KisImageSP KisTool::image() const
{

    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*> ( m_canvas );
    if ( !kisCanvas ) {
        kDebug(41007) << "The current canvas is not a kis canvas!\n";
        return 0;
    }
#if 0
    KisImageSP img = kisCanvas->currentImage();

    return img;
#endif

    KoShape * shape = kisCanvas->globalShapeManager()->selection()->firstSelectedShape();

    if ( !shape ) return 0;

    if ( shape->shapeId() == KIS_LAYER_CONTAINER_ID ) {
        return static_cast<KisLayerContainerShape*>( shape )->groupLayer()->image();
    } else if ( shape->shapeId() ==  KIS_LAYER_SHAPE_ID) {
        return static_cast<KisLayerShape*>( shape )->layer()->image();
    } else if ( shape->shapeId() == KIS_MASK_SHAPE_ID ) {
        // XXX
        return 0;
    } else if ( shape->shapeId() == KIS_SHAPE_LAYER_ID ) {
        return static_cast<KisShapeLayer*>( shape )->image();
    } else {
        // First selected shape is not a krita layer type shape
        return 0;
    }

}

void KisTool::notifyModified() const
{
    KisImageSP img = image();
    if ( img ) {
        img->setModified();
    }
}

KisPattern * KisTool::currentPattern()
{
    return d->currentPattern;
}

KisGradient * KisTool::currentGradient()
{
    return d->currentGradient;
}

KisPaintOpSettings * KisTool::currentPaintOpSettings()
{
    return d->currentPaintOpSettings;
}

QString KisTool::currentPaintOp()
{
    return d->currentPaintOp;
}

KisBrush* KisTool::currentBrush()
{
    return d->currentBrush;
}

KisLayerSP KisTool::currentLayer()
{
    return d->currentLayer;
}

KoColor KisTool::currentFgColor()
{
    return d->currentFgColor;
}

KoColor KisTool::currentBgColor()
{
    return d->currentBgColor;
}

KisImageSP KisTool::currentImage()
{
    return d->currentImage;
}

#include "kis_tool.moc"
