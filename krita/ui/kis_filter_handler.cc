/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#include "kis_filter_handler.h"

#include <QApplication>

#include "kis_doc2.h"
#include "kis_filter.h"
#include "kis_filter_dialog.h"
#include "kis_layer.h"
#include "kis_transaction.h"
#include "kis_view2.h"

struct KisFilterHandler::Private {
    KisFilterSP filter;
    KisView2* view;
};

KisFilterHandler::KisFilterHandler(KisFilterSP f, KisView2* view) : d(new Private)
{
    d->filter = f;
    d->view = view;
}

void KisFilterHandler::showDialog()
{
    KisFilterDialog dialog( d->view , d->view->activeLayer());
    dialog.setFilter( d->filter );
    connect(&dialog, SIGNAL(sigPleaseApplyFilter(KisLayerSP, KisFilterConfiguration*)),
            SLOT(apply(KisLayerSP, KisFilterConfiguration*)));
    dialog.exec();
}

void KisFilterHandler::reapply()
{
}

void KisFilterHandler::apply(KisLayerSP layer, KisFilterConfiguration* config)
{
    if( not layer ) return;
    
    KisPaintDeviceSP dev = layer->paintDevice();
    
    QRect r1 = dev->extent();
    QRect r2 = layer->image()->bounds();

    // Filters should work only on the visible part of an image.
    QRect rect = r1.intersect(r2);

    if (dev->hasSelection()) {
        QRect r3 = dev->selection()->selectedExactRect();
        rect = rect.intersect(r3);
    }
    
    KisTransaction * cmd = 0;
    if (layer->image()->undo()) cmd = new KisTransaction(d->filter->name(), dev);

    if ( !d->filter->supportsThreading() ) {
        d->filter->process(dev, rect, config);
    }
    else {
        // Chop up in rects.
        d->filter->process(dev, rect, config);
    }
    kDebug() << d->filter->cancelRequested() << endl;
    kDebug() << cmd << endl;
    if (d->filter->cancelRequested()) {
    } else {
        dev->setDirty(rect);
        d->view->document()->setModified(true);
        if (cmd) d->view->document()->addCommand(cmd);
        d->filter->disableProgress();
        QApplication::restoreOverrideCursor();
        d->filter->saveToBookmark(KisFilter::ConfigLastUsed.id(), config);
    }
}


#include "kis_filter_handler.moc"
