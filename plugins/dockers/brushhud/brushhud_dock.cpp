/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "brushhud_dock.h"

#include <klocalizedstring.h>

#include <KoCanvasResourceProvider.h>
#include <KoCanvasBase.h>

#include "kis_brush_hud.h"
#include "kis_canvas2.h"
#include "KisViewManager.h"


BrushHudDock::BrushHudDock( )
    : QDockWidget(i18nc("@title:window On-Canvas Brush Editor docker", "On-Canvas Brush Editor"))
    , m_canvas(0)
{
}

void BrushHudDock::setCanvas(KoCanvasBase *canvas)
{
    setEnabled(canvas != 0);

    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this);
    }

    m_canvas = dynamic_cast<KisCanvas2*>(canvas);

    if (m_canvas && m_canvas->viewManager() && m_canvas->resourceManager()) {
        m_brushHud = new KisBrushHud(m_canvas->viewManager()->canvasResourceProvider(), this);
        setWidget(m_brushHud);
    }
    else {
        setWidget(nullptr);
    }
}

