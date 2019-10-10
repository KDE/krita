/*
 * This file is part of Krita
 *
 * Copyright (c) 2019 Carl Olsson <carl.olsson@gmail.com>
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

#include "KisDitherWidget.h"

#include <kpluginfactory.h>
#include <KoUpdater.h>
#include <KoResourceServerProvider.h>
#include <KoResourceServer.h>
#include <KoResourceServerAdapter.h>
#include <KoResourceItemChooser.h>
#include <KoColorSet.h>
#include <KoPattern.h>
#include <kis_properties_configuration.h>
#include "KisDitherUtil.h"

KisDitherWidget::KisDitherWidget(QWidget* parent)
    : QWidget(parent), Ui::KisDitherWidget()
{
    setupUi(this);

    QObject::connect(thresholdModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &KisDitherWidget::sigConfigurationItemChanged);

    patternIconWidget->setFixedSize(32, 32);
    KoResourceServer<KoPattern>* patternServer = KoResourceServerProvider::instance()->patternServer();
    QSharedPointer<KoAbstractResourceServerAdapter> patternAdapter(new KoResourceServerAdapter<KoPattern>(patternServer));
    m_ditherPatternWidget = new KoResourceItemChooser(patternAdapter, this, false);
    patternIconWidget->setPopupWidget(m_ditherPatternWidget);
    QObject::connect(m_ditherPatternWidget, &KoResourceItemChooser::resourceSelected, patternIconWidget, &KisIconWidget::setResource);
    QObject::connect(m_ditherPatternWidget, &KoResourceItemChooser::resourceSelected, this, &KisDitherWidget::sigConfigurationItemChanged);

    QObject::connect(patternValueModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &KisDitherWidget::sigConfigurationItemChanged);

    noiseSeedLineEdit->setValidator(new QIntValidator(this));
    QObject::connect(noiseSeedLineEdit, &QLineEdit::textChanged, this, &KisDitherWidget::sigConfigurationItemChanged);

    QObject::connect(noiseSeedRandomizeButton, &QToolButton::clicked, [this](){
        noiseSeedLineEdit->setText(QString::number(rand()));
    });

    spreadSpinBox->setPrefix(QString("%1  ").arg(i18n("Spread:")));
    spreadSpinBox->setRange(0.0, 1.0, 3);
    spreadSpinBox->setSingleStep(0.125);
    QObject::connect(spreadSpinBox, &KisDoubleSliderSpinBox::valueChanged, this, &KisDitherWidget::sigConfigurationItemChanged);
}

void KisDitherWidget::setConfiguration(const KisPropertiesConfiguration &config, const QString &prefix)
{
    thresholdModeComboBox->setCurrentIndex(config.getInt(prefix + "thresholdMode"));
    KoPattern* pattern = KoResourceServerProvider::instance()->patternServer()->resourceByName(config.getString(prefix + "pattern"));
    if (pattern) m_ditherPatternWidget->setCurrentResource(pattern);
    patternValueModeComboBox->setCurrentIndex(config.getInt(prefix + "patternValueMode"));
    noiseSeedLineEdit->setText(QString::number(config.getInt(prefix + "noiseSeed")));
    spreadSpinBox->setValue(config.getDouble(prefix + "spread"));
}

void KisDitherWidget::configuration(KisPropertiesConfiguration &config, const QString &prefix) const
{
    config.setProperty(prefix + "thresholdMode",thresholdModeComboBox->currentIndex());
    if (m_ditherPatternWidget->currentResource()) config.setProperty(prefix + "pattern", QVariant(m_ditherPatternWidget->currentResource()->name()));
    config.setProperty(prefix + "patternValueMode", patternValueModeComboBox->currentIndex());
    config.setProperty(prefix + "noiseSeed", noiseSeedLineEdit->text().toInt());
    config.setProperty(prefix + "spread", spreadSpinBox->value());
}

void KisDitherWidget::factoryConfiguration(KisPropertiesConfiguration &config, const QString &prefix)
{
    config.setProperty(prefix + "thresholdMode", KisDitherUtil::ThresholdMode::Pattern);
    config.setProperty(prefix + "pattern", "DITH 0202 GEN ");
    config.setProperty(prefix + "patternValueMode", KisDitherUtil::PatternValueMode::Auto);
    config.setProperty(prefix + "noiseSeed", rand());
    config.setProperty(prefix + "spread", 1.0);
}
