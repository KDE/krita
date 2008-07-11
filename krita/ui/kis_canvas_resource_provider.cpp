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

#include "kis_canvas_resource_provider.h"
#include <QImage>
#include <QPainter>

#include <KoCanvasBase.h>
#include <KoID.h>
#include <KoColorSpaceRegistry.h>

#include "colorprofiles/KoIccColorProfile.h"

#include <KoAbstractGradient.h>
#include <kis_brush.h>
#include <kis_pattern.h>
#include <kis_paint_device.h>
#include <filter/kis_filter_configuration.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_paintop_preset.h>
#include "kis_exposure_visitor.h"
#include "kis_config.h"
#include "kis_view2.h"
#include "canvas/kis_canvas2.h"

KisCanvasResourceProvider::KisCanvasResourceProvider(KisView2 * view )
    : m_view( view )
{
}

KisCanvasResourceProvider::~KisCanvasResourceProvider()
{
    delete m_defaultBrush;
}

void KisCanvasResourceProvider::setCanvasResourceProvider( KoCanvasResourceProvider * resourceProvider )
{
    m_resourceProvider = resourceProvider;

    QVariant v;
    v.setValue( KoColor(Qt::black, m_view->image()->colorSpace()) );
    m_resourceProvider->setResource( KoCanvasResource::ForegroundColor, v );

    v.setValue( KoColor(Qt::white, m_view->image()->colorSpace()) );
    m_resourceProvider->setResource( KoCanvasResource::BackgroundColor, v );

    m_resourceProvider->setResource(CurrentPaintop, KoID( "paintbrush", "Paintbrush" ) );
    v = qVariantFromValue( ( void * ) 0 );
    m_resourceProvider->setResource( CurrentPaintopSettings, v );

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
    m_resourceProvider->setResource( CurrentBrush, v );

    resetDisplayProfile();

    connect(m_resourceProvider, SIGNAL(resourceChanged(int, const QVariant &)),
            this, SLOT(slotResourceChanged(int, const QVariant&)));

}


KoCanvasBase * KisCanvasResourceProvider::canvas() const
{
    return m_view->canvasBase();
}

KoColor KisCanvasResourceProvider::bgColor() const
{
    return m_resourceProvider->resource( KoCanvasResource::BackgroundColor ).value<KoColor>();
}

KoColor KisCanvasResourceProvider::fgColor() const
{
    return m_resourceProvider->resource( KoCanvasResource::ForegroundColor ).value<KoColor>();
}

float KisCanvasResourceProvider::HDRExposure() const
{
    return static_cast<float>( m_resourceProvider->resource( HdrExposure ).toDouble() );
}

void KisCanvasResourceProvider::setHDRExposure(float exposure)
{
    m_resourceProvider->setResource( HdrExposure, static_cast<double>( exposure ) );
    KisExposureVisitor eV(exposure);
    m_view->image()->projection()->colorSpace()->profile()->setProperty("exposure", exposure);
    m_view->image()->rootLayer()->accept( eV );
    m_view->canvasBase()->updateCanvas();
    m_view->canvasBase()->updateCanvasProjection( m_view->image()->bounds() );
}


KisBrush * KisCanvasResourceProvider::currentBrush() const
{
    return static_cast<KisBrush *>( m_resourceProvider->resource( CurrentBrush ).value<void *>() );
}


KisPattern * KisCanvasResourceProvider::currentPattern() const
{
    return static_cast<KisPattern*>( m_resourceProvider->resource( CurrentPattern ).value<void *>() );
}

KisFilterConfiguration * KisCanvasResourceProvider::currentGeneratorConfiguration() const
{
    return static_cast<KisFilterConfiguration*>( m_resourceProvider->
                                                    resource( CurrentGeneratorConfiguration ).value<void *>() );
}


KoAbstractGradient* KisCanvasResourceProvider::currentGradient() const
{
    return static_cast<KoAbstractGradient*>( m_resourceProvider->resource( CurrentGradient ).value<void *>() );
}


KoID KisCanvasResourceProvider::currentPaintop() const
{
    return m_resourceProvider->resource( CurrentPaintop ).value<KoID>();
}


const KisPaintOpSettingsSP KisCanvasResourceProvider::currentPaintopSettings() const
{
    return static_cast<KisPaintOpSettings*>( m_resourceProvider->resource( CurrentPaintopSettings )
                                             .value<void *>() );
}

void KisCanvasResourceProvider::resetDisplayProfile()
{
    // XXX: The X11 monitor profile overrides the settings
    m_displayProfile = KoIccColorProfile::getScreenProfile();

    if (m_displayProfile == 0) {
        KisConfig cfg;
        QString monitorProfileName = cfg.monitorProfile();
        m_displayProfile = KoColorSpaceRegistry::instance()->profileByName(monitorProfileName);
    }
    emit sigDisplayProfileChanged( m_displayProfile );
}

const KoColorProfile * KisCanvasResourceProvider::currentDisplayProfile() const
{
    return m_displayProfile;

}

KisImageSP KisCanvasResourceProvider::currentImage() const
{
    return m_view->image();
}

KisNodeSP KisCanvasResourceProvider::currentNode() const
{
    return m_view->activeNode();
}

KisPaintOpPresetSP KisCanvasResourceProvider::currentPreset() const
{
    return m_resourceProvider->resource( CurrentPaintOpPreset ).value<KisPaintOpPresetSP>();
}


