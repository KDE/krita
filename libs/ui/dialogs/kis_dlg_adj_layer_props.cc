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

#include "kis_dlg_adj_layer_props.h"
#include <klocalizedstring.h>

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <klineedit.h>


#include "kis_config_widget.h"
#include "kis_transaction.h"
#include "filter/kis_filter.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"
#include "kis_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_paint_device.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include "kis_node_filter_interface.h"

KisDlgAdjLayerProps::KisDlgAdjLayerProps(KisNodeSP node,
                                         KisNodeFilterInterface* nfi,
                                         KisPaintDeviceSP paintDevice,
                                         KisViewManager *view,
                                         KisFilterConfigurationSP configuration,
                                         const QString & layerName,
                                         const QString & caption,
                                         QWidget *parent,
                                         const char *name)
    : KoDialog(parent)
    , m_node(node)
    , m_paintDevice(paintDevice)
    , m_currentConfigWidget(0)
    , m_currentFilter(0)
    , m_currentConfiguration(0)
    , m_nodeFilterInterface(nfi)
{
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);

    setObjectName(name);

    m_currentConfiguration = configuration;
    if (m_currentConfiguration) {
        m_currentFilter = KisFilterRegistry::instance()->get(m_currentConfiguration->name()).data();
    }

    setCaption(caption);
    QWidget * page = new QWidget(this);
    page->setObjectName("page widget");
    QHBoxLayout * layout = new QHBoxLayout(page);
    layout->setMargin(0);
    setMainWidget(page);

    QVBoxLayout *v1 = new QVBoxLayout();
    layout->addLayout(v1);
    QHBoxLayout *hl = new QHBoxLayout();
    v1->addLayout(hl);

    QLabel * lblName = new QLabel(i18n("Layer name:"), page);
    lblName->setObjectName("lblName");
    hl->addWidget(lblName, 0);

    m_layerName = new KLineEdit(page);
    m_layerName->setObjectName("m_layerName");
    m_layerName->setText(layerName);
    m_layerName->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    hl->addWidget(m_layerName, 10);
    connect(m_layerName, SIGNAL(textChanged(QString)), this, SLOT(slotNameChanged(QString)));

    if (m_currentFilter) {
        m_currentConfigWidget = m_currentFilter->createConfigurationWidget(page, paintDevice);

        if (m_currentConfigWidget) {
            m_currentConfigWidget->setView(view);
            m_currentConfigWidget->setConfiguration(m_currentConfiguration);
        }
    }

    if (m_currentFilter == 0 || m_currentConfigWidget == 0) {
        QLabel * labelNoConfigWidget = new QLabel(i18n("No configuration options are available for this filter"), page);
        v1->addWidget(labelNoConfigWidget);
    }
    else {
        v1->addWidget(m_currentConfigWidget);
        connect(m_currentConfigWidget, SIGNAL(sigConfigurationUpdated()), SLOT(slotConfigChanged()));
    }

    enableButtonOk(!m_layerName->text().isEmpty());

}

void KisDlgAdjLayerProps::slotNameChanged(const QString & text)
{
    enableButtonOk(!text.isEmpty());
}

KisFilterConfigurationSP  KisDlgAdjLayerProps::filterConfiguration() const
{
    if (m_currentConfigWidget) {
        KisFilterConfigurationSP config = dynamic_cast<KisFilterConfiguration*>(m_currentConfigWidget->configuration().data());
        if (config) {
            return config;
        }
    }
    return m_currentFilter->defaultConfiguration();
}

QString KisDlgAdjLayerProps::layerName() const
{
    return m_layerName->text();
}

void KisDlgAdjLayerProps::slotConfigChanged()
{
    enableButtonOk(true);
    KisFilterConfigurationSP  config = filterConfiguration();
    if (config) {
        m_nodeFilterInterface->setFilter(config);
    }
    m_node->setDirty();
}


