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
#include <KoColorModelStandardIds.h>
#include <KoColorProfile.h>
#include <KoAbstractGradient.h>
#include <KoCompositeOp.h>
#include <KoResourceServerProvider.h>
#include <KoStopGradient.h>

#include <kis_pattern.h>
#include <kis_paint_device.h>
#include <filter/kis_filter_configuration.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_paintop_preset.h>
#include <kis_paintop_settings.h>
#include "ko_favorite_resource_manager.h"

#include "kis_config.h"
#include "kis_view2.h"
#include "canvas/kis_canvas2.h"

KisCanvasResourceProvider::KisCanvasResourceProvider(KisView2 * view)
        : m_view(view)
        , m_displayProfile(0)
{
    m_fGChanged = true;
    m_enablefGChange = true;    // default to true, so that colour history is working without popup palette
}

KisCanvasResourceProvider::~KisCanvasResourceProvider()
{
    disconnect(); // in case Qt gets confused
}

KoCanvasResourceManager* KisCanvasResourceProvider::resourceManager()
{
    return m_resourceManager;
}

void KisCanvasResourceProvider::setResourceManager(KoCanvasResourceManager *resourceManager)
{
    m_resourceManager = resourceManager;

    QVariant v;
    v.setValue(KoColor(Qt::black, m_view->image()->colorSpace()));
    m_resourceManager->setResource(KoCanvasResourceManager::ForegroundColor, v);

    v.setValue(KoColor(Qt::white, m_view->image()->colorSpace()));
    m_resourceManager->setResource(KoCanvasResourceManager::BackgroundColor, v);

    setCurrentCompositeOp(COMPOSITE_OVER);

    setMirrorHorizontal(false);
    setMirrorVertical(false);

    m_resourceManager->setResource(HdrExposure, 0.0);
    m_resourceManager->setResource(HdrGamma, 1.0);

    connect(m_resourceManager, SIGNAL(resourceChanged(int, const QVariant &)),
            this, SLOT(slotResourceChanged(int, const QVariant&)));
}


KoCanvasBase * KisCanvasResourceProvider::canvas() const
{
    return m_view->canvasBase();
}

KoColor KisCanvasResourceProvider::bgColor() const
{
    return m_resourceManager->resource(KoCanvasResourceManager::BackgroundColor).value<KoColor>();
}

KoColor KisCanvasResourceProvider::fgColor() const
{
    return m_resourceManager->resource(KoCanvasResourceManager::ForegroundColor).value<KoColor>();
}

float KisCanvasResourceProvider::HDRExposure() const
{
    return static_cast<float>(m_resourceManager->resource(HdrExposure).toDouble());
}

void KisCanvasResourceProvider::setHDRExposure(float exposure)
{
    m_resourceManager->setResource(HdrExposure, static_cast<double>(exposure));
}

float KisCanvasResourceProvider::HDRGamma() const
{
    return static_cast<float>(m_resourceManager->resource(HdrGamma).toDouble());
}

void KisCanvasResourceProvider::setHDRGamma(float gamma)
{
    m_resourceManager->setResource(HdrGamma, static_cast<double>(gamma));
}


KisPattern * KisCanvasResourceProvider::currentPattern() const
{
    return static_cast<KisPattern*>(m_resourceManager->resource(CurrentPattern).value<void *>());
}

KisFilterConfiguration * KisCanvasResourceProvider::currentGeneratorConfiguration() const
{
    return static_cast<KisFilterConfiguration*>(m_resourceManager->
            resource(CurrentGeneratorConfiguration).value<void *>());
}


KoAbstractGradient* KisCanvasResourceProvider::currentGradient() const
{
    return static_cast<KoAbstractGradient*>(m_resourceManager->resource(CurrentGradient).value<void *>());
}


void KisCanvasResourceProvider::resetDisplayProfile(int screen)
{
    KisConfig cfg;
    m_displayProfile = cfg.displayProfile(screen);
    emit sigDisplayProfileChanged(m_displayProfile);
}

const KoColorProfile * KisCanvasResourceProvider::currentDisplayProfile() const
{
    return m_displayProfile;
}

KisImageWSP KisCanvasResourceProvider::currentImage() const
{
    return m_view->image();
}

KisNodeSP KisCanvasResourceProvider::currentNode() const
{
    return m_view->activeNode();
}

KisPaintOpPresetSP KisCanvasResourceProvider::currentPreset() const
{
    KisPaintOpPresetSP preset = m_resourceManager->resource(CurrentPaintOpPreset).value<KisPaintOpPresetSP>();
    return preset;
}


