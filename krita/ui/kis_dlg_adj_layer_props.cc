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
#include <Q3VBoxLayout>
#include <Q3HBoxLayout>

#include <klineedit.h>
#include <klocale.h>

#include "kis_filter_config_widget.h"
#include "kis_transaction.h"
#include "kis_filter.h"
#include "kis_filter_configuration.h"
#include "kis_filters_listview.h"
#include "kis_image.h"
#include "kis_previewwidget.h"
#include "kis_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_paint_device.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include "kis_dlg_adj_layer_props.h"
#include "kis_filter.h"
#include "kis_filter_configuration.h"

KisDlgAdjLayerProps::KisDlgAdjLayerProps(KisAdjustmentLayerSP layer,
                                         const QString & layerName,
                                         const QString & caption,
                                         QWidget *parent,
                                         const char *name)
    : KDialogBase(parent, name, true, "", Ok | Cancel)
{
    Q_ASSERT( layer );
    m_layer = layer;

    KisLayerSP next = layer->nextSibling();
    Q_ASSERT( next );

    m_currentConfiguration = layer->filter();
    m_currentFilter = KisFilterRegistry::instance()->get(m_currentConfiguration->name());
    if (!m_currentFilter) {
        kWarning() << "No filter specified!\n";
    }

    KisPaintDeviceSP dev = 0;

    KisPaintLayer * pl = dynamic_cast<KisPaintLayer*>(next.data());
    if (pl) {
        dev = pl->paintDevice();
    }
    else {
        KisGroupLayer * gl = dynamic_cast<KisGroupLayer*>(next.data());
        if (gl) {
            dev = gl->projection(gl->extent());
        }
        else {
            KisAdjustmentLayer * al = dynamic_cast<KisAdjustmentLayer*>(next.data());
            if (al) {
                dev = al->cachedPaintDevice();
            }
        }
    }

    setCaption(caption);
    QWidget * page = new QWidget(this, "page widget");
    Q3HBoxLayout * layout = new Q3HBoxLayout(page, 0, 6);
    setMainWidget(page);

    m_preview = new KisPreviewWidget(page, "dlgadjustment.preview");
    m_preview->slotSetDevice( dev );

    connect( m_preview, SIGNAL(updated()), this, SLOT(refreshPreview()));
    layout->addWidget(m_preview, 1, 1);

    Q3VBoxLayout *v1 = new Q3VBoxLayout( layout );
    Q3HBoxLayout *hl = new Q3HBoxLayout( v1 );

    QLabel * lblName = new QLabel(i18n("Layer name:"), page, "lblName");
    hl->addWidget(lblName, 0, 0);

    m_layerName = new KLineEdit(page, "m_layerName");
    m_layerName->setText(layerName);
    m_layerName->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    hl->addWidget(m_layerName, 0, 1);
    connect( m_layerName, SIGNAL( textChanged ( const QString & ) ), this, SLOT( slotNameChanged( const QString & ) ) );

    if ( m_currentFilter ) {
        m_currentConfigWidget = m_currentFilter->createConfigurationWidget(page, dev);
        if (m_currentConfigWidget) {
            m_currentConfigWidget->setConfiguration( m_currentConfiguration );
        }
    }
    if ( m_currentFilter == 0 || m_currentConfigWidget == 0 ) {
        QLabel * labelNoConfigWidget = new QLabel( i18n("No configuration options are available for this filter"), page );
        v1->addWidget( labelNoConfigWidget );
    }
    else {
        v1->addWidget( m_currentConfigWidget );
        connect(m_currentConfigWidget, SIGNAL(sigPleaseUpdatePreview()), this, SLOT(slotConfigChanged()));
    }

    refreshPreview();
    enableButtonOK( !m_layerName->text().isEmpty() );

}

void KisDlgAdjLayerProps::slotNameChanged( const QString & text )
{
    enableButtonOK( !text.isEmpty() );
}

KisFilterConfiguration * KisDlgAdjLayerProps::filterConfiguration() const
{
    return m_currentFilter->configuration(m_currentConfigWidget);
}

QString KisDlgAdjLayerProps::layerName() const
{
    return m_layerName->text();
}

void KisDlgAdjLayerProps::slotConfigChanged()
{
    if(m_preview->getAutoUpdate())
    {
        refreshPreview();
    } else {
        m_preview->needUpdate();
    }
}

void KisDlgAdjLayerProps::refreshPreview()
{
    if (!m_preview) {
        kDebug() << "no preview!\n";
        return;
    }

    KisPaintDeviceSP layer =  m_preview->getDevice();

    if (!layer) {
        return;
    }

    if (!m_currentFilter) {
        return;
    }
    KisFilterConfiguration* config = m_currentFilter->configuration(m_currentConfigWidget);

    QRect rect = layer->extent();
    KisTransaction cmd("Temporary transaction", layer.data());
    m_currentFilter->process(layer.data(), layer.data(), config, rect);
    m_preview->slotUpdate();
    cmd.unexecute();
}

#include "kis_dlg_adj_layer_props.moc"
