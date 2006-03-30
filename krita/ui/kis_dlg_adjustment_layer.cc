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

#include <q3groupbox.h>
#include <qlabel.h>
#include <qlayout.h>
//Added by qt3to4:
#include <Q3GridLayout>

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
                                             const QString & /*layerName*/,
                                             const QString & caption,
                                             QWidget *parent,
                                             const char *name)
    : KDialogBase(parent, name, true, "", Ok | Cancel)
    , m_image(img)
    , m_currentFilter(0)
    , m_customName(false)
    , m_freezeName(false)
{
    Q_ASSERT(img);

    KisLayerSP activeLayer = img->activeLayer();
    m_dev = 0;

    KisPaintLayer * pl = dynamic_cast<KisPaintLayer*>(activeLayer.data());
    if (pl) {
        m_dev = pl->paintDevice();
    }
    else {
        KisGroupLayer * gl = dynamic_cast<KisGroupLayer*>(activeLayer.data());
        if (gl) {
            m_dev = gl->projection(img->bounds());
        }
        else {
            KisAdjustmentLayer * al = dynamic_cast<KisAdjustmentLayer*>(activeLayer.data());
            if (al) {
                m_dev = al->cachedPaintDevice();
            }
        }
    }

    setCaption(caption);
    QWidget * page = new QWidget(this, "page widget");
    Q3GridLayout * grid = new Q3GridLayout(page, 3, 2, 0, 6);
    setMainWidget(page);

    QLabel * lblName = new QLabel(i18n("Layer name:"), page, "lblName");
    grid->addWidget(lblName, 0, 0);

    m_layerName = new KLineEdit(page, "m_layerName");
    grid->addWidget(m_layerName, 0, 1);
    connect( m_layerName, SIGNAL( textChanged ( const QString & ) ), this, SLOT( slotNameChanged( const QString & ) ) );

    m_filtersList = new KisFiltersListView(m_dev, page, "dlgadjustment.filtersList");
    connect(m_filtersList , SIGNAL(selectionChanged(Q3IconViewItem*)), this, SLOT(selectionHasChanged(Q3IconViewItem* )));
    grid->addMultiCellWidget(m_filtersList, 1, 2, 0, 0);

    m_preview = new KisPreviewWidget(page, "dlgadjustment.preview");
    m_preview->slotSetDevice( m_dev );

    connect( m_preview, SIGNAL(updated()), this, SLOT(refreshPreview()));
    grid->addWidget(m_preview, 1, 1);

    m_configWidgetHolder = new Q3GroupBox(i18n("Configuration"), page, "currentConfigWidget");
    m_configWidgetHolder->setColumnLayout(0, Qt::Horizontal);
    grid->addWidget(m_configWidgetHolder, 2, 1);

    m_labelNoConfigWidget = new QLabel(i18n("No configuration options are available for this filter"),
                                        m_configWidgetHolder);
    m_configWidgetHolder->layout()->add(m_labelNoConfigWidget);
    m_labelNoConfigWidget->hide();

    resize( QSize(600, 480).expandedTo(minimumSizeHint()) );

    m_currentConfigWidget = 0;

    enableButtonOK(0);
}

void KisDlgAdjustmentLayer::slotNameChanged( const QString & text )
{
    if (m_freezeName)
        return;

    m_customName = !text.isEmpty();
    enableButtonOK( m_currentFilter && m_customName );
}

KisFilterConfiguration * KisDlgAdjustmentLayer::filterConfiguration() const
{
    return m_currentFilter->configuration(m_currentConfigWidget);
}

QString KisDlgAdjustmentLayer::layerName() const
{
    return m_layerName->text();
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

    QRect rect = layer->extent();
    m_currentFilter->process(layer.data(), layer.data(), config, rect);
    m_preview->slotUpdate();
    cmd.unexecute();
}

void KisDlgAdjustmentLayer::selectionHasChanged ( Q3IconViewItem * item )
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

    if (m_dev) {
        m_currentConfigWidget = m_currentFilter->createConfigurationWidget(m_configWidgetHolder,
                                                                           m_dev);
    }

    if (m_currentConfigWidget != 0)
    {
        m_configWidgetHolder->layout()->add(m_currentConfigWidget);
        m_currentConfigWidget->show();
        connect(m_currentConfigWidget, SIGNAL(sigPleaseUpdatePreview()), this, SLOT(slotConfigChanged()));
    } else {
        m_labelNoConfigWidget->show();
    }

    if (!m_customName) {
        m_freezeName = true;
        m_layerName->setText(m_currentFilter->id().name());
        m_freezeName = false;
    }

    enableButtonOK( !m_layerName->text().isEmpty() );
    refreshPreview();
}

#include "kis_dlg_adjustment_layer.moc"
