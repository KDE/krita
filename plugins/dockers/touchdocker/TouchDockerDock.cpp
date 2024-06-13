/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "TouchDockerDock.h"
#include "TouchDockerWidget.h"

TouchDockerDock::TouchDockerDock()
    : QDockWidget(i18n("Touch Docker"))
{
    m_page = new TouchDockerWidget(this);
    setWidget(m_page);
}

TouchDockerDock::~TouchDockerDock()
{
}

void TouchDockerDock::setCanvas(KoCanvasBase * canvas)
{
    m_canvas = dynamic_cast<KisCanvas2*>(canvas);
    m_page->setCanvas(m_canvas);
}

void TouchDockerDock::unsetCanvas()
{
    m_canvas = nullptr;
    m_page->unsetCanvas();
}
