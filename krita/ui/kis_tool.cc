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

#include "kis_tool.h"
#include <QCursor>

#include <KoCanvasBase.h>
#include <KoShapeManager.h>
#include <KoTool.h>
#include <KoColor.h>
#include <KoID.h>
#include <KoPointerEvent.h>
#include <KoViewConverter.h>
#include <KoSelection.h>
#include <KoAbstractGradient.h>

#include <kis_paintop_registry.h>
#include "kis_layer_shape.h"
#include "kis_layer_container_shape.h"
#include "kis_mask_shape.h"
#include "kis_shape_layer.h"

#include <kis_view2.h>
#include <kis_selection.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_adjustment_layer.h>
#include <kis_mask.h>
#include <kis_paint_layer.h>
#include <kis_painter.h>
#include <kis_paintop.h>
#include <kis_brush.h>
#include <kis_pattern.h>
#include "kis_canvas_resource_provider.h"
#include <kis_paintop_settings.h>
#include "canvas/kis_canvas2.h"
#include "filter/kis_filter_configuration.h"

struct KisTool::Private {
    Private() : currentBrush(0),
                currentPattern(0),
                currentGradient(0),
                currentPaintOpSettings(0),
                currentGenerator(0)
        { }
    QCursor cursor; // the cursor that should be shown on tool activation.

    // From the canvas resources
    KisBrush * currentBrush;
    KisPattern * currentPattern;
    KoAbstractGradient * currentGradient;
    KoColor currentFgColor;
    KoColor currentBgColor;
    QString currentPaintOp;
    KisPaintOpSettingsSP currentPaintOpSettings;
    KisNodeSP currentNode;
    float currentExposure;
    KisFilterConfiguration * currentGenerator;
};

KisTool::KisTool( KoCanvasBase * canvas, const QCursor & cursor )
    : KoTool( canvas )
    , d(new Private)
{
    d->cursor = cursor;
}

KisTool::~KisTool()
{
    delete d;
}

void KisTool::activate(bool )
{
    useCursor(d->cursor, true);

    d->currentFgColor = m_canvas->resourceProvider()->resource( KoCanvasResource::ForegroundColor ).value<KoColor>();
    d->currentBgColor = m_canvas->resourceProvider()->resource( KoCanvasResource::BackgroundColor ).value<KoColor>();
    d->currentBrush = static_cast<KisBrush *>( m_canvas->resourceProvider()->
                        resource( KisCanvasResourceProvider::CurrentBrush ).value<void *>() );
    d->currentPattern = static_cast<KisPattern *>( m_canvas->resourceProvider()->
                        resource( KisCanvasResourceProvider::CurrentPattern).value<void *>() );
    d->currentGradient = static_cast<KoAbstractGradient *>( m_canvas->resourceProvider()->
                        resource( KisCanvasResourceProvider::CurrentGradient ).value<void *>() );
    d->currentPaintOp = m_canvas->resourceProvider()->
                        resource( KisCanvasResourceProvider::CurrentPaintop ).value<KoID >().id();
    d->currentPaintOpSettings = static_cast<KisPaintOpSettings*>( m_canvas->resourceProvider()->
                        resource( KisCanvasResourceProvider::CurrentPaintopSettings ).value<void *>() );

    if( d->currentPaintOpSettings )
    {
        d->currentPaintOpSettings->activate();
    }

    d->currentNode = m_canvas->resourceProvider()->
                        resource( KisCanvasResourceProvider::CurrentKritaNode).value<KisNodeSP>();
    if (d->currentNode)
        dbgUI << "Activating tool " << toolId() << " with node " << d->currentNode->name();
    else
        dbgUI << "Activating tool " << toolId() << " with no node ";
    d->currentExposure = static_cast<float>( m_canvas->resourceProvider()->
                        resource( KisCanvasResourceProvider::HdrExposure ).toDouble() );
    d->currentGenerator = static_cast<KisFilterConfiguration*>(m_canvas->resourceProvider()->
                        resource( KisCanvasResourceProvider::CurrentGeneratorConfiguration).value<void *>() );
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
    case ( KisCanvasResourceProvider::CurrentBrush ):
        d->currentBrush = static_cast<KisBrush *>( v.value<void *>() );
        break;
    case ( KisCanvasResourceProvider::CurrentPattern ):
        d->currentPattern = static_cast<KisPattern *>( v.value<void *>() );
        break;
    case ( KisCanvasResourceProvider::CurrentGradient ):
        d->currentGradient = static_cast<KoAbstractGradient *>( v.value<void *>() );
        break;
    case ( KisCanvasResourceProvider::CurrentPaintop ):
        d->currentPaintOp = v.value<KoID >().id();
        break;
    case ( KisCanvasResourceProvider::CurrentPaintopSettings ):
        d->currentPaintOpSettings = static_cast<KisPaintOpSettings*>( v.value<void *>() );
        break;
    case ( KisCanvasResourceProvider::HdrExposure ):
        d->currentExposure = static_cast<float>( v.toDouble() );
    case ( KisCanvasResourceProvider::CurrentGeneratorConfiguration ):
        d->currentGenerator = static_cast<KisFilterConfiguration*>(v.value<void *>() );
    case ( KisCanvasResourceProvider::CurrentKritaNode):
        d->currentNode = (v.value<KisNodeSP>() );
        if (d->currentNode)
            dbgUI << " node changed to " << d->currentNode->name();
    default:
        ;
        // Do nothing
    };
}

