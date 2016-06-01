/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#include "histogramdocker_dock.h"
#include "histogramwidget.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QStatusBar>
#include <klocalizedstring.h>

#include "kis_canvas2.h"
#include <KisViewManager.h>
#include <kis_zoom_manager.h>
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_signal_compressor.h"
#include "kis_histogram_view.h"

HistogramDockerDock::HistogramDockerDock( )
    : QDockWidget(i18n("Histogram"))
    , m_canvas(0)
{
    QWidget *page = new QWidget(this);
    m_layout = new QVBoxLayout(page);

    m_histogramWidget = new KisHistogramView(this);
    m_histogramWidget->setMinimumHeight(50);
    m_layout->addWidget(m_histogramWidget, 1);
    setWidget(page);
}

void HistogramDockerDock::setPaintDevice(KisPaintDeviceSP dev, const QRect &bounds )
{
    auto cs = m_canvas->image()->colorSpace();
    QList<QString> producers = KoHistogramProducerFactoryRegistry::instance()->keysCompatibleWith(cs);
    KoHistogramProducer *producer = KoHistogramProducerFactoryRegistry::instance()->get(producers.at(0))->generate();
    m_histogramWidget->setPaintDevice( dev, producer, bounds );
}

void HistogramDockerDock::setCanvas(KoCanvasBase * canvas)
{
    if(m_canvas == canvas)
        return;

    setEnabled(canvas != 0);

    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this);
        m_canvas->image()->disconnect(this);
    }
    m_canvas = dynamic_cast<KisCanvas2*>(canvas);
}

void HistogramDockerDock::unsetCanvas()
{
    setEnabled(false);
    m_canvas = 0;
}


