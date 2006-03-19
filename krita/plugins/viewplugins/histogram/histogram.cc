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


#include <math.h>

#include <stdlib.h>

#include <qslider.h>
#include <qpoint.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>

#include <kis_doc.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_paint_device.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_view.h>
#include <kis_selection.h>

#include "histogram.h"
#include "dlg_histogram.h"
#include "kis_colorspace.h"
#include "kis_histogram.h"

typedef KGenericFactory<Histogram> HistogramFactory;
K_EXPORT_COMPONENT_FACTORY( kritahistogram, HistogramFactory( "krita" ) )

Histogram::Histogram(QObject *parent, const char *name, const QStringList &)
    : KParts::Plugin(parent, name)
{

    if ( parent->inherits("KisView") ) {

        setInstance(HistogramFactory::instance());
        setXMLFile(locate("data","kritaplugins/histogram.rc"), true);

        m_action = new KAction(i18n("&Histogram..."), 0, 0, this, SLOT(slotActivated()), actionCollection(), "histogram");

        m_view = (KisView*) parent;
        if (KisImageSP img = m_view->canvasSubject()->currentImg()) {
            connect(img, SIGNAL(sigLayersChanged(KisGroupLayerSP)), this, SLOT(slotLayersChanged()));
            connect(img, SIGNAL(sigLayerAdded(KisLayerSP)), this, SLOT(slotLayersChanged()));
            connect(img, SIGNAL(sigLayerActivated(KisLayerSP)), this, SLOT(slotLayersChanged()));
            connect(img, SIGNAL(sigLayerPropertiesChanged(KisLayerSP)), this, SLOT(slotLayersChanged()));
            connect(img, SIGNAL(sigLayerRemoved(KisLayerSP, KisGroupLayerSP, KisLayerSP)),
                    this, SLOT(slotLayersChanged()));
            connect(img, SIGNAL(sigLayerMoved(KisLayerSP, KisGroupLayerSP, KisLayerSP)),
                    this, SLOT(slotLayersChanged()));
            m_img = img;
        }
    }
}

Histogram::~Histogram()
{
}

void Histogram::slotLayersChanged() {
    m_action->setEnabled(m_img && m_img->activeLayer() && m_img->activeLayer()->visible());
}

void Histogram::slotActivated()
{
    DlgHistogram * dlgHistogram = new DlgHistogram(m_view, "Histogram");
    Q_CHECK_PTR(dlgHistogram);

    KisPaintDeviceSP dev = m_view->canvasSubject()->currentImg()->activeDevice();
    if (dev)
        dlgHistogram->setPaintDevice(dev);

    if (dlgHistogram->exec() == QDialog::Accepted) {
        // Do nothing; this is an informational dialog
    }
    delete dlgHistogram;
}

#include "histogram.moc"