QPointF KisTool::convertToPixelCoord( KoPointerEvent *e )
{
    if (!image())
        return e->point;

    return image()->documentToPixel(e->point);
}

QPointF KisTool::convertToPixelCoord( const QPointF& pt )
{
    if (!image())
        return pt;

    return image()->documentToPixel(pt);
}

QPoint KisTool::convertToIntPixelCoord( KoPointerEvent *e )
{
    if (!image())
        return e->point.toPoint();

    return image()->documentToIntPixel(e->point);
}

QPointF KisTool::viewToPixel(const QPointF &viewCoord)
{
    if (!image())
        return viewCoord;

    return image()->documentToPixel( m_canvas->viewConverter()->viewToDocument( viewCoord) );
}

QRectF KisTool::convertToPt( const QRectF &rect )
{
    if (!image())
        return rect;
    QRectF r;
    //We add 1 in the following to the extreme coords because a pixel always has size
    r.setCoords(int(rect.left()) / image()->xRes(), int(rect.top()) / image()->yRes(),
                int(1 + rect.right()) / image()->xRes(), int(1 + rect.bottom()) / image()->yRes());
    return r;
}

QPointF KisTool::pixelToView(const QPoint &pixelCoord)
{
    if (!image())
        return pixelCoord;
    QPointF documentCoord = image()->pixelToDocument(pixelCoord);
    return m_canvas->viewConverter()->documentToView(documentCoord);
}

QPointF KisTool::pixelToView(const QPointF &pixelCoord)
{
    if (!image())
        return pixelCoord;
    QPointF documentCoord = image()->pixelToDocument(pixelCoord);
    return m_canvas->viewConverter()->documentToView(documentCoord);
}

QRectF KisTool::pixelToView(const QRectF &pixelRect)
{
    if (!image())
        return pixelRect;
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
    // For now, krita tools only work in krita, not for a krita shape. Krita shapes are for 2.1
    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*> ( m_canvas );
    if ( kisCanvas ) {
        return kisCanvas->currentImage();
    }

    return 0;

}

KisSelectionSP KisTool::currentSelection() const
{
    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*> ( m_canvas );
    if ( kisCanvas ) {
        KisView2 * view = kisCanvas->view();
        if (view) return view->selection();
    }

    return 0;

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

KoAbstractGradient * KisTool::currentGradient()
{
    return d->currentGradient;
}

KisPaintOpSettingsSP KisTool::currentPaintOpSettings()
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

KisNodeSP KisTool::currentNode()
{
    return d->currentNode;
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
    return image();
}

KisFilterConfiguration * KisTool::currentGenerator()
{
    return d->currentGenerator;
}

void KisTool::mousePressEvent( KoPointerEvent *event ) {
    event->ignore();
}

void KisTool::mouseMoveEvent( KoPointerEvent *event ) {
    event->ignore();
}

void KisTool::mouseReleaseEvent( KoPointerEvent *event ) {
    event->ignore();
}

void KisTool::setupPainter(KisPainter * painter)
{
    Q_ASSERT(currentImage());
    if (!currentImage()) return;

    painter->setBounds( currentImage()->bounds() );
    painter->setPaintColor(currentFgColor());
    painter->setBackgroundColor(currentBgColor());
    painter->setGenerator(currentGenerator());
    painter->setBrush(currentBrush());
    painter->setPattern(currentPattern());
    painter->setGradient(currentGradient());

    KisPaintOp * op = KisPaintOpRegistry::instance()->paintOp(currentPaintOp(),
                                                              currentPaintOpSettings(),
                                                              painter,
                                                              currentImage());
    painter->setPaintOp(op);

}

#include "kis_tool.moc"
