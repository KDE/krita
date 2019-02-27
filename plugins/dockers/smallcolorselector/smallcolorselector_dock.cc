/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "smallcolorselector_dock.h"

#include <klocalizedstring.h>
#include "kis_canvas2.h"

#include "kis_small_color_widget.h"
#include "kis_canvas_resource_provider.h"

#include <KoColorSpaceRegistry.h>

#include <QVBoxLayout>

SmallColorSelectorDock::SmallColorSelectorDock()
        : QDockWidget()
        , m_canvas(0)
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    m_smallColorWidget = new KisSmallColorWidget(this);
    layout->addWidget(m_smallColorWidget, 1);
    page->setLayout(layout);

    setWidget(page);

    m_smallColorWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    connect(m_smallColorWidget, SIGNAL(colorChanged(KoColor)),
            this, SLOT(colorChangedProxy(KoColor)));

    connect(this, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            m_smallColorWidget, SLOT(update()));

    setWindowTitle(i18n("Small Color Selector"));
}

void SmallColorSelectorDock::setCanvas(KoCanvasBase * canvas)
{
    setEnabled(canvas != 0);

    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this);
        m_smallColorWidget->setColor(KoColor(Qt::black, KoColorSpaceRegistry::instance()->rgb8()));
        m_smallColorWidget->setDisplayColorConverter(0);
    }
    m_canvas = canvas;
    if (m_canvas && m_canvas->resourceManager()) {
        connect(m_canvas->resourceManager(), SIGNAL(canvasResourceChanged(int,QVariant)),
                this, SLOT(canvasResourceChanged(int,QVariant)));

        KisCanvas2 *kisCanvas = dynamic_cast<KisCanvas2*>(canvas);
        m_smallColorWidget->setDisplayColorConverter(kisCanvas->displayColorConverter());
        m_smallColorWidget->setColor(m_canvas->resourceManager()->foregroundColor());
    }
}

void SmallColorSelectorDock::colorChangedProxy(const KoColor& c)
{
    if (m_canvas)
        m_canvas->resourceManager()->setForegroundColor(c);
}

void SmallColorSelectorDock::canvasResourceChanged(int key, const QVariant& v)
{
    if (key == KoCanvasResourceProvider::ForegroundColor) {
        m_smallColorWidget->setColor(v.value<KoColor>());
    }
}

