/*
 *  SPDX-FileCopyrightText: 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_canvas_resource_provider.h"

#include <QImage>
#include <QPainter>

#include <KoCanvasBase.h>
#include <KoID.h>
#include <KoColorModelStandardIds.h>
#include <KoColorProfile.h>
#include <resources/KoAbstractGradient.h>
#include <KoCompositeOpRegistry.h>
#include <KoResourceServerProvider.h>
#include <resources/KoStopGradient.h>
#include <KoColorSpaceRegistry.h>

#include <resources/KoPattern.h>
#include <kis_paint_device.h>
#include <filter/kis_filter_configuration.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <brushengine/kis_paintop_preset.h>
#include <brushengine/kis_paintop_settings.h>
#include "kis_favorite_resource_manager.h"

#include "kis_config.h"
#include "KisViewManager.h"
#include "canvas/kis_canvas2.h"

KisCanvasResourceProvider::KisCanvasResourceProvider(KisViewManager * view)
    : m_view(view)
{
    m_fGChanged = true;
}

KisCanvasResourceProvider::~KisCanvasResourceProvider()
{
    disconnect(); // in case Qt gets confused
}

KoCanvasResourceProvider* KisCanvasResourceProvider::resourceManager()
{
    return m_resourceManager;
}

void KisCanvasResourceProvider::setResourceManager(KoCanvasResourceProvider *resourceManager)
{
    m_resourceManager = resourceManager;

    QVariant v;
    v.setValue(KoColor(Qt::black, KoColorSpaceRegistry::instance()->rgb8()));
    m_resourceManager->setResource(KoCanvasResource::ForegroundColor, v);

    v.setValue(KoColor(Qt::white, KoColorSpaceRegistry::instance()->rgb8()));
    m_resourceManager->setResource(KoCanvasResource::BackgroundColor, v);

    setCurrentCompositeOp(COMPOSITE_OVER);

    setMirrorHorizontal(false);
    setMirrorVertical(false);

    m_resourceManager->setResource(KoCanvasResource::HdrExposure, 0.0);
    m_resourceManager->setResource(KoCanvasResource::HdrGamma, 1.0);
    m_resourceManager->setResource(KoCanvasResource::EffectiveZoom, 1.0);

    connect(m_resourceManager, SIGNAL(canvasResourceChanged(int,QVariant)),
            this, SLOT(slotCanvasResourceChanged(int,QVariant)));

    m_resourceManager->setResource(KoCanvasResource::GamutMaskActive, false);
}


KoCanvasBase * KisCanvasResourceProvider::canvas() const
{
    return m_view->canvasBase();
}

KoColor KisCanvasResourceProvider::bgColor() const
{
    QVariant c = m_resourceManager->resource(KoCanvasResource::BackgroundColor);
    if (c.isValid()) {
        return c.value<KoColor>();
    }
    else {
        return KoColor();
    }
}

KoColor KisCanvasResourceProvider::fgColor() const
{
    QVariant c = m_resourceManager->resource(KoCanvasResource::ForegroundColor);
    if (c.isValid()) {
        return c.value<KoColor>();
    }
    else {
        return KoColor();
    }
}

float KisCanvasResourceProvider::HDRExposure() const
{
    return static_cast<float>(m_resourceManager->resource(KoCanvasResource::HdrExposure).toDouble());
}

void KisCanvasResourceProvider::setHDRExposure(float exposure)
{
    m_resourceManager->setResource(KoCanvasResource::HdrExposure, static_cast<double>(exposure));
}

float KisCanvasResourceProvider::HDRGamma() const
{
    return static_cast<float>(m_resourceManager->resource(KoCanvasResource::HdrGamma).toDouble());
}

void KisCanvasResourceProvider::setHDRGamma(float gamma)
{
    m_resourceManager->setResource(KoCanvasResource::HdrGamma, static_cast<double>(gamma));
}


KoPatternSP KisCanvasResourceProvider::currentPattern() const
{
    if (m_resourceManager->hasResource(KoCanvasResource::CurrentPattern)) {
        return m_resourceManager->resource(KoCanvasResource::CurrentPattern).value<KoPatternSP>();
    }
    else {
        return 0;
    }
}

KoAbstractGradientSP KisCanvasResourceProvider::currentGradient() const
{
    if (m_resourceManager->hasResource(KoCanvasResource::CurrentGradient)) {
        return m_resourceManager->resource(KoCanvasResource::CurrentGradient).value<KoAbstractGradientSP>();
    }
    else {
        return 0;
    }
}

KisImageWSP KisCanvasResourceProvider::currentImage() const
{
    return m_view->image();
}

KisNodeSP KisCanvasResourceProvider::currentNode() const
{
    return m_view->activeNode();
}

KoGamutMaskSP KisCanvasResourceProvider::currentGamutMask() const
{
    if (m_resourceManager->hasResource(KoCanvasResource::CurrentGamutMask)) {
        return m_resourceManager->resource(KoCanvasResource::CurrentGamutMask).value<KoGamutMaskSP>();
    }
    else {
        return nullptr;
    }
}

bool KisCanvasResourceProvider::gamutMaskActive() const
{
    return m_resourceManager->resource(KoCanvasResource::GamutMaskActive).toBool();
}

KisPaintOpPresetSP KisCanvasResourceProvider::currentPreset() const
{
    KisPaintOpPresetSP preset = m_resourceManager->resource(KoCanvasResource::CurrentPaintOpPreset).value<KisPaintOpPresetSP>();
    return preset;
}

void KisCanvasResourceProvider::setPaintOpPreset(const KisPaintOpPresetSP preset)
{
    if (!preset) return;

    Q_ASSERT(preset->valid());
    Q_ASSERT(!preset->paintOp().id().isEmpty());
    Q_ASSERT(preset->settings());

    QVariant v;
    v.setValue(preset);
    m_resourceManager->setResource(KoCanvasResource::CurrentPaintOpPreset, v);
}

KisPaintOpPresetSP KisCanvasResourceProvider::previousPreset() const
{
    KisPaintOpPresetSP preset = m_resourceManager->resource(KoCanvasResource::PreviousPaintOpPreset).value<KisPaintOpPresetSP>();
    return preset;
}

void KisCanvasResourceProvider::setPreviousPaintOpPreset(const KisPaintOpPresetSP preset)
{
    Q_ASSERT(preset->valid());
    Q_ASSERT(!preset->paintOp().id().isEmpty());
    Q_ASSERT(preset->settings());
    if (!preset) return;

    dbgUI << "setPreviousPaintOpPreset" << preset->paintOp();

    QVariant v;
    v.setValue(preset);
    m_resourceManager->setResource(KoCanvasResource::PreviousPaintOpPreset, v);
}

void KisCanvasResourceProvider::slotPatternActivated(KoResourceSP res)
{
    KoPatternSP pattern = res.dynamicCast<KoPattern>();
    QVariant v;
    v.setValue<KoPatternSP>(pattern);
    m_resourceManager->setResource(KoCanvasResource::CurrentPattern, v);
    emit sigPatternChanged(pattern);
}

void KisCanvasResourceProvider::slotGradientActivated(KoResourceSP res)
{

    KoAbstractGradientSP gradient = res.dynamicCast<KoAbstractGradient>();
    QVariant v;
    v.setValue<KoAbstractGradientSP>(gradient);
    m_resourceManager->setResource(KoCanvasResource::CurrentGradient, v);
    emit sigGradientChanged(gradient);
}


void KisCanvasResourceProvider::setBGColor(const KoColor& c)
{
    QVariant v;
    v.setValue(c);
    m_resourceManager->setResource(KoCanvasResource::BackgroundColor, v);
    emit sigBGColorChanged(c);
}

void KisCanvasResourceProvider::setFGColor(const KoColor& c)
{
    m_fGChanged = true;

    QVariant v;
    v.setValue(c);
    m_resourceManager->setResource(KoCanvasResource::ForegroundColor, v);
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
    v.setValue(KisNodeWSP(node));
    m_resourceManager->setResource(KoCanvasResource::CurrentKritaNode, v);
    emit sigNodeChanged(currentNode());
}


void KisCanvasResourceProvider::slotImageSizeChanged()
{
    if (KisImageWSP image = m_view->image()) {
        float fw = image->width() / image->xRes();
        float fh = image->height() / image->yRes();

        QSizeF postscriptSize(fw, fh);
        m_resourceManager->setResource(KoCanvasResource::PageSize, postscriptSize);
    }
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

void KisCanvasResourceProvider::slotCanvasResourceChanged(int key, const QVariant & res)
{
    switch (key) {
    case(KoCanvasResource::ForegroundColor):
        m_fGChanged = true;
        emit sigFGColorChanged(res.value<KoColor>());
        break;
    case(KoCanvasResource::BackgroundColor):
        emit sigBGColorChanged(res.value<KoColor>());
        break;
    case(KoCanvasResource::CurrentPattern):
        emit sigPatternChanged(res.value<KoPatternSP>());
        break;
    case(KoCanvasResource::CurrentGradient):
        emit sigGradientChanged(res.value<KoAbstractGradientSP>());
        break;
    case(KoCanvasResource::CurrentKritaNode) :
        emit sigNodeChanged(currentNode());
        break;
    case (KoCanvasResource::Opacity):
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
    m_resourceManager->setResource(KoCanvasResource::CurrentCompositeOp,
                                   QVariant::fromValue(compositeOp));
}

QString KisCanvasResourceProvider::currentCompositeOp() const
{
    return m_resourceManager->resource(KoCanvasResource::CurrentCompositeOp).value<QString>();
}

bool KisCanvasResourceProvider::eraserMode() const
{
    return m_resourceManager->resource(KoCanvasResource::EraserMode).toBool();
}

void KisCanvasResourceProvider::setEraserMode(bool value)
{
    m_resourceManager->setResource(KoCanvasResource::EraserMode,
                                   QVariant::fromValue(value));
}

void KisCanvasResourceProvider::slotPainting()
{
    if (m_fGChanged) {
        emit sigFGColorUsed(fgColor());
        m_fGChanged = false;
    }
}

void KisCanvasResourceProvider::slotGamutMaskActivated(KoGamutMaskSP mask)
{
    QVariant v;
    v.setValue<KoGamutMaskSP>(mask);
    m_resourceManager->setResource(KoCanvasResource::CurrentGamutMask, v);

    m_resourceManager->setResource(KoCanvasResource::GamutMaskActive, QVariant::fromValue(true));

    emit sigGamutMaskChanged(mask);
}

void KisCanvasResourceProvider::slotGamutMaskUnset()
{
    m_resourceManager->setResource(KoCanvasResource::GamutMaskActive, QVariant::fromValue(false));
    m_resourceManager->clearResource(KoCanvasResource::CurrentGamutMask);
    emit sigGamutMaskUnset();
}

void KisCanvasResourceProvider::slotGamutMaskPreviewUpdate()
{
    emit sigGamutMaskPreviewUpdate();
}

void KisCanvasResourceProvider::slotGamutMaskDeactivate()
{
    m_resourceManager->setResource(KoCanvasResource::GamutMaskActive, QVariant::fromValue(false));
    emit sigGamutMaskDeactivated();
}

QList<QPointer<KisAbstractPerspectiveGrid> > KisCanvasResourceProvider::perspectiveGrids() const
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
    m_resourceManager->setResource(KoCanvasResource::MirrorHorizontal, mirrorHorizontal);
    emit mirrorModeChanged();
}

bool KisCanvasResourceProvider::mirrorHorizontal() const
{
    return m_resourceManager->resource(KoCanvasResource::MirrorHorizontal).toBool();
}

void KisCanvasResourceProvider::setMirrorVertical(bool mirrorVertical)
{
    m_resourceManager->setResource(KoCanvasResource::MirrorVertical, mirrorVertical);
    emit mirrorModeChanged();
}

bool KisCanvasResourceProvider::mirrorVertical() const
{
    return m_resourceManager->resource(KoCanvasResource::MirrorVertical).toBool();
}

void KisCanvasResourceProvider::setMirrorHorizontalLock(bool isLocked)
{
    m_resourceManager->setResource(KoCanvasResource::MirrorHorizontalLock, isLocked);
    emit mirrorModeChanged();
}

bool KisCanvasResourceProvider::mirrorHorizontalLock() {
     return m_resourceManager->resource(KoCanvasResource::MirrorHorizontalLock).toBool();
}

void KisCanvasResourceProvider::setMirrorVerticalLock(bool isLocked)
{
    m_resourceManager->setResource(KoCanvasResource::MirrorVerticalLock, isLocked);
    emit mirrorModeChanged();
}



bool KisCanvasResourceProvider::mirrorVerticalHideDecorations() {
     return m_resourceManager->resource(KoCanvasResource::MirrorVerticalHideDecorations).toBool();
}

void KisCanvasResourceProvider::setMirrorVerticalHideDecorations(bool hide)
{
    m_resourceManager->setResource(KoCanvasResource::MirrorVerticalHideDecorations, hide);
    emit mirrorModeChanged();
}


bool KisCanvasResourceProvider::mirrorHorizontalHideDecorations() {
     return m_resourceManager->resource(KoCanvasResource::MirrorHorizontalHideDecorations).toBool();
}

void KisCanvasResourceProvider::setMirrorHorizontalHideDecorations(bool hide)
{
    m_resourceManager->setResource(KoCanvasResource::MirrorHorizontalHideDecorations, hide);
    emit mirrorModeChanged();
}


bool KisCanvasResourceProvider::mirrorVerticalLock() {
     return m_resourceManager->resource(KoCanvasResource::MirrorVerticalLock).toBool();
}

void KisCanvasResourceProvider::mirrorVerticalMoveCanvasToCenter() {
     emit moveMirrorVerticalCenter();
}

void KisCanvasResourceProvider::mirrorHorizontalMoveCanvasToCenter() {
     emit moveMirrorHorizontalCenter();
}



void KisCanvasResourceProvider::setOpacity(qreal opacity)
{
    m_resourceManager->setResource(KoCanvasResource::Opacity, opacity);
}

qreal KisCanvasResourceProvider::opacity() const
{
    return m_resourceManager->resource(KoCanvasResource::Opacity).toReal();
}

void KisCanvasResourceProvider::setFlow(qreal flow)
{
    m_resourceManager->setResource(KoCanvasResource::Flow, flow);
}

qreal KisCanvasResourceProvider::flow() const
{
    return m_resourceManager->resource(KoCanvasResource::Flow).toReal();
}

void KisCanvasResourceProvider::setSize(qreal size)
{
    m_resourceManager->setResource(KoCanvasResource::Size, size);
}

qreal KisCanvasResourceProvider::size() const
{
    return m_resourceManager->resource(KoCanvasResource::Size).toReal();
}

void KisCanvasResourceProvider::setPatternSize(qreal size)
{
    m_resourceManager->setResource(KoCanvasResource::PatternSize, size);
}

qreal KisCanvasResourceProvider::patternSize() const
{
    return m_resourceManager->resource(KoCanvasResource::PatternSize).toReal();
}

void KisCanvasResourceProvider::setGlobalAlphaLock(bool lock)
{
    m_resourceManager->setResource(KoCanvasResource::GlobalAlphaLock, lock);
}

bool KisCanvasResourceProvider::globalAlphaLock() const
{
    return m_resourceManager->resource(KoCanvasResource::GlobalAlphaLock).toBool();
}

void KisCanvasResourceProvider::setDisablePressure(bool value)
{
    m_resourceManager->setResource(KoCanvasResource::DisablePressure, value);
}

bool KisCanvasResourceProvider::disablePressure() const
{
    return m_resourceManager->resource(KoCanvasResource::DisablePressure).toBool();
}

void KisCanvasResourceProvider::notifyLoadingWorkspace(KisWorkspaceResourceSP workspace)
{
    emit sigLoadingWorkspace(workspace);
}

void KisCanvasResourceProvider::notifySavingWorkspace(KisWorkspaceResourceSP workspace)
{
    emit sigSavingWorkspace(workspace);
}

