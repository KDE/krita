/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "overviewdocker_dock.h"
#include "overviewwidget.h"
#include "overviewdocker_page.h"

OverviewDockerDock::OverviewDockerDock()
    : QDockWidget(i18n("Overview"))
{
    m_page = new OverviewDockerPage(this);

    setWidget(m_page);

    setEnabled(false);
}

OverviewDockerDock::~OverviewDockerDock()
{
}

void OverviewDockerDock::setCanvas(KoCanvasBase * canvas)
{
    if(m_canvas == canvas)
        return;

    setEnabled(canvas != nullptr);

    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this);
        m_canvas->image()->disconnect(this);
    }

    m_canvas = dynamic_cast<KisCanvas2*>(canvas);

    m_page->setCanvas(canvas);
}

void OverviewDockerDock::unsetCanvas()
{
    setEnabled(false);
    m_canvas = nullptr;
    m_page->setCanvas(0);
}
