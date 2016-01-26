/*
 *  Copyright (c) 2003-2009 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2014 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_canvas_controls_manager.h"

#include <kactioncollection.h>
#include <QAction>

#include <KoCanvasResourceManager.h>

#include "kis_action.h"
#include "kis_action_manager.h"
#include "KisViewManager.h"
#include "kis_canvas2.h"
#include "kis_canvas_resource_provider.h"

#include <brushengine/kis_locked_properties_proxy.h>
#include <brushengine/kis_locked_properties_server.h>
#include <brushengine/kis_locked_properties.h>

#include <klocalizedstring.h>

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

    KisAction *increaseOpacity = actionManager->createAction("increase_opacity");
    connect(increaseOpacity, SIGNAL(triggered()), SLOT(increaseOpacity()));

    KisAction *decreaseOpacity = actionManager->createAction("decrease_opacity");
    connect(decreaseOpacity, SIGNAL(triggered()), SLOT(decreaseOpacity()));
}

void KisCanvasControlsManager::setView(QPointer<KisView>imageView)
{
    Q_UNUSED(imageView);
}

void KisCanvasControlsManager::transformColor(int step)
{
    if (!m_view) return;
    if (!m_view->canvasBase()) return;
    if (!m_view->resourceProvider()->resourceManager()) return;

    KoColor color = m_view->resourceProvider()->resourceManager()->resource(KoCanvasResourceManager::ForegroundColor).value<KoColor>();
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
    m_view->resourceProvider()->resourceManager()->setResource(KoCanvasResourceManager::ForegroundColor, color);
}


void KisCanvasControlsManager::makeColorDarker()
{
    transformColor(-STEP);
}

void KisCanvasControlsManager::makeColorLighter()
{
    transformColor(STEP);
}

void KisCanvasControlsManager::stepAlpha(float step)
{
    if (!m_view) return;
    if (!m_view->canvasBase()) return;
    if (!m_view->resourceProvider()->resourceManager()) return;

    qreal alpha = m_view->resourceProvider()->resourceManager()->resource(KisCanvasResourceProvider::Opacity).toDouble();
    alpha += step;
    alpha = qBound<qreal>(0.0, alpha, 1.0);
    m_view->canvasBase()->resourceManager ()->setResource(KisCanvasResourceProvider::Opacity, alpha);

    // FIXME: DK: should we uncomment it back?
    //KisLockedPropertiesProxy *p = KisLockedPropertiesServer::instance()->createLockedPropertiesProxy(m_view->resourceProvider()->currentPreset()->settings());
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