void KisCanvasResourceProvider::slotPatternActivated(KoResource * res)
{
    KisPattern * pattern = dynamic_cast<KisPattern*>(res);
    QVariant v = qVariantFromValue((void *) pattern);
    m_resourceManager->setResource(CurrentPattern, v);
    emit sigPatternChanged(pattern);
}

void KisCanvasResourceProvider::slotGeneratorConfigurationActivated(KisFilterConfiguration * res)
{
    KisFilterConfiguration * generatorConfiguration = dynamic_cast<KisFilterConfiguration*>(res);
    QVariant v = qVariantFromValue((void *) generatorConfiguration);
    m_resourceManager->setResource(CurrentGeneratorConfiguration, v);
    emit sigGeneratorConfigurationChanged(generatorConfiguration);
}

void KisCanvasResourceProvider::slotGradientActivated(KoResource *res)
{

    KoAbstractGradient * gradient = dynamic_cast<KoAbstractGradient*>(res);
    QVariant v = qVariantFromValue((void *) gradient);
    m_resourceManager->setResource(CurrentGradient, v);
    emit sigGradientChanged(gradient);
}

void KisCanvasResourceProvider::setPaintOpPreset(const KisPaintOpPresetSP preset)
{
    Q_ASSERT(preset->valid());
    Q_ASSERT(!preset->paintOp().id().isEmpty());
    Q_ASSERT(preset->settings());
    if (!preset) return;

    dbgUI << "setPaintOpPreset" << preset->paintOp();

    QVariant v;
    v.setValue(preset);
    m_resourceManager->setResource(CurrentPaintOpPreset, v);
    emit sigPaintOpPresetChanged(preset);
}

void KisCanvasResourceProvider::setBGColor(const KoColor& c)
{

    QVariant v;
    v.setValue(c);
    m_resourceManager->setResource(KoCanvasResourceManager::BackgroundColor, v);
    emit sigBGColorChanged(c);
}

void KisCanvasResourceProvider::setFGColor(const KoColor& c)
{
    m_fGChanged = true;

    QVariant v;
    v.setValue(c);
    m_resourceManager->setResource(KoCanvasResourceManager::ForegroundColor, v);
    emit sigFGColorChanged(c);
}

void KisCanvasResourceProvider::slotSetFGColor(const KoColor& c)
{
    setFGColor(c);
}

void KisCanvasResourceProvider::slotSetBGColor(const KoColor& c)
{
    setBGColor(c);
}

void KisCanvasResourceProvider::slotNodeActivated(const KisNodeSP node)
{
    QVariant v;
    v.setValue(node);
    m_resourceManager->setResource(CurrentKritaNode, v);
    emit sigNodeChanged(currentNode());
}


void KisCanvasResourceProvider::slotImageSizeChanged()
{
    if (KisImageWSP image = m_view->image()) {
        float fw = image->width() / image->xRes();
        float fh = image->height() / image->yRes();

        QSizeF postscriptSize(fw, fh);
        m_resourceManager->setResource(KoCanvasResourceManager::PageSize, postscriptSize);
    }
}

void KisCanvasResourceProvider::slotSetDisplayProfile(const KoColorProfile * profile)
{
    m_displayProfile = const_cast<KoColorProfile*>(profile);
    emit sigDisplayProfileChanged(profile);
}

void KisCanvasResourceProvider::slotOnScreenResolutionChanged()
{
    KisImageWSP image = m_view->image();
    KisCanvas2 *canvas = m_view->canvasBase();

    if(!image || !canvas) return;

    qreal zoomX, zoomY;
    canvas->coordinatesConverter()->zoom(&zoomX, &zoomY);

    qreal scaleX = zoomX / image->xRes();
    qreal scaleY = zoomY / image->yRes();

    emit sigOnScreenResolutionChanged(scaleX, scaleY);
}

