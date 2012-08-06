/*
 * histogram.h -- Part of Krita
 *
 * Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "histogram.h"


#include <math.h>

#include <stdlib.h>

#include <QSlider>
#include <QPoint>

#include <klocale.h>
#include <kcomponentdata.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kis_debug.h>
#include <kpluginfactory.h>
#include <kactioncollection.h>

#include <kis_image.h>
#include <kis_layer.h>
#include <kis_paint_device.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_view2.h>
#include <kis_selection.h>


#include "dlg_histogram.h"
#include "KoColorSpace.h"
#include "kis_histogram.h"
#include "kis_node_manager.h"

K_PLUGIN_FACTORY(HistogramFactory, registerPlugin<Histogram>();)
K_EXPORT_PLUGIN(HistogramFactory("krita"))

Histogram::Histogram(QObject *parent, const QVariantList &)
        : KParts::Plugin(parent)
{
    if (parent->inherits("KisView2")) {
        setXMLFile(KStandardDirs::locate("data", "kritaplugins/histogram.rc"),
                   true);

        m_action  = new KAction(i18n("&Histogram..."), this);
        actionCollection()->addAction("histogram", m_action);
        connect(m_action,  SIGNAL(triggered()), this, SLOT(slotActivated()));

        m_view = (KisView2*) parent;
        m_image = m_view->image();

        if (m_image) {
            connect(m_image, SIGNAL(sigLayersChangedAsync()), SLOT(slotLayersChanged()));
            connect(m_image, SIGNAL(sigNodeAddedAsync(KisNodeSP)), SLOT(slotLayersChanged()));
            connect(m_image, SIGNAL(sigRemoveNodeAsync(KisNodeSP)), SLOT(slotLayersChanged()));

            connect(m_view->nodeManager(), SIGNAL(sigLayerActivated(KisLayerSP)), SLOT(slotLayersChanged()));
        }
    }
}

Histogram::~Histogram()
{
}

void Histogram::slotLayersChanged()
{
    m_action->setEnabled(m_image && m_view->nodeManager()->activeLayer());
}

void Histogram::slotActivated()
{
    DlgHistogram * dlgHistogram = new DlgHistogram(m_view, "Histogram");
    Q_CHECK_PTR(dlgHistogram);

    KisLayerSP layer = m_view->nodeManager()->activeLayer();
    if (layer) {
        KisPaintDeviceSP dev = layer->paintDevice();

        if (dev) {
            dlgHistogram->setPaintDevice(dev, layer->image()->bounds());
        }
        if (dlgHistogram->exec() == QDialog::Accepted) {
            // Do nothing; this is an informational dialog
        }


    }
    delete dlgHistogram;
}

#include "histogram.moc"

