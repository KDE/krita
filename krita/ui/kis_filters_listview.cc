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

#include "kis_filters_listview.h"
#include <qapplication.h>

#include "kis_cursor.h"
#include "kis_types.h"
#include "kis_image.h"
#include "kis_image.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include "kis_filter.h"
#include "kis_filter_strategy.h"


KisFiltersListView::KisFiltersListView(QWidget* parent, const char* name) : KIconView(parent, name), m_original(0)
{
    init();
}

KisFiltersListView::KisFiltersListView(KisLayerSP layer, QWidget* parent, const char * name) : KIconView(parent, name) , m_original(0)
{
    KisPaintLayer* pl = dynamic_cast<KisPaintLayer*>(layer.data());
    if(pl != 0)
    {
        m_original = pl->paintDevice();
        buildPreview();
    }
    init();
}

KisFiltersListView::KisFiltersListView(KisPaintDeviceSP device, QWidget* parent, const char * name) : KIconView(parent, name) , m_original(device)
{
    buildPreview();
    init();
}

void KisFiltersListView::init()
{
    setCaption(i18n("Filters List"));
    setItemsMovable(false);
    setSelectionMode(QIconView::Single);
    setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding ));
    setMinimumWidth(240);
}

void KisFiltersListView::setLayer(KisLayerSP layer) {
    KisPaintLayer* pl = dynamic_cast<KisPaintLayer*>(layer.data());
    if(pl == 0)
        return;
    KisPaintDeviceSP npd = pl->paintDevice();
    if(npd!= m_original)
    {
        m_original = npd;
        buildPreview();
    }
}

void KisFiltersListView::setCurrentFilter(KisID filter)
{
    
    kdDebug() << "Current is: " << currentItem() << ", Set current to: " << filter.name() << "\n";
    setCurrentItem(findItem(filter.name()));
    kdDebug() << "Current is now: " << currentItem() << "\n";
    
}

KisPaintDeviceSP createThumbnail(KisPaintDeviceSP dev, Q_INT32 w, Q_INT32 h)
{
    kdDebug() << "Going to create a thumbnail size " << w << ", " << h << endl;
    KisPaintDeviceSP thumbnail = new KisPaintDevice(dev->colorSpace(), "thumbnail");
    
    int srcw, srch;
    if( dev->image() )
    {
        srcw = dev->image()->width();
        srch = dev->image()->height();
    }
    else
    {
        const QRect e = dev->extent();
        srcw = e.width();
        srch = e.height();
    }

    kdDebug() << "Source size: " << srcw << ", " << srch << "\n";
    
    if (w > srcw)
    {
        w = srcw;
        h = Q_INT32(double(srcw) / w * h);
    }
    if (h > srch)
    {
        h = srch;
        w = Q_INT32(double(srch) / h * w);
    }

    if (srcw > srch)
        h = Q_INT32(double(srch) / srcw * w);
    else if (srch > srcw)
        w = Q_INT32(double(srcw) / srch * h);

    kdDebug() << "w has become: " << w << ", h: " << h << endl;
    
    for (Q_INT32 y=0; y < h; ++y) {
        Q_INT32 iY = (y * srch ) / h;
        for (Q_INT32 x=0; x < w; ++x) {
            Q_INT32 iX = (x * srcw ) / w;
            
            thumbnail->setPixel(x, y, dev->colorAt(iX, iY));
        }
    }
    
    return thumbnail;

}

void KisFiltersListView::buildPreview()
{
    
    if(m_original== 0)
        return;

    QApplication::setOverrideCursor(KisCursor::waitCursor());
    m_thumb = createThumbnail(m_original, 150, 100);
    QRect bounds = m_thumb->exactBounds();
    kdDebug() << "bound: " << bounds.x() << ", " << bounds.y() << ", " << bounds.width() << ", " << bounds.height() << endl;
    KisIDList l = KisFilterRegistry::instance()->listKeys();
    KisIDList::iterator it;
    it = l.begin();
    // Iterate over the list of filters
    for (it = l.begin(); it !=  l.end(); ++it) {
        KisFilterSP f = KisFilterRegistry::instance()->get(*it);
        kdDebug() << "Going to create preview for filter " << f->id().name() << "\n";
        // Check if filter support the preview and work with the current colorspace
        if (f -> supportsPreview() && f->workWith( m_original->colorSpace() ) ) {
            std::list<KisFilterConfiguration*> configlist = f->listOfExamplesConfiguration(m_thumb);
            // apply the filter for each of example of configuration
            for(std::list<KisFilterConfiguration*>::iterator itc = configlist.begin();
                         itc != configlist.end(); itc++)
            {
                kdDebug() << "Copying preview paint device\n";
                KisPaintDeviceSP thumbPreview = new KisPaintDevice(*m_thumb);
                // Apply the filter
                f->disableProgress();
                kdDebug() << "Processing thumbnail\n";
                f->process(m_thumb, thumbPreview, *itc, bounds);
                kdDebug() << "Done processing thumbnail\n";
                // Add the preview to the list
                QImage qm_image =  thumbPreview->convertToQImage(0);
                new KisFiltersIconViewItem( this, (*it).name(), QPixmap(qm_image), *it, f, *itc );
            }
        }
    }

    QApplication::restoreOverrideCursor();
}

