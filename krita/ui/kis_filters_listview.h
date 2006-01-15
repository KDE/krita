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
#ifndef _KIS_FILTERS_LIST_VIEW_H_
#define _KIS_FILTERS_LIST_VIEW_H_

#include <kiconview.h>

#include "kis_id.h"
#include "kis_types.h"
#include "kis_layer.h"
#include "kis_paint_device_impl.h"

class KisView;
class KisFilter;
class KisFilterConfiguration;
class KisPreviewView;

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
    KisFiltersListView(QWidget* parent, const char* name = 0);
    KisFiltersListView(KisLayerSP layer, QWidget* parent, const char * name = 0) KDE_DEPRECATED;
    KisFiltersListView(KisPaintDeviceImplSP layer, QWidget* parent, const char * name = 0);
    
private:
    void init();
public:
    void setLayer(KisLayerSP layer) KDE_DEPRECATED;
    inline void setPaintDevice(KisPaintDeviceImplSP pd) {
        if( pd != m_original)
        {
            m_original = pd;
            buildPreview();
        }
    }
    void buildPreview();
private:
    KisPaintDeviceImplSP m_original;
    KisImageSP m_imgthumb;
    KisPaintLayerSP m_thumb;
};

#endif
