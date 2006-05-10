/*
 * This file is part of Krita
 *
 * Copyright (c) 2005-2006 Cyrille Berger <cberger@cberger.net>
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
#include "kis_dlg_filtersgallery.h"

#include <q3groupbox.h>
#include <QLayout>
#include <QLabel>
#include <qdatetime.h>

#include <kis_filter.h>
#include <kis_filter_config_widget.h>
#include <kis_filters_listview.h>
#include <kis_paint_device.h>
#include <kis_paint_layer.h>
#include <kis_previewwidget.h>
#include <kis_transaction.h>
#include <kis_types.h>
#include <kis_view.h>

#include "kis_wdg_filtersgallery.h"

namespace Krita {
namespace Plugins {
namespace FiltersGallery {


KisDlgFiltersGallery::KisDlgFiltersGallery(KisView* view, QWidget* parent,const char *name)
  : KDialogBase(parent,name, true,i18n("Filters Gallery"), Ok | Cancel), m_view(view),m_currentConfigWidget(0), m_currentFilter(0)
{
   // Initialize main widget
    m_widget = new KisWdgFiltersGallery(this);
    m_widget->filtersList->setLayer(view->canvasSubject()->currentImg()->activeLayer());
    m_widget->filtersList->setProfile(view->canvasSubject()->monitorProfile());
    
    setMainWidget(m_widget);
    // Initialize filters list
    connect(m_widget->filtersList , SIGNAL(selectionChanged(Q3IconViewItem*)), this, SLOT(selectionHasChanged(Q3IconViewItem* )));
    // Initialize configWidgetHolder
    m_widget->configWidgetHolder->setColumnLayout ( 0, Qt::Horizontal );
    //m_widget->configWidgetHolder->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    // Initialize preview widget
    
    if (m_view->canvasSubject()->currentImg() && m_view->canvasSubject()->currentImg()->activeDevice())
    {
        m_widget->previewWidget->slotSetDevice( m_view->canvasSubject()->currentImg()->activeDevice() );
    }
    connect( m_widget->previewWidget, SIGNAL(updated()), this, SLOT(refreshPreview()));
    resize( minimumSizeHint());
    m_widget->previewWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    m_labelNoCW = new QLabel(i18n("No configuration options are available for this filter."), m_widget->configWidgetHolder);
    m_widget->configWidgetHolder->layout()->add(m_labelNoCW);
    m_labelNoCW->hide();
}

KisDlgFiltersGallery::~KisDlgFiltersGallery()
{
}

void KisDlgFiltersGallery::selectionHasChanged ( Q3IconViewItem * item )
{
    KisFiltersIconViewItem* kisitem = (KisFiltersIconViewItem*) item;
    m_currentFilter = kisitem->filter();
    if(m_currentConfigWidget != 0)
    {
        m_widget->configWidgetHolder->layout()->remove(m_currentConfigWidget);
        delete m_currentConfigWidget;
        m_currentConfigWidget = 0;
    } else {
        m_labelNoCW->hide();
    }
    KisImageSP img = m_view->canvasSubject()->currentImg();
    KisPaintLayerSP activeLayer = KisPaintLayerSP(dynamic_cast<KisPaintLayer*>(img->activeLayer().data()));
    
    if (activeLayer)
       m_currentConfigWidget = m_currentFilter->createConfigurationWidget(m_widget->configWidgetHolder, activeLayer->paintDevice());
    
    if(m_currentConfigWidget != 0) {
        //m_currentConfigWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_widget->configWidgetHolder->layout()->add(m_currentConfigWidget);
        m_currentConfigWidget->show();
        connect(m_currentConfigWidget, SIGNAL(sigPleaseUpdatePreview()), this, SLOT(slotConfigChanged()));
    }
    else {
        m_labelNoCW->show();
    }
    
    refreshPreview();
}

void KisDlgFiltersGallery::slotConfigChanged()
{
    if(m_widget->previewWidget->getAutoUpdate())
    {
        refreshPreview();
    } else {
        m_widget->previewWidget->needUpdate();
    }
}


void KisDlgFiltersGallery::refreshPreview( )
{
    KisPaintDeviceSP layer =  m_widget->previewWidget->getDevice();

    KisTransaction cmd("Temporary transaction", layer);
    KisFilterConfiguration* config = m_currentFilter->configuration(m_currentConfigWidget);

    QRect rect = layer->exactBounds();
    m_currentFilter->process(layer, layer, config, rect);
    m_widget->previewWidget->slotUpdate();
    cmd.unexecute();

}

}
}
}

#include "kis_dlg_filtersgallery.moc"
