/*
 * Copyright (c) 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
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
#include "kis_wdg_edge_detection.h"

#include <filter/kis_filter_configuration.h>
#include <QComboBox>
#include <klocalizedstring.h>

KisWdgEdgeDetection::KisWdgEdgeDetection(QWidget *parent) :
    KisConfigWidget(parent),
    ui(new Ui_WidgetEdgeDetection)
{
    ui->setupUi(this);

    m_types << "prewitt"<< "sobol"<< "simple";
    m_types_translatable << i18n("Prewitt") << i18n("Sobol") << i18n("Simple");

    ui->cmbType->addItems(m_types_translatable);

    connect(ui->cmbType, SIGNAL(currentIndexChanged(int)), this, SIGNAL(sigConfigurationItemChanged()));
    connect(ui->spnHorizontalRadius, SIGNAL(valueChanged(qreal)), this, SIGNAL(sigConfigurationItemChanged()));
    connect(ui->spnVerticalRadius, SIGNAL(valueChanged(qreal)), this, SIGNAL(sigConfigurationItemChanged()));
    connect(ui->chkTransparent, SIGNAL(clicked()), this, SIGNAL(sigConfigurationItemChanged()));
}

KisWdgEdgeDetection::~KisWdgEdgeDetection()
{
    delete ui;
}

KisPropertiesConfigurationSP KisWdgEdgeDetection::configuration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("edge detection", 1);
    config->setProperty("horizRadius", ui->spnHorizontalRadius->value());
    config->setProperty("vertRadius", ui->spnVerticalRadius->value());
    config->setProperty("type", m_types.at(ui->cmbType->currentIndex()));
    config->setProperty("lockAspect", true);
    config->setProperty("transparency", ui->chkTransparent->isChecked());

    return config;
}

void KisWdgEdgeDetection::setConfiguration(const KisPropertiesConfigurationSP config)
{
    ui->spnHorizontalRadius->setValue(config->getFloat("horizRadius", 1.0));
    ui->spnVerticalRadius->setValue(config->getFloat("vertRadius", 1.0));
    int index = 0;
    if (m_types.contains(config->getString("type", "prewitt"))){
        index = m_types.indexOf(config->getString("type", "prewitt"));
    }
    ui->cmbType->setCurrentIndex(index);
    ui->chkTransparent->setChecked(config->getBool("transparency", false));

}
