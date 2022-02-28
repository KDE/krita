/*
 *  SPDX-FileCopyrightText: 2003-2009 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2014 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_canvas_controls_manager.h"

#include <kactioncollection.h>
#include <QAction>

#include <KoCanvasResourceProvider.h>

#include "kis_action.h"
#include "kis_action_manager.h"
#include "KisViewManager.h"
#include "kis_canvas2.h"
#include "kis_canvas_resource_provider.h"

#include <brushengine/kis_locked_properties_proxy.h>
#include <brushengine/kis_locked_properties_server.h>
#include <brushengine/kis_locked_properties.h>

#include <klocalizedstring.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include <math.h>

const int STEP = 25;

KisCanvasControlsManager::KisCanvasControlsManager(KisViewManager * view) : m_view(view)
{

}

KisCanvasControlsManager::~KisCanvasControlsManager()
{

}

void KisCanvasControlsManager::setup(KisActionManager *actionManager)
{
    KisAction *lighterColor = actionManager->createAction("make_brush_color_lighter");
    connect(lighterColor, SIGNAL(triggered()), SLOT(makeColorLighter()));

    KisAction *darkerColor = actionManager->createAction("make_brush_color_darker");
    connect(darkerColor, SIGNAL(triggered()), SLOT(makeColorDarker()));
    KisAction *saturatedColor = actionManager->createAction("make_brush_color_saturated");
    connect(saturatedColor, SIGNAL(triggered()), SLOT(makeColorSaturated()));

    KisAction *desaturatedColor = actionManager->createAction("make_brush_color_desaturated");
    connect(desaturatedColor, SIGNAL(triggered()), SLOT(makeColorDesaturated()));

    KisAction *hueClockwise = actionManager->createAction("shift_brush_color_clockwise");
    connect(hueClockwise, SIGNAL(triggered()), SLOT(shiftHueClockWise()));

    KisAction *hueCounterClockwise = actionManager->createAction("shift_brush_color_counter_clockwise");
    connect(hueCounterClockwise, SIGNAL(triggered()), SLOT(shiftHueCounterClockWise()));

    KisAction *moreRed = actionManager->createAction("make_brush_color_redder");
    connect(moreRed, SIGNAL(triggered()), SLOT(makeColorRed()));

    KisAction *moreGreen = actionManager->createAction("make_brush_color_greener");
    connect(moreGreen, SIGNAL(triggered()), SLOT(makeColorGreen()));

    KisAction *moreBlue = actionManager->createAction("make_brush_color_bluer");
    connect(moreBlue, SIGNAL(triggered()), SLOT(makeColorBlue()));

    KisAction *moreYellow = actionManager->createAction("make_brush_color_yellower");
    connect(moreYellow, SIGNAL(triggered()), SLOT(makeColorYellow()));

    KisAction *increaseOpacity = actionManager->createAction("increase_opacity");
    connect(increaseOpacity, SIGNAL(triggered()), SLOT(increaseOpacity()));

    KisAction *decreaseOpacity = actionManager->createAction("decrease_opacity");
    connect(decreaseOpacity, SIGNAL(triggered()), SLOT(decreaseOpacity()));

    KisAction *increaseFlow = actionManager->createAction("increase_flow");
    connect(increaseFlow, SIGNAL(triggered()), SLOT(increaseFlow()));

    KisAction *decreaseFlow = actionManager->createAction("decrease_flow");
    connect(decreaseFlow, SIGNAL(triggered()), SLOT(decreaseFlow()));

    KisAction *increaseFade = actionManager->createAction("increase_fade");
    connect(increaseFade, SIGNAL(triggered()), SLOT(increaseFade()));

    KisAction *decreaseFade = actionManager->createAction("decrease_fade");
    connect(decreaseFade, SIGNAL(triggered()), SLOT(decreaseFade()));

    KisAction *increaseScatter = actionManager->createAction("increase_scatter");
    connect(increaseScatter, SIGNAL(triggered()), SLOT(increaseScatter()));

    KisAction *decreaseScatter = actionManager->createAction("decrease_scatter");
    connect(decreaseScatter, SIGNAL(triggered()), SLOT(decreaseScatter()));
}

void KisCanvasControlsManager::setView(QPointer<KisView>imageView)
{
    Q_UNUSED(imageView);
}

void KisCanvasControlsManager::transformColor(int step)
{
    if (!m_view) return;
    if (!m_view->canvasBase()) return;
    if (!m_view->canvasResourceProvider()->resourceManager()) return;
    KConfigGroup hotkeycfg =  KSharedConfig::openConfig()->group("colorhotkeys");
    int steps = hotkeycfg.readEntry("steps_lightness", 10);
    if (steps <= 0) {
        steps = 1;
    }


    KoColor color = m_view->canvasResourceProvider()->resourceManager()->resource(KoCanvasResource::ForegroundColor).value<KoColor>();
    if (color.colorSpace()->colorModelId().id()=="CMYKA" || color.colorSpace()->colorModelId().id()=="XYZA"){
        QColor rgb = color.toQColor();
        int h = 0, s = 0, v = 0;
        rgb.getHsv(&h,&s,&v);
        if ((v < 255) || ((s == 0) || (s == 255))) {
            v += step;
            v = qBound(0,v,255);
        } else {
            s += -step;
            s = qBound(0,s,255);
        }
        rgb.setHsv(h,s,v);
        color.fromQColor(rgb);
    } else if (step<0){
        color.colorSpace()->decreaseLuminosity(color.data(), 1.0/steps);
    } else {
        color.colorSpace()->increaseLuminosity(color.data(), 1.0/steps);
    }
    m_view->canvasResourceProvider()->resourceManager()->setResource(KoCanvasResource::ForegroundColor, color);
}
void KisCanvasControlsManager::transformSaturation(int step)
{
    if (!m_view) return;
    if (!m_view->canvasBase()) return;
    if (!m_view->canvasResourceProvider()->resourceManager()) return;
    KConfigGroup hotkeycfg =  KSharedConfig::openConfig()->group("colorhotkeys");
    int steps = hotkeycfg.readEntry("steps_saturation", 10);
    if (steps <= 0) {
        steps = 1;
    }

    KoColor color = m_view->canvasResourceProvider()->resourceManager()->resource(KoCanvasResource::ForegroundColor).value<KoColor>();
    if (color.colorSpace()->colorModelId().id()=="CMYKA" || color.colorSpace()->colorModelId().id()=="XYZA"){
        QColor rgb = color.toQColor();
        int h = 0, s = 0, v = 0;
        rgb.getHsl(&h,&s,&v);
        s += step;
        s = qBound(0,s,255);
        rgb.setHsl(h,s,v);
        color.fromQColor(rgb);
    } else if (step<0){
        color.colorSpace()->decreaseSaturation(color.data(), 1.0/steps);
    } else {
        color.colorSpace()->increaseSaturation(color.data(), 1.0/steps);
    }
    m_view->canvasResourceProvider()->resourceManager()->setResource(KoCanvasResource::ForegroundColor, color);
}
void KisCanvasControlsManager::transformHue(int step)
{
    if (!m_view) return;
    if (!m_view->canvasBase()) return;
    if (!m_view->canvasResourceProvider()->resourceManager()) return;
    KConfigGroup hotkeycfg =  KSharedConfig::openConfig()->group("colorhotkeys");
    int steps = hotkeycfg.readEntry("steps_hue", 36);
    if (steps <= 0) {
        steps = 1;
    }

    KoColor color = m_view->canvasResourceProvider()->resourceManager()->resource(KoCanvasResource::ForegroundColor).value<KoColor>();
    if (color.colorSpace()->colorModelId().id()=="CMYKA" || color.colorSpace()->colorModelId().id()=="XYZA"){
        QColor rgb = color.toQColor();
        int h = 0, s = 0, v = 0;
        rgb.getHsl(&h,&s,&v);
        h += step;
        if (h>360.0 || h<0.0){h=fmod(h, 360.0);}
        rgb.setHsl(h,s,v);
        color.fromQColor(rgb);
    } else if (step<0){
        color.colorSpace()->increaseHue(color.data(), 1.0/steps);
    } else {
        color.colorSpace()->decreaseHue(color.data(), 1.0/steps);
    }
    m_view->canvasResourceProvider()->resourceManager()->setResource(KoCanvasResource::ForegroundColor, color);
}
void KisCanvasControlsManager::transformRed(int step)
{
    if (!m_view) return;
    if (!m_view->canvasBase()) return;
    if (!m_view->canvasResourceProvider()->resourceManager()) return;
    KConfigGroup hotkeycfg =  KSharedConfig::openConfig()->group("colorhotkeys");
    int steps = hotkeycfg.readEntry("steps_redgreen", 10);
    if (steps <= 0) {
        steps = 1;
    }

    KoColor color = m_view->canvasResourceProvider()->resourceManager()->resource(KoCanvasResource::ForegroundColor).value<KoColor>();
    if (step<0){
        color.colorSpace()->increaseGreen(color.data(), 1.0/steps);
    } else {
        color.colorSpace()->increaseRed(color.data(), 1.0/steps);
    }
    m_view->canvasResourceProvider()->resourceManager()->setResource(KoCanvasResource::ForegroundColor, color);
}
void KisCanvasControlsManager::transformBlue(int step)
{
    if (!m_view) return;
    if (!m_view->canvasBase()) return;
    if (!m_view->canvasResourceProvider()->resourceManager()) return;
    KConfigGroup hotkeycfg =  KSharedConfig::openConfig()->group("colorhotkeys");
    int steps = hotkeycfg.readEntry("steps_blueyellow", 10);
    if (steps <= 0) {
        steps = 1;
    }

    KoColor color = m_view->canvasResourceProvider()->resourceManager()->resource(KoCanvasResource::ForegroundColor).value<KoColor>();
    if (step<0){
        color.colorSpace()->increaseYellow(color.data(), 1.0/steps);
    } else {
        color.colorSpace()->increaseBlue(color.data(), 1.0/steps);
    }
    m_view->canvasResourceProvider()->resourceManager()->setResource(KoCanvasResource::ForegroundColor, color);
}


void KisCanvasControlsManager::makeColorDarker()
{
    transformColor(-STEP);
}

void KisCanvasControlsManager::makeColorLighter()
{
    transformColor(STEP);
}

void KisCanvasControlsManager::makeColorDesaturated()
{
    transformSaturation(-STEP);
}

void KisCanvasControlsManager::makeColorSaturated()
{
    transformSaturation(STEP);
}
void KisCanvasControlsManager::shiftHueClockWise()
{
    transformHue(STEP);
}

void KisCanvasControlsManager::shiftHueCounterClockWise()
{
    transformHue(-STEP);
}
void KisCanvasControlsManager::makeColorRed()
{
    transformRed(STEP);
}
void KisCanvasControlsManager::makeColorGreen()
{
    transformRed(-STEP);
}
void KisCanvasControlsManager::makeColorBlue()
{
    transformBlue(STEP);
}
void KisCanvasControlsManager::makeColorYellow()
{
    transformBlue(-STEP);
}

void KisCanvasControlsManager::stepAlpha(float step)
{
    if (!m_view) return;
    if (!m_view->canvasBase()) return;
    if (!m_view->canvasResourceProvider()->resourceManager()) return;

    qreal alpha = m_view->canvasResourceProvider()->resourceManager()->resource(KoCanvasResource::Opacity).toDouble();
    alpha += step;
    alpha = qBound<qreal>(0.0, alpha, 1.0);
    m_view->canvasBase()->resourceManager ()->setResource(KoCanvasResource::Opacity, alpha);

    m_view->showFloatingMessage(i18n("Brush Opacity: %1%", alpha * 100), QIcon(), 1000, KisFloatingMessage::High,
                                Qt::AlignLeft | Qt::TextWordWrap | Qt::AlignVCenter);

    // FIXME: DK: should we uncomment it back?
    //KisLockedPropertiesProxySP p = KisLockedPropertiesServer::instance()->createLockedPropertiesProxy(m_view->resourceProvider()->currentPreset()->settings());
    //p->setProperty("OpacityValue", alpha);
}

void KisCanvasControlsManager::increaseOpacity()
{
    stepAlpha(0.1f);
}

void KisCanvasControlsManager::decreaseOpacity()
{
    stepAlpha(-0.1f);
}

void KisCanvasControlsManager::stepFlow(float step)
{
    if (!m_view) return;
    if (!m_view->canvasBase()) return;
    if (!m_view->canvasResourceProvider()->resourceManager()) return;

    qreal flow = m_view->canvasResourceProvider()->resourceManager()->resource(KoCanvasResource::Flow).toDouble();
    flow += step;
    flow = qBound<qreal>(0.0, flow, 1.0);
    m_view->canvasBase()->resourceManager ()->setResource(KoCanvasResource::Flow, flow);

    // verify if the brush does actually support this change
    flow = m_view->canvasBase()->resourceManager ()->resource(KoCanvasResource::Flow).toReal();

    m_view->showFloatingMessage(i18nc("Brush Option Flow", "Flow: %1%", flow * 100), QIcon(), 1000, KisFloatingMessage::High,
                                Qt::AlignLeft | Qt::TextWordWrap | Qt::AlignVCenter);
}

void KisCanvasControlsManager::increaseFlow()
{
    qreal flow = m_view->canvasResourceProvider()->resourceManager()->resource(KoCanvasResource::Flow).toDouble();

    stepFlow(0.1f);
}

void KisCanvasControlsManager::decreaseFlow()
{
    qreal flow = m_view->canvasResourceProvider()->resourceManager()->resource(KoCanvasResource::Flow).toDouble();

    stepFlow(-0.1f);
}

void KisCanvasControlsManager::stepFade(float step)
{
    if (!m_view) return;
    if (!m_view->canvasBase()) return;
    if (!m_view->canvasResourceProvider()->resourceManager()) return;

    qreal fade = m_view->canvasResourceProvider()->resourceManager()->resource(KoCanvasResource::Fade).toDouble();
    fade += step;
    fade = qBound<qreal>(0.0, fade, 1.0);
    m_view->canvasBase()->resourceManager ()->setResource(KoCanvasResource::Fade, fade);

    // verify if the brush does actually support this change
    fade = m_view->canvasBase()->resourceManager ()->resource(KoCanvasResource::Fade).toReal();

    m_view->showFloatingMessage(i18nc("Edge softness, Brush Option Fade", "Fade: %1", fade),
                                QIcon(), 1000, KisFloatingMessage::High, Qt::AlignLeft | Qt::TextWordWrap | Qt::AlignVCenter);
}

void KisCanvasControlsManager::increaseFade()
{
    stepFade(0.1f);
}

void KisCanvasControlsManager::decreaseFade()
{
    stepFade(-0.1f);
}

void KisCanvasControlsManager::stepScatter(float step)
{
    if (!m_view) return;
    if (!m_view->canvasBase()) return;
    if (!m_view->canvasResourceProvider()->resourceManager()) return;

    qreal scatter = m_view->canvasResourceProvider()->resourceManager()->resource(KoCanvasResource::Scatter).toDouble();
    scatter += step;
    scatter = qBound<qreal>(0.0, scatter, 5.0);
    m_view->canvasBase()->resourceManager ()->setResource(KoCanvasResource::Scatter, scatter);

    // verify if the brush does actually support this change
    scatter = m_view->canvasBase()->resourceManager ()->resource(KoCanvasResource::Scatter).toReal();

    m_view->showFloatingMessage(i18nc("Brush Option Scatter", "Scatter: %1%", scatter * 100), QIcon(), 1000, KisFloatingMessage::High,
                                Qt::AlignLeft | Qt::TextWordWrap | Qt::AlignVCenter);
}

void KisCanvasControlsManager::increaseScatter()
{
    stepScatter(0.1f);
}

void KisCanvasControlsManager::decreaseScatter()
{
    stepScatter(-0.1f);
}
