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
#include "kis_wdg_convert_height_to_normal_map.h"
#include <filter/kis_filter_configuration.h>
#include <QComboBox>
#include <klocalizedstring.h>
#include <KoChannelInfo.h>

KisWdgConvertHeightToNormalMap::KisWdgConvertHeightToNormalMap(QWidget *parent, const KoColorSpace *cs) :
    KisConfigWidget(parent),
    ui(new Ui_WidgetConvertHeightToNormalMap),
    m_cs(cs)

{
    ui->setupUi(this);
    m_types << "prewitt"<< "sobol"<< "simple";
    m_types_translatable << i18n("Prewitt") << i18n("Sobel") << i18n("Simple");
    QStringList swizzle;
    swizzle<< "X+" << "X-" << "Y+" << "Y-" << "Z+" << "Z-";

    ui->cmbType->addItems(m_types_translatable);
    ui->cmbRed->addItems(swizzle);
    ui->cmbGreen->addItems(swizzle);
    ui->cmbBlue->addItems(swizzle);

    for (int c = 0; c < (int)m_cs->channelCount(); c++) {
        ui->cmbChannel->addItem(m_cs->channels().at(c)->name());
    }

    ui->btnAspect->setKeepAspectRatio(false);
    ui->sldHorizontalRadius->setRange(1.0, 100.0, 2);
    ui->sldHorizontalRadius->setPrefix(i18n("Horizontal Radius:"));
    connect(ui->sldHorizontalRadius, SIGNAL(valueChanged(qreal)), this, SLOT(horizontalRadiusChanged(qreal)));

    ui->sldVerticalRadius->setRange(1.0, 100.0, 2);
    ui->sldVerticalRadius->setPrefix(i18n("Vertical Radius:"));
    connect(ui->sldVerticalRadius, SIGNAL(valueChanged(qreal)), this, SLOT(verticalRadiusChanged(qreal)));

    connect(ui->sldHorizontalRadius, SIGNAL(valueChanged(qreal)), this, SIGNAL(sigConfigurationItemChanged()));
    connect(ui->sldVerticalRadius, SIGNAL(valueChanged(qreal)), this, SIGNAL(sigConfigurationItemChanged()));
    connect(ui->btnAspect, SIGNAL(keepAspectRatioChanged(bool)), this, SLOT(aspectLockChanged(bool)));
    connect(ui->cmbType, SIGNAL(currentIndexChanged(int)), this, SIGNAL(sigConfigurationItemChanged()));
    connect(ui->cmbChannel, SIGNAL(currentIndexChanged(int)), this, SIGNAL(sigConfigurationItemChanged()));
    connect(ui->cmbRed, SIGNAL(currentIndexChanged(int)), this, SIGNAL(sigConfigurationItemChanged()));
    connect(ui->cmbGreen, SIGNAL(currentIndexChanged(int)), this, SIGNAL(sigConfigurationItemChanged()));
    connect(ui->cmbBlue, SIGNAL(currentIndexChanged(int)), this, SIGNAL(sigConfigurationItemChanged()));

}

KisWdgConvertHeightToNormalMap::~KisWdgConvertHeightToNormalMap()
{
    delete ui;
}

KisPropertiesConfigurationSP KisWdgConvertHeightToNormalMap::configuration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("height to normal", 1);
    config->setProperty("horizRadius", ui->sldHorizontalRadius->value());
    config->setProperty("vertRadius", ui->sldVerticalRadius->value());
    config->setProperty("type", m_types.at(ui->cmbType->currentIndex()));
    config->setProperty("lockAspect", ui->btnAspect->keepAspectRatio());
    config->setProperty("channelToConvert", ui->cmbChannel->currentIndex());
    config->setProperty("redSwizzle", ui->cmbRed->currentIndex());
    config->setProperty("greenSwizzle", ui->cmbGreen->currentIndex());
    config->setProperty("blueSwizzle", ui->cmbBlue->currentIndex());

    return config;
}

void KisWdgConvertHeightToNormalMap::setConfiguration(const KisPropertiesConfigurationSP config)
{
    ui->sldHorizontalRadius->setValue(config->getFloat("horizRadius", 1.0));
    ui->sldVerticalRadius->setValue(config->getFloat("vertRadius", 1.0));
    int index = 0;
    if (m_types.contains(config->getString("type", "prewitt"))){
        index = m_types.indexOf(config->getString("type", "sobol"));
    }
    ui->cmbType->setCurrentIndex(index);
    ui->cmbChannel->setCurrentIndex(config->getInt("channelToConvert", 0));
    ui->btnAspect->setKeepAspectRatio(config->getBool("lockAspect", false));
    ui->cmbRed->setCurrentIndex(config->getInt("redSwizzle", xPlus));
    ui->cmbGreen->setCurrentIndex(config->getInt("greenSwizzle", yPlus));
    ui->cmbBlue->setCurrentIndex(config->getInt("blueSwizzle", zPlus));
}

void KisWdgConvertHeightToNormalMap::horizontalRadiusChanged(qreal r)
{
    ui->sldHorizontalRadius->blockSignals(true);
    ui->sldHorizontalRadius->setValue(r);
    ui->sldHorizontalRadius->blockSignals(false);

    if (ui->btnAspect->keepAspectRatio()) {
        ui->sldVerticalRadius->blockSignals(true);
        ui->sldVerticalRadius->setValue(r);
        ui->sldVerticalRadius->blockSignals(false);
    }
}

void KisWdgConvertHeightToNormalMap::verticalRadiusChanged(qreal r)
{
    ui->sldVerticalRadius->blockSignals(true);
    ui->sldVerticalRadius->setValue(r);
    ui->sldVerticalRadius->blockSignals(false);

    if (ui->btnAspect->keepAspectRatio()) {
        ui->sldHorizontalRadius->blockSignals(true);
        ui->sldHorizontalRadius->setValue(r);
        ui->sldHorizontalRadius->blockSignals(false);
    }
}

void KisWdgConvertHeightToNormalMap::aspectLockChanged(bool v)
{
    if (v) {
        ui->sldVerticalRadius->setValue( ui->sldHorizontalRadius->value() );
    }
}
