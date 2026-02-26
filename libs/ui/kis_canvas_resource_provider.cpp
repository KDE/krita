/*
 *  SPDX-FileCopyrightText: 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_canvas_resource_provider.h"

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
#include <KoResource.h>
#include <KoSvgTextPropertyData.h>

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
#include <KoUnit.h>


KisCanvasResourceProvider::KisCanvasResourceProvider(KisViewManager * view)
    : m_view(view),
      m_presetShadowUpdater(view)
{
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
    m_resourceManager->setResource(KoCanvasResource::EffectivePhysicalZoom, 1.0);

    connect(m_resourceManager, SIGNAL(canvasResourceChanged(int,QVariant)),
            this, SLOT(slotCanvasResourceChanged(int,QVariant)));

    m_resourceManager->setResource(KoCanvasResource::GamutMaskActive, false);

    connect(m_resourceManager, SIGNAL(canvasResourceChanged(int,QVariant)),
            &m_presetShadowUpdater, SLOT(slotCanvasResourceChanged(int,QVariant)));
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

QList<KoColor> KisCanvasResourceProvider::colorHistory() const
{
    QVariant c = m_resourceManager->resource(KoCanvasResource::ColorHistory);
    if (c.isValid()) {
        return c.value<QList<KoColor>>();
    }
    else {
        return QList<KoColor>();
    }
}

void KisCanvasResourceProvider::setColorHistory(const QList<KoColor>& colors)
{
    QVariant v;
    v.setValue(colors);
    m_resourceManager->setResource(KoCanvasResource::ColorHistory, v);
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
    QVariant v;
    v.setValue(preset);
    m_resourceManager->setResource(KoCanvasResource::CurrentPaintOpPreset, v);

    Q_EMIT sigPaintOpPresetChanged(preset);
}

KisPaintOpPresetSP KisCanvasResourceProvider::previousPreset() const
{
    KisPaintOpPresetSP preset = m_resourceManager->resource(KoCanvasResource::PreviousPaintOpPreset).value<KisPaintOpPresetSP>();
    return preset;
}

void KisCanvasResourceProvider::setPreviousPaintOpPreset(const KisPaintOpPresetSP preset)
{
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
    v.setValue(pattern);
    m_resourceManager->setResource(KoCanvasResource::CurrentPattern, v);
    Q_EMIT sigPatternChanged(pattern);
}

void KisCanvasResourceProvider::slotGradientActivated(KoResourceSP res)
{

    KoAbstractGradientSP gradient = res.dynamicCast<KoAbstractGradient>();
    QVariant v;
    v.setValue(gradient);
    m_resourceManager->setResource(KoCanvasResource::CurrentGradient, v);
    Q_EMIT sigGradientChanged(gradient);
}


void KisCanvasResourceProvider::setBGColor(const KoColor& c)
{
    QVariant v;
    v.setValue(c);
    m_resourceManager->setResource(KoCanvasResource::BackgroundColor, v);
    Q_EMIT sigBGColorChanged(c);
}

void KisCanvasResourceProvider::setFGColor(const KoColor& c)
{
    m_fGChanged = true;

    QVariant v;
    v.setValue(c);
    m_resourceManager->setResource(KoCanvasResource::ForegroundColor, v);
    Q_EMIT sigFGColorChanged(c);
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
    Q_EMIT sigNodeChanged(currentNode());
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

    // update KoUnit value for the document
    m_resourceManager->
                setResource(KoCanvasResource::Unit, canvas->unit());

    qreal scaleX, scaleY;
    canvas->coordinatesConverter()->imageScale(&scaleX, &scaleY);

    Q_EMIT sigOnScreenResolutionChanged(scaleX, scaleY);
}

void KisCanvasResourceProvider::slotCanvasResourceChanged(int key, const QVariant & res)
{
    switch (key) {
    case(KoCanvasResource::ForegroundColor):
        m_fGChanged = true;
        Q_EMIT sigFGColorChanged(res.value<KoColor>());
        break;
    case(KoCanvasResource::BackgroundColor):
        Q_EMIT sigBGColorChanged(res.value<KoColor>());
        break;
    case(KoCanvasResource::CurrentPattern):
        Q_EMIT sigPatternChanged(res.value<KoPatternSP>());
        break;
    case(KoCanvasResource::CurrentGradient):
        Q_EMIT sigGradientChanged(res.value<KoAbstractGradientSP>());
        break;
    case(KoCanvasResource::CurrentKritaNode) :
        Q_EMIT sigNodeChanged(currentNode());
        break;
    case(KoCanvasResource::CurrentEffectiveCompositeOp) :
        Q_EMIT sigEffectiveCompositeOpChanged();
        break;
    case (KoCanvasResource::Opacity):
    {
        Q_EMIT sigOpacityChanged(res.toDouble());
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
        Q_EMIT sigFGColorUsed(fgColor());
        m_fGChanged = false;
    }
}

void KisCanvasResourceProvider::slotGamutMaskActivated(KoGamutMaskSP mask)
{
    QVariant v;
    v.setValue(mask);
    m_resourceManager->setResource(KoCanvasResource::CurrentGamutMask, v);

    m_resourceManager->setResource(KoCanvasResource::GamutMaskActive, QVariant::fromValue(true));

    Q_EMIT sigGamutMaskChanged(mask);
}

void KisCanvasResourceProvider::slotGamutMaskUnset()
{
    m_resourceManager->setResource(KoCanvasResource::GamutMaskActive, QVariant::fromValue(false));
    m_resourceManager->clearResource(KoCanvasResource::CurrentGamutMask);
    Q_EMIT sigGamutMaskUnset();
}

void KisCanvasResourceProvider::slotGamutMaskPreviewUpdate()
{
    Q_EMIT sigGamutMaskPreviewUpdate();
}

void KisCanvasResourceProvider::slotGamutMaskDeactivate()
{
    m_resourceManager->setResource(KoCanvasResource::GamutMaskActive, QVariant::fromValue(false));
    Q_EMIT sigGamutMaskDeactivated();
}

void KisCanvasResourceProvider::setMirrorHorizontal(bool mirrorHorizontal)
{
    m_resourceManager->setResource(KoCanvasResource::MirrorHorizontal, mirrorHorizontal);
    Q_EMIT mirrorModeChanged();
}

bool KisCanvasResourceProvider::mirrorHorizontal() const
{
    return m_resourceManager->resource(KoCanvasResource::MirrorHorizontal).toBool();
}

void KisCanvasResourceProvider::setMirrorVertical(bool mirrorVertical)
{
    m_resourceManager->setResource(KoCanvasResource::MirrorVertical, mirrorVertical);
    Q_EMIT mirrorModeChanged();
}

bool KisCanvasResourceProvider::mirrorVertical() const
{
    return m_resourceManager->resource(KoCanvasResource::MirrorVertical).toBool();
}

void KisCanvasResourceProvider::setMirrorHorizontalLock(bool isLocked)
{
    m_resourceManager->setResource(KoCanvasResource::MirrorHorizontalLock, isLocked);
    Q_EMIT mirrorModeChanged();
}

bool KisCanvasResourceProvider::mirrorHorizontalLock() {
     return m_resourceManager->resource(KoCanvasResource::MirrorHorizontalLock).toBool();
}

void KisCanvasResourceProvider::setMirrorVerticalLock(bool isLocked)
{
    m_resourceManager->setResource(KoCanvasResource::MirrorVerticalLock, isLocked);
    Q_EMIT mirrorModeChanged();
}



bool KisCanvasResourceProvider::mirrorVerticalHideDecorations() {
     return m_resourceManager->resource(KoCanvasResource::MirrorVerticalHideDecorations).toBool();
}

void KisCanvasResourceProvider::setMirrorVerticalHideDecorations(bool hide)
{
    m_resourceManager->setResource(KoCanvasResource::MirrorVerticalHideDecorations, hide);
    Q_EMIT mirrorModeChanged();
}


bool KisCanvasResourceProvider::mirrorHorizontalHideDecorations() {
     return m_resourceManager->resource(KoCanvasResource::MirrorHorizontalHideDecorations).toBool();
}

void KisCanvasResourceProvider::setMirrorHorizontalHideDecorations(bool hide)
{
    m_resourceManager->setResource(KoCanvasResource::MirrorHorizontalHideDecorations, hide);
    Q_EMIT mirrorModeChanged();
}


bool KisCanvasResourceProvider::mirrorVerticalLock() {
     return m_resourceManager->resource(KoCanvasResource::MirrorVerticalLock).toBool();
}

void KisCanvasResourceProvider::mirrorVerticalMoveCanvasToCenter() {
     Q_EMIT moveMirrorVerticalCenter();
}

void KisCanvasResourceProvider::mirrorHorizontalMoveCanvasToCenter() {
     Q_EMIT moveMirrorHorizontalCenter();
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

void KisCanvasResourceProvider::setFade(qreal fade)
{
    m_resourceManager->setResource(KoCanvasResource::Fade, fade);
}

qreal KisCanvasResourceProvider::fade() const
{
    return m_resourceManager->resource(KoCanvasResource::Fade).toReal();
}

void KisCanvasResourceProvider::setBrushRotation(qreal rotation)
{
    m_resourceManager->setResource(KoCanvasResource::BrushRotation, rotation);
}

qreal KisCanvasResourceProvider::brushRotation() const
{
    return m_resourceManager->resource(KoCanvasResource::BrushRotation).toReal();
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

void KisCanvasResourceProvider::setTextPropertyData(KoSvgTextPropertyData data)
{
    m_resourceManager->setResource(KoCanvasResource::SvgTextPropertyData, QVariant::fromValue(data));
    Q_EMIT sigTextPropertiesChanged();
}

KoSvgTextPropertyData KisCanvasResourceProvider::textPropertyData() const
{
    return m_resourceManager->resource(KoCanvasResource::SvgTextPropertyData).value<KoSvgTextPropertyData>();
}

void KisCanvasResourceProvider::setCharacterPropertyData(KoSvgTextPropertyData data)
{
    m_resourceManager->setResource(KoCanvasResource::SvgCharacterTextPropertyData, QVariant::fromValue(data));
    Q_EMIT sigCharacterPropertiesChanged();
}

KoSvgTextPropertyData KisCanvasResourceProvider::characterTextPropertyData() const
{
    return m_resourceManager->resource(KoCanvasResource::SvgCharacterTextPropertyData).value<KoSvgTextPropertyData>();
}

void KisCanvasResourceProvider::notifyLoadingWorkspace(KisWorkspaceResourceSP workspace)
{
    Q_EMIT sigLoadingWorkspace(workspace);
}

void KisCanvasResourceProvider::notifySavingWorkspace(KisWorkspaceResourceSP workspace)
{
    Q_EMIT sigSavingWorkspace(workspace);
}

