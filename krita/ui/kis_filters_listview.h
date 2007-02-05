/*
 * This file is part of Krita
 *
 * Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 * Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include <QEvent>
#include <QPixmap>
#include <QListWidget>
#include <QListWidgetItem>
#include <QWidget>

#include <threadweaver/ThreadWeaver.h>
#include <threadweaver/Job.h>
#include "KoID.h"

#include <krita_export.h>
#include "kis_types.h"
#include "kis_layer.h"
#include "kis_paint_device.h"

class KisView;
class KisFilter;
class KisFilterConfiguration;
class KisPreviewView;
class KisFiltersIconViewItem;
class KisFiltersListView;

using namespace ThreadWeaver;

class KRITAUI_EXPORT KisFiltersIconViewItem : public QListWidgetItem
{

public:

    KisFiltersIconViewItem( KisFilter * filter, KisFilterConfiguration * config )
        : QListWidgetItem()
        , m_filter( filter )
        , m_config( config )
        {
        }

    KisFilter * filter() { return m_filter; }
    KisFilterConfiguration * filterConfiguration() { return m_config; }
private:

    KisFilter * m_filter;
    KisFilterConfiguration * m_config;
};

class KRITAUI_EXPORT KisFiltersListView : public QListWidget {

    Q_OBJECT

public:

    KisFiltersListView(QWidget* parent, bool filterForAdjustmentLayers = false);

    KisFiltersListView(KisLayerSP layer, QWidget* parent, bool filterForAdjustmentLayers = false) KDE_DEPRECATED;

    KisFiltersListView(KisPaintDeviceSP layer, QWidget* parent, bool filterForAdjustmentLayers = false);

    ~KisFiltersListView();

private:

    void init();

public:

    void setLayer(KisLayerSP layer);
    void setProfile(KoColorProfile * profile) { m_profile = profile; };
    void setPaintDevice(KisPaintDeviceSP pd);
    void buildPreviews();
    void setCurrentFilter(KoID filter);


private slots:

    void itemDone( Job * );

private:

    KisPaintDeviceSP m_original;
    KisImageSP m_imgthumb;
    KisPaintDeviceSP m_thumb;
    KoColorProfile * m_profile;
    bool m_filterForAdjustmentLayers;

    ThreadWeaver::Weaver * m_weaver;

};

#endif
