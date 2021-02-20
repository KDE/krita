/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_color_selector_ng_dock.h"

#include <klocalizedstring.h>
#include "kis_canvas2.h"

#include "kis_color_selector_ng_docker_widget.h"


KisColorSelectorNgDock::KisColorSelectorNgDock()
    : QDockWidget()
{
    m_colorSelectorNgWidget = new KisColorSelectorNgDockerWidget(this);

    setWidget(m_colorSelectorNgWidget);
    m_colorSelectorNgWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setWindowTitle(i18n("Advanced Color Selector"));
}

void KisColorSelectorNgDock::setCanvas(KoCanvasBase * canvas)
{
    setEnabled(canvas != 0);
    m_colorSelectorNgWidget->setCanvas(dynamic_cast<KisCanvas2*>(canvas));
}

void KisColorSelectorNgDock::unsetCanvas()
{
    setEnabled(false);
    m_colorSelectorNgWidget->unsetCanvas();
}

