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
#include <KoCanvasBase.h>

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
    layout->addWidget(m_smallColorWidget);
    layout->addStretch(1);
    setWidget(page);
    m_smallColorWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    connect(m_smallColorWidget, SIGNAL(colorChanged(const QColor&)),
            this, SLOT(colorChangedProxy(const QColor&)));

    setWindowTitle(i18n("Small Color Selector"));
}

void SmallColorSelectorDock::setCanvas(KoCanvasBase * canvas)
{
    setEnabled(canvas != 0);

    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this);
        m_smallColorWidget->setQColor(Qt::black);
    }
    m_canvas = canvas;
    if (m_canvas && m_canvas->resourceManager()) {
        connect(m_canvas->resourceManager(), SIGNAL(canvasResourceChanged(int, const QVariant&)),
                this, SLOT(canvasResourceChanged(int, const QVariant&)));
        m_smallColorWidget->setQColor(m_canvas->resourceManager()->foregroundColor().toQColor());
    }
}

void SmallColorSelectorDock::colorChangedProxy(const QColor& c)
{
    if (m_canvas)
        m_canvas->resourceManager()->setForegroundColor(KoColor(c , KoColorSpaceRegistry::instance()->rgb8()));
}

void SmallColorSelectorDock::canvasResourceChanged(int key, const QVariant& v)
{
    if (key == KoCanvasResourceProvider::ForegroundColor) {
        m_smallColorWidget->setQColor(v.value<KoColor>().toQColor());
    }
}

