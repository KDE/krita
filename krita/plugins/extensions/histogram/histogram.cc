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
#include <kiconloader.h>
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
#include "kis_layer_manager.h"

K_PLUGIN_FACTORY(HistogramFactory, registerPlugin<Histogram>();)
K_EXPORT_PLUGIN(HistogramFactory("krita"))

Histogram::Histogram(QObject *parent, const QVariantList &)
        : KParts::Plugin(parent)
{
    if (parent->inherits("KisView2")) {

        setComponentData(HistogramFactory::componentData());

        setXMLFile(KStandardDirs::locate("data", "kritaplugins/histogram.rc"),
                   true);

        m_action  = new KAction(i18n("&Histogram..."), this);
        actionCollection()->addAction("histogram", m_action);
        connect(m_action,  SIGNAL(triggered()), this, SLOT(slotActivated()));

        m_view = (KisView2*) parent;
        if (KisImageWSP image = m_view->image()) {
            connect(image.data(), SIGNAL(sigLayersChanged(KisGroupLayerSP)), SLOT(slotLayersChanged()));
            connect(image.data(), SIGNAL(sigNodeHasBeenAdded(KisNode *, int)), SLOT(slotLayersChanged()));
            connect(m_view->layerManager(), SIGNAL(sigLayerActivated(KisLayerSP)), SLOT(slotLayersChanged()));
            connect(image.data(), SIGNAL(sigLayerPropertiesChanged(KisLayerSP)), SLOT(slotLayersChanged()));
            connect(image.data(), SIGNAL(sigNodeHasBeenRemoved(KisNode *, int)), SLOT(slotLayersChanged()));
            m_image = image.data();
        }
    }
}

Histogram::~Histogram()
{
}

void Histogram::slotLayersChanged()
{
    m_action->setEnabled(m_image && m_view->layerManager()->activeLayer() && m_view->layerManager()->activeLayer()->visible());
}

void Histogram::slotActivated()
{
    DlgHistogram * dlgHistogram = new DlgHistogram(m_view, "Histogram");
    Q_CHECK_PTR(dlgHistogram);

    KisLayerSP layer = m_view->layerManager()->activeLayer();
    if (layer) {
        KisPaintDeviceSP dev = layer->paintDevice();

        if (dev)
            dlgHistogram->setPaintDevice(dev);

        if (dlgHistogram->exec() == QDialog::Accepted) {
            // Do nothing; this is an informational dialog
        }


    }
    delete dlgHistogram;
}

#include "histogram.moc"

