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

#include "kis_types.h"
#include "kis_view.h"
#include "kis_image.h"
#include "kis_paint_layer.h"
#include "kis_filter.h"
#include "kis_filter_strategy.h"

namespace Krita {
namespace Plugins {
namespace FiltersGallery {

KisFiltersListView::KisFiltersListView(QWidget* parent, const char* name) : KIconView(parent, name), m_view(0)
{
    init();
}

KisFiltersListView::KisFiltersListView(KisView* view, QWidget* parent) : KIconView(parent) , m_view(view)
{
    buildPreview();
    init();
}
void KisFiltersListView::init()
{
    setItemsMovable(false);
    setSelectionMode(QIconView::Single);
    setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding ));
    setMinimumWidth(240);
    if(layout ())
    {
        kdDebug() << " hassssssssssssssssssssssssssssssssssssssssssssss layooooooooooooooooouuuuuuuuuuuuuutttttttttttttttttt" << endl;
    }
}

void KisFiltersListView::buildPreview()
{
    if(m_view == 0)
        return;
    // Check which filters support painting
    KisImageSP img = m_view->getCanvasSubject()->currentImg();
    KisPaintLayerSP activeLayer = dynamic_cast<KisPaintLayer*>( img->activeLayer().data());
    if(activeLayer == 0)
        return; // TODO: warn the user and fix the filters engine
    m_thumb = new KisPaintLayer( *activeLayer );
    m_imgthumb = new KisImage(0, m_thumb->exactBounds().width(), m_thumb->exactBounds().height(), m_thumb->paintDevice()->colorSpace(), "thumbnail");
    m_imgthumb->addLayer(m_thumb.data(), m_imgthumb->rootLayer(), 0);
    double sx = 100./m_thumb->exactBounds().width();
    double sy = 100./m_thumb->exactBounds().height();
    m_imgthumb->scale(sx, sy, 0, new KisMitchellFilterStrategy());
    
    KisIDList l = KisFilterRegistry::instance()->listKeys();
    KisIDList::iterator it;
    it = l.begin();
    for (it = l.begin(); it !=  l.end(); ++it) {
        KisFilterSP f = KisFilterRegistry::instance()->get(*it);
        
        if (f -> supportsPreview()) {
            std::list<KisFilterConfiguration*> configlist = f->listOfExamplesConfiguration((KisPaintDeviceImplSP)m_thumb->paintDevice());
            // apply the filter
            for(std::list<KisFilterConfiguration*>::iterator itc = configlist.begin();
                         itc != configlist.end(); itc++)
            {
                KisImageSP imgthumbPreview = new KisImage(0, m_imgthumb->width(), m_imgthumb->height(), m_imgthumb->colorSpace(), "preview");
                KisPaintLayerSP thumbPreview = new KisPaintLayer(*m_thumb/*imgthumbPreview,"",50*/);
                imgthumbPreview->addLayer(thumbPreview.data(), imgthumbPreview->rootLayer(), 0);
                f->disableProgress();
                f->process(m_thumb->paintDevice(), thumbPreview->paintDevice(),*itc, imgthumbPreview->bounds());
                QImage qimg =  thumbPreview->paintDevice()->convertToQImage(0);
                new KisFiltersIconViewItem( this, (*it).name(), QPixmap(qimg), *it, f, *itc );
            }
            
        }
    }

}

}
}
}
