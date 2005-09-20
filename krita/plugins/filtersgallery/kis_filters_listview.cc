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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "kis_filters_listview.h"

#include "kis_types.h"
#include "kis_view.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_filter.h"
#include "kis_filter_strategy.h"

namespace Krita {
namespace Plugins {
namespace FiltersGallery {

KisFiltersListView::KisFiltersListView(KisView* view, QWidget* parent) : KIconView(parent) , m_view(view)
{
    buildPreview();
    setItemsMovable(false);
    setSelectionMode(QIconView::Single);
}

void KisFiltersListView::buildPreview()
{
    // Check which filters support painting
    KisImageSP img = m_view->getCanvasSubject()->currentImg();
    KisLayerSP activeLayer = img->activeLayer();
    m_thumb = new KisLayer(*activeLayer);
    m_imgthumb = new KisImage(0, m_thumb->exactBounds().width(), m_thumb->exactBounds().height(), m_thumb->colorSpace(), "thumbnail");
    m_imgthumb->add(m_thumb,0);
    double sx = 100./m_thumb->exactBounds().width();
    double sy = 100./m_thumb->exactBounds().height();
    m_imgthumb->scale(sx, sy, 0, new KisMitchellFilterStrategy());
    
    KisIDList l = KisFilterRegistry::instance()->listKeys();
    KisIDList::iterator it;
    it = l.begin();
    for (it = l.begin(); it !=  l.end(); ++it) {
        KisFilterSP f = KisFilterRegistry::instance()->get(*it);
        
        if (f -> supportsPreview()) {
            std::list<KisFilterConfiguration*> configlist = f->listOfExamplesConfiguration((KisPaintDeviceImplSP)m_thumb);
            // apply the filter
            for(std::list<KisFilterConfiguration*>::iterator itc = configlist.begin();
                         itc != configlist.end(); itc++)
            {
                KisImageSP imgthumbPreview = new KisImage(0, m_imgthumb->width(), m_imgthumb->height(), m_imgthumb->colorSpace(), "preview");
                KisLayerSP thumbPreview = new KisLayer(*m_thumb/*imgthumbPreview,"",50*/);
                imgthumbPreview->add(thumbPreview,0);
                f->disableProgress();
                f->process((KisPaintDeviceImplSP)m_thumb, (KisPaintDeviceImplSP)thumbPreview,*itc, imgthumbPreview->bounds());
                QImage qimg =  thumbPreview->convertToQImage(thumbPreview->profile());
                new KisFiltersIconViewItem( this, (*it).name(),
                                                                        QPixmap(qimg), *it, f, *itc );
            }
            
        }
    }

}

// KisImageSP KisFiltersListView::image() { return m_imgthumb; }
// KisLayerSP KisFiltersListView::layer() { return m_thumb; }


}
}
}