void KisCanvasResourceProvider::slotResourceChanged(int key, const QVariant & res)
{
    if(key == KoCanvasResourceManager::ForegroundColor || key == KoCanvasResourceManager::BackgroundColor) {
        KoAbstractGradient* resource = KoResourceServerProvider::instance()->gradientServer()->resources()[0];
        KoStopGradient* stopGradient = dynamic_cast<KoStopGradient*>(resource);
        if(stopGradient) {
            QList<KoGradientStop> stops;
            stops << KoGradientStop(0.0, fgColor()) << KoGradientStop(1.0, bgColor());
            stopGradient->setStops(stops);
            KoResourceServerProvider::instance()->gradientServer()->updateResource(resource);
        }
        resource = KoResourceServerProvider::instance()->gradientServer()->resources()[1];
        stopGradient = dynamic_cast<KoStopGradient*>(resource);
        if(stopGradient) {
            QList<KoGradientStop> stops;
            stops << KoGradientStop(0.0, fgColor()) << KoGradientStop(1.0,  KoColor(QColor(0, 0, 0, 0), fgColor().colorSpace()));
            stopGradient->setStops(stops);
            KoResourceServerProvider::instance()->gradientServer()->updateResource(resource);
        }
    }
    switch (key) {
    case(KoCanvasResourceManager::ForegroundColor):
        m_fGChanged = true;
        emit sigFGColorChanged(res.value<KoColor>());
        break;
    case(KoCanvasResourceManager::BackgroundColor):
        emit sigBGColorChanged(res.value<KoColor>());
        break;
    case(CurrentPattern):
        emit sigPatternChanged(static_cast<KisPattern *>(res.value<void *>()));
        break;
    case(CurrentGeneratorConfiguration):
        emit sigGeneratorConfigurationChanged(static_cast<KisFilterConfiguration*>(res.value<void*>()));
    case(CurrentGradient):
        emit sigGradientChanged(static_cast<KoAbstractGradient *>(res.value<void *>()));
        break;
    case(CurrentPaintOpPreset):
        emit sigPaintOpPresetChanged(currentPreset());
        break;
    case(CurrentKritaNode) :
        emit sigNodeChanged(currentNode());
        break;
    case(CurrentCompositeOp) :
        emit sigCompositeOpChanged(currentCompositeOp());
        break;
    case (Opacity):
    {
        emit sigOpacityChanged(res.toDouble());
    }
    default:
        ;
        // Do nothing
    };
}

void KisCanvasResourceProvider::setCurrentCompositeOp(const QString& compositeOp)
{
    QVariant v;
    v.setValue(compositeOp);
    m_resourceManager->setResource(CurrentCompositeOp, v);
    emit sigCompositeOpChanged(compositeOp);
}

QString KisCanvasResourceProvider::currentCompositeOp() const
{
    return m_resourceManager->resource(CurrentCompositeOp).value<QString>();
}

void KisCanvasResourceProvider::slotPainting()
{
    if (m_fGChanged && m_enablefGChange) {
        emit sigFGColorUsed(fgColor());
        m_fGChanged = false;
    }
}

void KisCanvasResourceProvider::slotResetEnableFGChange(bool b)
{
    m_enablefGChange = b;
}

QList<KisAbstractPerspectiveGrid*> KisCanvasResourceProvider::perspectiveGrids() const
{
    return m_perspectiveGrids;
}

void KisCanvasResourceProvider::addPerspectiveGrid(KisAbstractPerspectiveGrid* grid)
{
    m_perspectiveGrids.append(grid);
}

void KisCanvasResourceProvider::removePerspectiveGrid(KisAbstractPerspectiveGrid* grid)
{
    m_perspectiveGrids.removeOne(grid);
}

void KisCanvasResourceProvider::clearPerspectiveGrids()
{
    m_perspectiveGrids.clear();
}

void KisCanvasResourceProvider::setMirrorHorizontal(bool mirrorHorizontal)
{
    m_resourceManager->setResource(MirrorHorizontal, mirrorHorizontal);
}

bool KisCanvasResourceProvider::mirrorHorizontal() const
{
    return m_resourceManager->resource(MirrorHorizontal).toBool();
}

void KisCanvasResourceProvider::setMirrorVertical(bool mirrorVertical)
{
    m_resourceManager->setResource(MirrorVertical, mirrorVertical);
}

bool KisCanvasResourceProvider::mirrorVertical() const
{
    return m_resourceManager->resource(MirrorVertical).toBool();
}

void KisCanvasResourceProvider::setOpacity(qreal opacity)
{
    m_resourceManager->setResource(Opacity, opacity);
}

qreal KisCanvasResourceProvider::opacity()
{
    return m_resourceManager->resource(Opacity).toDouble();
}

void KisCanvasResourceProvider::notifyLoadingWorkspace(KisWorkspaceResource* workspace)
{
    emit sigLoadingWorkspace(workspace);
}

void KisCanvasResourceProvider::notifySavingWorkspace(KisWorkspaceResource* workspace)
{
    emit sigSavingWorkspace(workspace);
}

#include "kis_canvas_resource_provider.moc"
