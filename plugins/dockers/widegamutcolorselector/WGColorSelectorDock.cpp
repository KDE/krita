/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "WGColorSelectorDock.h"
#include "KisVisualColorSelector.h"

#include <klocalizedstring.h>

#include <kis_canvas2.h>
#include <kis_display_color_converter.h>
#include <KoCanvasResourceProvider.h>

#include <QLabel>
#include <QBoxLayout>
#include <QDebug>

WGColorSelectorDock::WGColorSelectorDock()
	: QDockWidget()
{
    setWindowTitle(i18n("Wide Gamut Color Selector"));

    QWidget *mainWidget = new QWidget();
    mainWidget->setLayout(new QVBoxLayout());
    m_selector = new KisVisualColorSelector(mainWidget);
    connect(m_selector, SIGNAL(sigNewColor(KoColor)), this, SLOT(slotColorSelected(KoColor)));
    mainWidget->layout()->addWidget(m_selector);
    setWidget(mainWidget);
    setEnabled(false);
}

void WGColorSelectorDock::setCanvas(KoCanvasBase *canvas)
{
    if (m_canvas.data() == canvas)
    {
        // not sure why setCanvas gets called 3 times for new canvas, just skip
        return;
    }
    if (m_canvas) {
        disconnectFromCanvas();
    }
    m_canvas = qobject_cast<KisCanvas2*>(canvas);
    if (m_canvas) {
        KoColorDisplayRendererInterface *dri = m_canvas->displayColorConverter()->displayRendererInterface();
        connect(dri, SIGNAL(displayConfigurationChanged()), this, SLOT(slotDisplayConfigurationChanged()));
        connect(m_canvas->resourceManager(), SIGNAL(canvasResourceChanged(int,QVariant)),
                this, SLOT(slotCanvasResourceChanged(int,QVariant)));
    }
    setEnabled(canvas != 0);
}

void WGColorSelectorDock::unsetCanvas()
{
    setEnabled(false);
    m_canvas = 0;
}

void WGColorSelectorDock::disconnectFromCanvas()
{
    m_canvas->disconnectCanvasObserver(this);
    m_canvas->displayColorConverter()->displayRendererInterface()->disconnect(this);
    m_canvas = 0;
}

void WGColorSelectorDock::slotDisplayConfigurationChanged()
{
    m_selector->slotSetColorSpace(m_canvas->displayColorConverter()->paintingColorSpace());
    // TODO: use m_viewManager->canvasResourceProvider()->fgColor();
    m_selector->slotSetColor(m_canvas->resourceManager()->foregroundColor());
}

void WGColorSelectorDock::slotColorSelected(const KoColor &color)
{
    bool selectingBg = false;
    if (selectingBg) {
        m_canvas->resourceManager()->setBackgroundColor(color);
    }
    else {
        m_canvas->resourceManager()->setForegroundColor(color);
    }
}

void WGColorSelectorDock::slotCanvasResourceChanged(int key, const QVariant &value)
{
    bool selectingBg = false;
    switch (key) {
    case KoCanvasResource::ForegroundColor:
        if (!selectingBg) {
            m_selector->slotSetColor(value.value<KoColor>());
        }
        break;
    case KoCanvasResource::BackgroundColor:
        if (selectingBg) {
            m_selector->slotSetColor(value.value<KoColor>());
        }
    default:
        break;
    }
}
