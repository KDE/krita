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

KisFiltersListView::KisFiltersListView(KisPaintDeviceImplSP device, QWidget* parent, const char * name) : KIconView(parent, name) , m_original(device)
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
}

void KisFiltersListView::setLayer(KisLayerSP layer) {
    KisPaintLayer* pl = dynamic_cast<KisPaintLayer*>(layer.data());
    if(pl == 0)
        return;
    KisPaintDeviceImplSP npd = pl->paintDevice();
    if(npd!= m_original)
    {
        m_original = npd;
        buildPreview();
    }
}


void KisFiltersListView::buildPreview()
{
    
    if(m_original== 0)
        return;
    
    // Check which filters support painting

    // Create a paint layer -- if this is not a paint layer, exit. This very ugly, refactore for 2.0. XXX
//     m_thumb = new KisPaintLayer( *dynamic_cast<KisPaintLayer*>(m_layer.data()) );
//     if (m_thumb == 0)
//         return;
    
    m_imgthumb = new KisImage(0, m_original->exactBounds().width(), m_original->exactBounds().height(), m_original->colorSpace(), "thumbnail");
    m_thumb = new KisPaintLayer( m_imgthumb, "thumbnail", 255, new KisPaintDeviceImpl(*m_original));
    
    
    m_imgthumb->addLayer(m_thumb.data(), m_imgthumb->rootLayer(), 0);
    double sx = 100./m_thumb->exactBounds().width();
    double sy = 100./m_thumb->exactBounds().height();
    m_imgthumb->scale(sx, sy, 0, new KisMitchellFilterStrategy());

    KisIDList l = KisFilterRegistry::instance()->listKeys();
    KisIDList::iterator it;
    it = l.begin();
    // Iterate over the list of filters
    for (it = l.begin(); it !=  l.end(); ++it) {
        KisFilterSP f = KisFilterRegistry::instance()->get(*it);
        // Check if filter support the preview
        if (f -> supportsPreview()) {
            std::list<KisFilterConfiguration*> configlist = f->listOfExamplesConfiguration((KisPaintDeviceImplSP)m_thumb->paintDevice());
            // apply the filter for each of example of configuration
            for(std::list<KisFilterConfiguration*>::iterator itc = configlist.begin();
                         itc != configlist.end(); itc++)
            {
                // Creates a new image for this preview
                KisImageSP m_imagethumbPreview = new KisImage(0, m_imgthumb->width(), m_imgthumb->height(), m_imgthumb->colorSpace(), "preview");
                // Creates a copy of the preview
                KisPaintLayerSP thumbPreview = new KisPaintLayer(*m_thumb);
                m_imagethumbPreview->addLayer(thumbPreview.data(), m_imagethumbPreview->rootLayer(), 0);
                // Apply the filter
                f->disableProgress();
                f->process(m_thumb->paintDevice(), thumbPreview->paintDevice(),*itc, m_imagethumbPreview->bounds());
                // Add the preview to the list
                QImage qm_image =  thumbPreview->paintDevice()->convertToQImage(0);
                new KisFiltersIconViewItem( this, (*it).name(), QPixmap(qm_image), *it, f, *itc );
            }
        }
    }
}

