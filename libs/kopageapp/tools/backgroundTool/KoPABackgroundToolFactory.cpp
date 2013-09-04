/* This file is part of the KDE project
 * Copyright (C) 2008 Carlos Licea <carlos.licea@kdemail.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoPABackgroundToolFactory.h"

//Calligra includes
#include "KoPABackgroundTool.h"

#include <KoPageApp.h>
#include <KoPADocument.h>
#include <KoPACanvas.h>
#include <KoPAViewBase.h>
#include <KoIcon.h>

KoPABackgroundToolFactory::KoPABackgroundToolFactory()
    : KoToolFactoryBase("KoPABackgroundTool")
{
    setToolType("calligraflow, calligrastage");
    setActivationShapeId("flake/always");
    setIconName(koIconNameCStr("backgroundtool"));
    setPriority(3);
}

KoPABackgroundToolFactory::~KoPABackgroundToolFactory()
{
}

KoToolBase * KoPABackgroundToolFactory::createTool(KoCanvasBase *canvas)
{
    // We need the canvas to know in which app we are to turn the tooltip to page or slide design
    KoPAViewBase *view = static_cast<KoPACanvasBase *>(canvas)->koPAView();
    const QString toolTip =
        (view->kopaDocument()->pageType() == KoPageApp::Page) ? i18n("Page Design") : i18n("Slide Design");
    setToolTip(toolTip);
    return new KoPABackgroundTool(canvas);
}

bool KoPABackgroundToolFactory::canCreateTool(KoCanvasBase *canvas) const
{
    KoPACanvas *paCanvas = dynamic_cast<KoPACanvas *>(canvas);
    return paCanvas != 0; // we only work in KoPACanvas
}
