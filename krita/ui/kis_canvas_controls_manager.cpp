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

#include "kis_view2.h"
#include "kis_canvas2.h"
#include "kis_canvas_resource_provider.h"

#include "kis_locked_properties_proxy.h"
#include "kis_locked_properties_server.h"
#include "kis_locked_properties.h"

#include <klocale.h>

const int STEP = 25;

KisCanvasControlsManager::KisCanvasControlsManager(KisView2 * view) : m_view(view)
{

}

KisCanvasControlsManager::~KisCanvasControlsManager()
{

}

void KisCanvasControlsManager::setup(KActionCollection* collection)
{
    KAction *lighterColor = new KAction(i18n("Make brush color lighter"), collection);
    lighterColor->setShortcut(Qt::Key_L);
    collection->addAction("make_brush_color_lighter", lighterColor);
    connect(lighterColor, SIGNAL(triggered()), SLOT(makeColorLighter()));

    KAction *darkerColor = new KAction(i18n("Make brush color darker"), collection);
    darkerColor->setShortcut(Qt::Key_K);
    collection->addAction("make_brush_color_darker", darkerColor);
    connect(darkerColor, SIGNAL(triggered()), SLOT(makeColorDarker()));

    KAction *increaseOpacity = new KAction(i18n("Increase opacity"), collection);
    increaseOpacity->setShortcut(Qt::Key_O);
    collection->addAction("increase_opacity", increaseOpacity);
    connect(increaseOpacity, SIGNAL(triggered()), SLOT(increaseOpacity()));

    KAction *decreaseOpacity = new KAction(i18n("Decrease opacity"), collection);
    decreaseOpacity->setShortcut(Qt::Key_I);
    collection->addAction("decrease_opacity", decreaseOpacity);
    connect(decreaseOpacity, SIGNAL(triggered()), SLOT(decreaseOpacity()));
}

void KisCanvasControlsManager::transformColor(int step)
{
    KoColor color = m_view->canvasBase()->resourceManager()->resource(KoCanvasResourceManager::ForegroundColor).value<KoColor>();
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
    m_view->canvasBase()->resourceManager()->setResource(KoCanvasResourceManager::ForegroundColor, color);
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
    qreal alpha = m_view->canvasBase()->resourceManager()->resource(KisCanvasResourceProvider::Opacity).toDouble();
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
