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

KisWdgEdgeDetection::KisWdgEdgeDetection(QWidget *parent) :
    KisConfigWidget(parent),
    ui(new Ui_WidgetEdgeDetection)
{
    ui->setupUi(this);

    connect(ui->cmbType, SIGNAL(currentIndexChanged(int)), this, SIGNAL(sigConfigurationItemChanged()));
    connect(ui->spnHorizontalRadius, SIGNAL(valueChanged(qreal)), this, SIGNAL(sigConfigurationItemChanged()));
    connect(ui->spnVerticalRadius, SIGNAL(valueChanged(qreal)), this, SIGNAL(sigConfigurationItemChanged()));
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
    if (ui->cmbType->currentIndex() == 0) {
        config->setProperty("type", "prewit");
    } else if (ui->cmbType->currentIndex() == 1) {
        config->setProperty("type", "sobolvector");
    } else if (ui->cmbType->currentIndex() == 2) {
        config->setProperty("type", "simple");
    }
    config->setProperty("lockAspect", true);

    return config;
}

void KisWdgEdgeDetection::setConfiguration(const KisPropertiesConfigurationSP config)
{
    ui->spnHorizontalRadius->setValue(config->getFloat("horizRadius", 1.0));
    ui->spnVerticalRadius->setValue(config->getFloat("vertRadius", 1.0));
}
