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
#include <klocale.h>

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <klineedit.h>


#include "kis_config_widget.h"
#include "kis_transaction.h"
#include "filter/kis_filter.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_paint_device.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"


KisDlgAdjLayerProps::KisDlgAdjLayerProps(KisPaintDeviceSP paintDevice,
        const KisImageWSP image,
        KisFilterConfiguration * configuration,
        const QString & layerName,
        const QString & caption,
        QWidget *parent,
        const char *name)
        : KDialog(parent)
        , m_paintDevice(paintDevice)
        , m_image(0)
        , m_currentConfigWidget(0)
        , m_currentFilter(0)
        , m_currentConfiguration(0)
        , m_layer(0)
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
    layout->setSpacing(6);
    setMainWidget(page);

    QVBoxLayout *v1 = new QVBoxLayout();
    layout->addLayout(v1);
    QHBoxLayout *hl = new QHBoxLayout();
    v1->addLayout(hl);

    QLabel * lblName = new QLabel(i18n("Layer name:"), page);
    lblName->setObjectName("lblName");
    hl->addWidget(lblName, 0, 0);

    m_layerName = new KLineEdit(page);
    m_layerName->setObjectName("m_layerName");
    m_layerName->setText(layerName);
    m_layerName->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    hl->addWidget(m_layerName, 0, Qt::AlignLeft);
    connect(m_layerName, SIGNAL(textChanged(const QString &)), this, SLOT(slotNameChanged(const QString &)));

    if (m_currentFilter) {
        m_currentConfigWidget = m_currentFilter->createConfigurationWidget(page, paintDevice, image);
        if (m_currentConfigWidget) {
            m_currentConfigWidget->setConfiguration(m_currentConfiguration);
        }
    }
    if (m_currentFilter == 0 || m_currentConfigWidget == 0) {
        QLabel * labelNoConfigWidget = new QLabel(i18n("No configuration options are available for this filter"), page);
        v1->addWidget(labelNoConfigWidget);
    } else {
        v1->addWidget(m_currentConfigWidget);
    }

    enableButtonOk(!m_layerName->text().isEmpty());

}

void KisDlgAdjLayerProps::slotNameChanged(const QString & text)
{
    enableButtonOk(!text.isEmpty());
}

KisFilterConfiguration * KisDlgAdjLayerProps::filterConfiguration() const
{
    if (m_currentConfigWidget) {
        KisFilterConfiguration * config
        = dynamic_cast<KisFilterConfiguration*>(m_currentConfigWidget->configuration());
        if (config) {
            return config;
        }
    }
    return m_currentFilter->defaultConfiguration(m_paintDevice);
}

QString KisDlgAdjLayerProps::layerName() const
{
    return m_layerName->text();
}


#include "kis_dlg_adj_layer_props.moc"
