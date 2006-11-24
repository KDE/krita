/*
 *  Copyright (c) 2006 Boudewijn Rempt  <boud@valdyas.org>
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

#include "kis_resource_provider.h"

#include <QImage>
#include <QPainter>

#include <KoCanvasBase.h>
#include <KoID.h>

#include <kis_brush.h>
#include <kis_pattern.h>
#include <kis_gradient.h>
#include <kis_layer.h>

#include "kis_view2.h"
#include "kis_canvas2.h"

KisResourceProvider::KisResourceProvider(KisView2 * view )
    : m_view( view )
    , m_resourceProvider( view->canvasBase()->resourceProvider() )
{
    QVariant v;
    v.setValue( KoColor(Qt::black, view->image()->colorSpace()) );
    m_resourceProvider->setResource( KoCanvasResource::ForegroundColor, v );

    v.setValue( KoColor(Qt::white, view->image()->colorSpace()) );
    m_resourceProvider->setResource( KoCanvasResource::BackgroundColor, v );

    m_resourceProvider->setKoID(KoCanvasResource::CurrentPaintop, KoID( "paintbrush" ) );
    v = qVariantFromValue( ( void * ) 0 );
    m_resourceProvider->setResource( KoCanvasResource::CurrentPaintopSettings, v );

    // Create a big default brush. XXX: We really need to have a way
    // to get at the loaded brushes, gradients etc. The data structure
    // is now completely hidden behind the gui in really, really old
    // code.
    QImage img( 100, 100, QImage::Format_ARGB32 );
    QPainter p( &img );
    p.setRenderHint( QPainter::Antialiasing );
    p.fillRect( 0, 0, 100, 100, QBrush(QColor( 255, 255, 255, 0) ) );
    p.setBrush( QBrush( QColor( 0, 0, 0, 255 ) ) );
    p.drawEllipse( 0, 0, 100, 100 );
    p.end();

    m_defaultBrush = new KisBrush( img );
    v = qVariantFromValue( static_cast<void *>( m_defaultBrush ) );
    m_resourceProvider->setResource( KoCanvasResource::CurrentBrush, v );

}

KisResourceProvider::~KisResourceProvider()
{
    delete m_defaultBrush;
}


KoCanvasBase * KisResourceProvider::canvas() const
{
    return m_view->canvasBase();
}

KoColor KisResourceProvider::bgColor() const
{
    return m_resourceProvider->resource( KoCanvasResource::ForegroundColor ).value<KoColor>();
}

KoColor KisResourceProvider::fgColor() const
{
    return m_resourceProvider->resource( KoCanvasResource::BackgroundColor ).value<KoColor>();
}

float KisResourceProvider::HDRExposure() const
{
    return static_cast<float>( m_resourceProvider->resource( KoCanvasResource::HdrExposure ).toDouble() );
}

void KisResourceProvider::setHDRExposure(float exposure)

{
    m_resourceProvider->setResource( KoCanvasResource::HdrExposure, static_cast<double>( exposure ) );
}


KisBrush * KisResourceProvider::currentBrush() const
{
    return static_cast<KisBrush *>( m_resourceProvider->resource( KoCanvasResource::CurrentBrush ).value<void *>() );
}


KisPattern * KisResourceProvider::currentPattern() const
{
    return static_cast<KisPattern*>( m_resourceProvider->resource( KoCanvasResource::CurrentPattern ).value<void *>() );
}


KisGradient * KisResourceProvider::currentGradient() const
{
    return static_cast<KisGradient*>( m_resourceProvider->resource( KoCanvasResource::CurrentGradient ).value<void *>() );
}


KoID KisResourceProvider::currentPaintop() const
{
    return m_resourceProvider->resource( KoCanvasResource::CurrentPaintop ).value<KoID>();
}


const KisPaintOpSettings * KisResourceProvider::currentPaintopSettings() const
{
    return static_cast<KisPaintOpSettings*>( m_resourceProvider->resource( KoCanvasResource::CurrentPaintopSettings )
                                             .value<void *>() );
}

KisLayerSP KisResourceProvider::currentLayer() const
{
    return m_resourceProvider->resource( KoCanvasResource::CurrentKritaLayer ).value<KisLayerSP>();
}

void KisResourceProvider::slotBrushActivated(KisResource *res)
{

    KisBrush * brush = dynamic_cast<KisBrush*>(res);
    QVariant v = qVariantFromValue( ( void * ) brush );
    m_resourceProvider->setResource( KoCanvasResource::CurrentBrush, v );
    if (brush )
    {
        emit sigBrushChanged(brush);
    }
}

void KisResourceProvider::slotPatternActivated(KisResource * res)
{
    KisPattern * pattern = dynamic_cast<KisPattern*>(res);
    QVariant v = qVariantFromValue( ( void * ) pattern );
    m_resourceProvider->setResource( KoCanvasResource::CurrentPattern, v );
    if (pattern) {
        emit sigPatternChanged(pattern);
    }
}

void KisResourceProvider::slotGradientActivated(KisResource *res)
{

    KisGradient * gradient = dynamic_cast<KisGradient*>(res);
    QVariant v = qVariantFromValue( ( void * ) gradient );
    m_resourceProvider->setResource( KoCanvasResource::CurrentGradient, v );
    if (gradient) {
        emit sigGradientChanged(gradient);
    }
}

void KisResourceProvider::slotPaintopActivated(const KoID & paintop,
                                           const KisPaintOpSettings *paintopSettings)
{
    if (paintop.id().isNull() || paintop.id().isEmpty()) {
        return;
    }

    QVariant  v;
    v.setValue( paintop );
    m_resourceProvider->setResource( KoCanvasResource::CurrentPaintop, v );

    v = qVariantFromValue( ( void * ) paintopSettings );
    m_resourceProvider->setResource( KoCanvasResource::CurrentPaintopSettings, v );

    emit sigPaintopChanged(paintop, paintopSettings);
}

void KisResourceProvider::setBGColor(const KoColor& c)
{

    QVariant v;
    v.setValue( c );
    m_resourceProvider->setResource( KoCanvasResource::BackgroundColor, v );
    emit sigBGColorChanged( c );
}

void KisResourceProvider::setFGColor(const KoColor& c)
{
    QVariant v;
    v.setValue( c );
    m_resourceProvider->setResource( KoCanvasResource::ForegroundColor, v );
    emit sigFGColorChanged( c );
}

void KisResourceProvider::slotSetFGColor(const KoColor& c)
{
    setFGColor( c );
}

void KisResourceProvider::slotSetBGColor(const KoColor& c)
{
    setBGColor( c );
}

void KisResourceProvider::slotLayerActivated( const KisLayerSP l )
{
    QVariant v;
    v.setValue( l );
    m_resourceProvider->setResource( KoCanvasResource::CurrentPaintop, v );

}


void KisResourceProvider::slotResourceChanged( KoCanvasResource::EnumCanvasResource key, const QVariant & res )
{
    switch ( key ) {
    case ( KoCanvasResource::ForegroundColor ):
        emit sigFGColorChanged( res.value<KoColor>() );
        break;
    case ( KoCanvasResource::BackgroundColor ):
        emit sigBGColorChanged( res.value<KoColor>() );
        break;
    case ( KoCanvasResource::CurrentBrush ):
        emit sigBrushChanged( static_cast<KisBrush *>( res.value<void *>() ) );
        break;
    case ( KoCanvasResource::CurrentPattern ):
        emit sigPatternChanged( static_cast<KisPattern *>( res.value<void *>() ) );
        break;
    case ( KoCanvasResource::CurrentGradient ):
        emit sigGradientChanged( static_cast<KisGradient *>( res.value<void *>() ) );
        break;
    case ( KoCanvasResource::CurrentPaintop ):
        emit sigPaintopChanged(res.value<KoID >(), currentPaintopSettings());
        break;
    case ( KoCanvasResource::CurrentPaintopSettings ):
        emit sigPaintopChanged(currentPaintop(), currentPaintopSettings() );
        break;
    default:
        ;
        // Do nothing
    };
}

#include "kis_resource_provider.moc"
