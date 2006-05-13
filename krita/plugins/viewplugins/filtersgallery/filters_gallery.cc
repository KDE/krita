/*
 * This file is part of Krita
 *
 * Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "filters_gallery.h"

#include <QApplication>

#include <kdebug.h>
#include <kgenericfactory.h>
#include <kstandarddirs.h>

#include <kopalettemanager.h>

#include <kis_colorspace_factory_registry.h>
#include "kis_progress_display_interface.h"
#include <kis_dlg_filtersgallery.h>
#include <kis_doc.h>
#include <kis_filter.h>
#include <kis_filters_listview.h>
#include <kis_meta_registry.h>
#include <kis_paint_device.h>
#include <kis_selection.h>
#include <kis_view.h>
#include <kis_transaction.h>

namespace Krita {
namespace Plugins {
namespace FiltersGallery {

typedef KGenericFactory<KritaFiltersGallery> KritaFiltersGalleryFactory;
K_EXPORT_COMPONENT_FACTORY( kritafiltersgallery, KritaFiltersGalleryFactory( "krita" ) )

KritaFiltersGallery::KritaFiltersGallery(QObject *parent, const QStringList &)
        : KParts::Plugin(parent)
{
    if ( parent->inherits("KisView") )
    {
        setInstance(KritaFiltersGallery::instance());
        setXMLFile(locate("data","kritaplugins/kritafiltersgallery.rc"), true);

        m_view = (KisView*) parent;

        (void) new KAction(i18n("&Filters Gallery"), 0, 0, this, SLOT(showFiltersGalleryDialog()), actionCollection(), "krita_filters_gallery");

        // Add a docker with the list of filters
//         QImage img;
//         if(img.load(locate("data","krita/images/previewfilter.png")))
//         {
//            KisPaintDeviceSP preview = new KisPaintDevice(KisMetaRegistry::instance()->csRegistry()->getColorSpace(KisID("RGBA",""),""));
//            preview->convertFromQImage(img,"");
//            m_view->canvasSubject()->paletteManager()->addWidget(new KisFiltersListView(preview,m_view),"filterslist",krita::EFFECTSBOX, 0);
//         }

    }


}

KritaFiltersGallery::~KritaFiltersGallery()
{
}

void KritaFiltersGallery::showFiltersGalleryDialog()
{
    KisDlgFiltersGallery dlg(m_view, m_view);
    if (dlg.exec())
    {
        QApplication::setOverrideCursor( Qt::WaitCursor );

        KisFilter* filter = dlg.currentFilter();
        if(filter )
        {
            KisImageSP img = m_view->canvasSubject()->currentImg();
            if (!img) return;

            KisPaintDeviceSP dev = img->activeDevice();
            if (!dev) return;
            QRect r1 = dev->exactBounds();
            QRect r2 = img->bounds();

            QRect rect = r1.intersect(r2);

            if (dev->hasSelection()) {
                QRect r3 = dev->selection()->selectedExactRect();
                rect = rect.intersect(r3);
            }
            KisFilterConfiguration* config = filter->configuration( dlg.currentConfigWidget());

            filter->enableProgress();
            m_view->canvasSubject()->progressDisplay()->setSubject(filter, true, true);
            filter->setProgressDisplay(m_view->canvasSubject()->progressDisplay());

            KisTransaction * cmd = new KisTransaction(filter->id().name(), dev);

            filter->process(dev,dev, config, rect);

            delete config;
            if (filter->cancelRequested()) {
                cmd->unexecute();
                delete cmd;
            } else {
                dev->setDirty(rect);
                if (img->undo())
                    img->undoAdapter()->addCommand(cmd);
                else
                    delete cmd;
            }
            filter->disableProgress();
            QApplication::restoreOverrideCursor();

        }
    }
}

}
}
}

#include "filters_gallery.moc"
