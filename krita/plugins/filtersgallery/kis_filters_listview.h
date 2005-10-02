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
#ifndef _KIS_IMAGE_LIST_VIEW_H_
#define _KIS_IMAGE_LIST_VIEW_H_

#include <kiconview.h>

#include "kis_id.h"
#include "kis_types.h"

class KisView;
class KisFilter;
class KisFilterConfiguration;
class KisPreviewView;

namespace Krita {
namespace Plugins {
namespace FiltersGallery {

    class KisFiltersIconViewItem : public QIconViewItem {
        public:
            KisFiltersIconViewItem( QIconView * parent, const QString & text, const QPixmap & icon, KisID id, KisFilter* filter, KisFilterConfiguration* filterConfig ) : QIconViewItem(parent, text, icon), m_id(id), m_filter(filter), m_filterconfig(filterConfig)
            {
            }
            inline KisID id() { return m_id; }
            inline KisFilter* filter() { return m_filter; }
                        inline void setFilterConfiguration(KisFilterConfiguration* fc) { m_filterconfig = fc; }
        private:
            KisID m_id;
            KisFilter* m_filter;
            KisFilterConfiguration* m_filterconfig;
    };
    class KisFiltersListView : public KIconView {
        public:
            KisFiltersListView(KisView* view, QWidget* parent);
        public:
            void buildPreview();
//             KisImageSP image();
//             KisLayerSP layer();
        private:
            KisView* m_view;
            KisImageSP m_imgthumb;
            KisLayerSP m_thumb;
    };

}
}
}
#endif