void KisCanvasResourceProvider::slotBrushActivated(KoResource *res)
{

    KisBrush * brush = dynamic_cast<KisBrush*>(res);
    QVariant v = qVariantFromValue( ( void * ) brush );
    m_resourceProvider->setResource( CurrentBrush, v );
    emit sigBrushChanged(brush);
}

void KisCanvasResourceProvider::slotPatternActivated(KoResource * res)
{
    KisPattern * pattern = dynamic_cast<KisPattern*>(res);
    QVariant v = qVariantFromValue( ( void * ) pattern );
    m_resourceProvider->setResource( CurrentPattern, v );
    emit sigPatternChanged(pattern);
}

void KisCanvasResourceProvider::slotGeneratorConfigurationActivated(KisFilterConfiguration * res)
{
    KisFilterConfiguration * generatorConfiguration = dynamic_cast<KisFilterConfiguration*>(res);
    QVariant v = qVariantFromValue( ( void * ) generatorConfiguration  );
    m_resourceProvider->setResource( CurrentGeneratorConfiguration, v );
    emit sigGeneratorConfigurationChanged(generatorConfiguration);
}

void KisCanvasResourceProvider::slotGradientActivated(KoResource *res)
{

    KoAbstractGradient * gradient = dynamic_cast<KoAbstractGradient*>(res);
    QVariant v = qVariantFromValue( ( void * ) gradient );
    m_resourceProvider->setResource( CurrentGradient, v );
    emit sigGradientChanged(gradient);
}

void KisCanvasResourceProvider::slotPaintopActivated(const KoID & paintop,
                                                     const KisPaintOpSettingsSP paintopSettings)
{
    if (paintop.id().isNull() || paintop.id().isEmpty()) {
        return;
    }

    QVariant  v;
    v.setValue( paintop );
    m_resourceProvider->setResource( CurrentPaintop, v );

    v = qVariantFromValue( ( void * ) paintopSettings.data() );
    m_resourceProvider->setResource( CurrentPaintopSettings, v );

    emit sigPaintopChanged(paintop, paintopSettings);
}

void KisCanvasResourceProvider::slotPaintOpPresetActivated( const KisPaintOpPresetSP preset )
{
    if (!preset) return;
    QVariant v;
    v.setValue(preset);
    m_resourceProvider->setResource( CurrentPaintOpPreset, v );
    emit sigPaintOpPresetChanged( preset );
}

void KisCanvasResourceProvider::setBGColor(const KoColor& c)
{

    QVariant v;
    v.setValue( c );
    m_resourceProvider->setResource( KoCanvasResource::BackgroundColor, v );
    emit sigBGColorChanged( c );
}

void KisCanvasResourceProvider::setFGColor(const KoColor& c)
{
    QVariant v;
    v.setValue( c );
    m_resourceProvider->setResource( KoCanvasResource::ForegroundColor, v );
    emit sigFGColorChanged( c );
}

void KisCanvasResourceProvider::slotSetFGColor(const KoColor& c)
{
    setFGColor( c );
}

void KisCanvasResourceProvider::slotSetBGColor(const KoColor& c)
{
    setBGColor( c );
}

void KisCanvasResourceProvider::slotNodeActivated( const KisNodeSP node )
{
    if (node)
        dbgUI << " node activated: " << node->name();
    else
        dbgUI << " null node activated";

    QVariant v;
    v.setValue( node );
    m_resourceProvider->setResource( CurrentKritaNode, v );
    emit sigNodeChanged( currentNode() );
}


void KisCanvasResourceProvider::slotSetImageSize( qint32 w, qint32 h )
{
    if ( KisImageSP image = m_view->image() ) {
        float fw = w / image->xRes();
        float fh = h / image->yRes();

        QSizeF postscriptSize( fw, fh );
        m_resourceProvider->setResource( KoCanvasResource::PageSize, postscriptSize );
    }
}

void KisCanvasResourceProvider::slotSetDisplayProfile( const KoColorProfile * profile )
{
    m_displayProfile = const_cast<KoColorProfile*>(profile);
    emit sigDisplayProfileChanged( profile );
}

void KisCanvasResourceProvider::slotResourceChanged( int key, const QVariant & res )
{
    switch ( key ) {
    case ( KoCanvasResource::ForegroundColor ):
        emit sigFGColorChanged( res.value<KoColor>() );
        break;
    case ( KoCanvasResource::BackgroundColor ):
        emit sigBGColorChanged( res.value<KoColor>() );
        break;
    case ( CurrentBrush ):
        emit sigBrushChanged( static_cast<KisBrush *>( res.value<void *>() ) );
        break;
    case ( CurrentPattern ):
        emit sigPatternChanged( static_cast<KisPattern *>( res.value<void *>() ) );
        break;
    case ( CurrentGeneratorConfiguration ):
        emit sigGeneratorConfigurationChanged(static_cast<KisFilterConfiguration*>(res.value<void*>()));
    case ( CurrentGradient ):
        emit sigGradientChanged( static_cast<KoAbstractGradient *>( res.value<void *>() ) );
        break;
    case ( CurrentPaintop ):
        emit sigPaintopChanged(res.value<KoID >(), currentPaintopSettings());
        break;
    case ( CurrentPaintopSettings ):
        emit sigPaintopChanged(currentPaintop(), currentPaintopSettings() );
        break;
    case ( CurrentKritaNode ) :
        emit sigNodeChanged( currentNode() );
    default:
        ;
        // Do nothing
    };
}

#include "kis_canvas_resource_provider.moc"
