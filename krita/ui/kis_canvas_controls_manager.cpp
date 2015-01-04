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
#include <kaction.h>

#include <KoCanvasResourceManager.h>

#include "kis_action.h"
#include "kis_action_manager.h"
#include "KisViewManager.h"
#include "kis_canvas2.h"
#include "kis_canvas_resource_provider.h"

#include "kis_locked_properties_proxy.h"
#include "kis_locked_properties_server.h"
#include "kis_locked_properties.h"

#include <klocale.h>

const int STEP = 25;

KisCanvasControlsManager::KisCanvasControlsManager(KisViewManager * view) : m_view(view)
{

}

KisCanvasControlsManager::~KisCanvasControlsManager()
{

}

void KisCanvasControlsManager::setup(KisActionManager *actionManager)
{
    KisAction *lighterColor = new KisAction(i18n("Make brush color lighter"));
    lighterColor->setShortcut(Qt::Key_L);
    actionManager->addAction("make_brush_color_lighter", lighterColor);
    connect(lighterColor, SIGNAL(triggered()), SLOT(makeColorLighter()));

    KisAction *darkerColor = new KisAction(i18n("Make brush color darker"));
    darkerColor->setShortcut(Qt::Key_K);
    actionManager->addAction("make_brush_color_darker", darkerColor);
    connect(darkerColor, SIGNAL(triggered()), SLOT(makeColorDarker()));

    KisAction *increaseOpacity = new KisAction(i18n("Increase opacity"));
    increaseOpacity->setShortcut(Qt::Key_O);
    actionManager->addAction("increase_opacity", increaseOpacity);
    connect(increaseOpacity, SIGNAL(triggered()), SLOT(increaseOpacity()));

    KisAction *decreaseOpacity = new KisAction(i18n("Decrease opacity"));
    decreaseOpacity->setShortcut(Qt::Key_I);
    actionManager->addAction("decrease_opacity", decreaseOpacity);
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
    if (m_view->resourceProvider()->currentPreset()->settings()->hasProperty("OpacityValue"))
        m_view->resourceProvider()->currentPreset()->settings()->setProperty("OpacityValue", alpha);

    KisLockedPropertiesProxy *p = KisLockedPropertiesServer::instance()->createLockedPropertiesProxy(m_view->resourceProvider()->currentPreset()->settings());
    p->setProperty("OpacityValue", alpha);
}

void KisCanvasControlsManager::increaseOpacity()
{
    stepAlpha(0.1f);
}

void KisCanvasControlsManager::decreaseOpacity()
{
    stepAlpha(-0.1f);
}
