/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
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
#include <klocale.h>

#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>

#include <klineedit.h>
#include <klocale.h>

#include "kis_filter_config_widget.h"
#include "kis_transaction.h"
#include "kis_filter.h"
#include "kis_filter_configuration.h"
#include "kis_dlg_adjustment_layer.h"
#include "kis_filters_listview.h"
#include "kis_image.h"
#include "kis_previewwidget.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_filter.h"
#include "kis_filter_configuration.h"

KisDlgAdjustmentLayer::KisDlgAdjustmentLayer(KisImage * img,
                                             const QString & layerName,
                                             const QString & caption,
                                             bool create,
                                             QWidget *parent,
                                             const char *name)
    : KDialogBase(parent, name, true, "", Ok | Cancel)
    , m_image(img)
{
    Q_ASSERT(img);
    
    KisLayerSP activeLayer = 0;
    KisFilterConfiguration * kfc = 0;
    KisFilter * filter = 0;
    KisAdjustmentLayer * adjustmentLayer = 0;
    
    if (create) {
        activeLayer = img->activeLayer();
    } else {
        adjustmentLayer = dynamic_cast<KisAdjustmentLayer*>(activeLayer.data());
        if (adjustmentLayer) {
            KisLayerSP next = adjustmentLayer->nextSibling();
            if (!next) {
                create = true;
                activeLayer = img->activeLayer();
            }

            kfc = adjustmentLayer->filter();
            filter = KisFilterRegistry::instance()->get(kfc->name());
            if (!filter) {
                create = true; // XXX: warning after message freeze
                activeLayer = img->activeLayer();
            }
        }
        else {
            create = true;
            activeLayer = img->activeLayer();
        }
    }

    kdDebug() << "Create: " << create << ", layer: " << activeLayer->name() << ", adj: " << adjustmentLayer << "\n";
    
    KisPaintDeviceSP dev = 0;

    KisPaintLayer * pl = dynamic_cast<KisPaintLayer*>(activeLayer.data());
    if (pl) {
        dev = pl->paintDevice();
    }
    else {
        KisGroupLayer * gl = dynamic_cast<KisGroupLayer*>(activeLayer.data());
        if (gl) {
            dev = gl->projection();
        }
        else {
            KisAdjustmentLayer * al = dynamic_cast<KisAdjustmentLayer*>(activeLayer.data());
            if (al) {
                dev = al->cachedPaintDevice();
            }
        }
    }
    kdDebug() << "Paint device: " << dev << "\n";
    
    // XXX: Do the rest of the property setting stuff
    
    setCaption(caption);
    QWidget * page = new QWidget(this, "page widget");
    QGridLayout * grid = new QGridLayout(page, 3, 2, 0, 6);
    setMainWidget(page);

    QLabel * lblName = new QLabel(i18n("Layer name:"), page, "lblName");
    grid->addWidget(lblName, 0, 0);

    m_layerName = new KLineEdit(page, "m_layerName");
    m_layerName->setText(layerName);
    grid->addWidget(m_layerName, 0, 1);
    connect( m_layerName, SIGNAL( textChanged ( const QString & ) ), this, SLOT( slotNameChanged( const QString & ) ) );

    m_filtersList = new KisFiltersListView(dev, page, "dlgadjustment.filtersList");
    connect(m_filtersList , SIGNAL(selectionChanged(QIconViewItem*)), this, SLOT(selectionHasChanged(QIconViewItem* )));
    grid->addMultiCellWidget(m_filtersList, 1, 2, 0, 0);
    
    m_preview = new KisPreviewWidget(page, "dlgadjustment.preview");
    m_preview->slotSetDevice( dev );
    
    connect( m_preview, SIGNAL(updated()), this, SLOT(refreshPreview()));
    grid->addWidget(m_preview, 1, 1);
    
    m_configWidgetHolder = new QGroupBox(i18n("Configuration"), page, "currentConfigWidget");
    m_configWidgetHolder->setColumnLayout(0, Qt::Horizontal);
    grid->addWidget(m_configWidgetHolder, 2, 1);

    m_labelNoConfigWidget = new QLabel(i18n("No configuration options are available for this filter"),
                                        m_configWidgetHolder);
    m_configWidgetHolder->layout()->add(m_labelNoConfigWidget);
    m_labelNoConfigWidget->hide();

    resize( QSize(600, 480).expandedTo(minimumSizeHint()) );

    m_currentConfigWidget = 0;

    enableButtonOK( !m_layerName->text().isEmpty() );

    if (!create) {
        m_filtersList->setCurrentFilter(filter->id());
    }
    
}

void KisDlgAdjustmentLayer::slotNameChanged( const QString & text )
{
    enableButtonOK( !text.isEmpty() );
}

KisFilterConfiguration * KisDlgAdjustmentLayer::filterConfiguration() const
{
    return m_currentFilter->configuration(m_currentConfigWidget);
}

QString KisDlgAdjustmentLayer::layerName() const
{
    return m_layerName -> text();
}

void KisDlgAdjustmentLayer::slotConfigChanged()
{
    if(m_preview->getAutoUpdate())
    {
        refreshPreview();
    } else {
        m_preview->needUpdate();
    }
}

void KisDlgAdjustmentLayer::refreshPreview()
{
    KisPaintDeviceSP layer =  m_preview->getDevice();

    KisTransaction cmd("Temporary transaction", layer.data());
    KisFilterConfiguration* config = m_currentFilter->configuration(m_currentConfigWidget);

    QRect rect = layer -> extent();
    m_currentFilter->process(layer.data(), layer.data(), config, rect);
    m_preview->slotUpdate();
    cmd.unexecute();
}

void KisDlgAdjustmentLayer::selectionHasChanged ( QIconViewItem * item )
{
    KisFiltersIconViewItem* kisitem = (KisFiltersIconViewItem*) item;

    m_currentFilter = kisitem->filter();

    if ( m_currentConfigWidget != 0 )
    {
        m_configWidgetHolder->layout()->remove(m_currentConfigWidget);
        
        delete m_currentConfigWidget;
        m_currentConfigWidget = 0;
        
    } else {
        
        m_labelNoConfigWidget->hide();
    }
    
    KisPaintLayerSP activeLayer = (KisPaintLayer*) m_image->activeLayer().data();
    
    if (activeLayer) {
        m_currentConfigWidget = m_currentFilter->createConfigurationWidget(m_configWidgetHolder,
                                                                           activeLayer->paintDevice());
    }
    
    if (m_currentConfigWidget != 0)
    {
        m_configWidgetHolder->layout()->add(m_currentConfigWidget);
        m_currentConfigWidget->show();
        connect(m_currentConfigWidget, SIGNAL(sigPleaseUpdatePreview()), this, SLOT(slotConfigChanged()));
    } else {
        m_labelNoConfigWidget->show();
    }
    refreshPreview();
}

#include "kis_dlg_adjustment_layer.moc"
