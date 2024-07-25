/*
 *  SPDX-FileCopyrightText: 2011 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "channeldocker_dock.h"

#include "ChannelDockerWidget.h"

#include <KoCanvasBase.h>
#include <kis_canvas2.h>

ChannelDockerDock::ChannelDockerDock()
{
    setWindowTitle(i18nc("Channel as in Color Channels", "Channels"));

    m_widget = new ChannelDockerWidget(this);
    setWidget(m_widget);
}

void ChannelDockerDock::setCanvas(KoCanvasBase * canvas)
{
    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this);
        m_canvas->image()->disconnect(this);
    }

    KisCanvas2 *newCanvas = canvas ? dynamic_cast<KisCanvas2*>(canvas) : nullptr;

    m_widget->setCanvas(newCanvas);

    setEnabled(bool(canvas));
}

void ChannelDockerDock::unsetCanvas()
{
    setCanvas(0);
}
