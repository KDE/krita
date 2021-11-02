/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2019 Carl Olsson <carl.olsson@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisDitherWidget.h"

#include <kpluginfactory.h>
#include <KoUpdater.h>
#include "kis_filter_configuration.h"
#include "KisResourcesInterface.h"
#include <KisResourceItemChooser.h>
#include <KoColorSet.h>
#include <KoPattern.h>
#include <kis_properties_configuration.h>
#include "KisDitherUtil.h"
#include <KoResourceLoadResult.h>

KisDitherWidget::KisDitherWidget(QWidget* parent)
    : QWidget(parent), Ui::KisDitherWidget()
{
    setupUi(this);

    QObject::connect(thresholdModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &KisDitherWidget::sigConfigurationItemChanged);

    patternIconWidget->setFixedSize(64, 64);
    patternIconWidget->setBackgroundColor(Qt::white);
    m_ditherPatternWidget = new KisResourceItemChooser(ResourceType::Patterns, false, this);
    patternIconWidget->setPopupWidget(m_ditherPatternWidget);
    QObject::connect(m_ditherPatternWidget, &KisResourceItemChooser::resourceSelected, patternIconWidget, &KisIconWidget::setResource);
    QObject::connect(m_ditherPatternWidget, &KisResourceItemChooser::resourceSelected, this, &KisDitherWidget::sigConfigurationItemChanged);

    QObject::connect(patternValueModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &KisDitherWidget::sigConfigurationItemChanged);

    noiseSeedLineEdit->setValidator(new QIntValidator(this));
    QObject::connect(noiseSeedLineEdit, &QLineEdit::textChanged, this, &KisDitherWidget::sigConfigurationItemChanged);

    QObject::connect(noiseSeedRandomizeButton, &QToolButton::clicked, [this](){
        noiseSeedLineEdit->setText(QString::number(rand()));
    });

    spreadSpinBox->setPrefix(QString("%1  ").arg(i18n("Spread:")));
    spreadSpinBox->setRange(0.0, 1.0, 3);
    spreadSpinBox->setSingleStep(0.125);
    QObject::connect(spreadSpinBox, QOverload<double>::of(&KisDoubleSliderSpinBox::valueChanged), this, &KisDitherWidget::sigConfigurationItemChanged);
}

void KisDitherWidget::setConfiguration(const KisFilterConfiguration &config, const QString &prefix)
{
    thresholdModeComboBox->setCurrentIndex(config.getInt(prefix + "thresholdMode"));

    auto source = config.resourcesInterface()->source<KoPattern>(ResourceType::Patterns);
    QString patternMD5 = config.getString(prefix + "md5sum");
    QString patternName = config.getString(prefix + "pattern");
    KoPatternSP pattern = source.bestMatch(patternMD5, "", patternName);

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

QList<KoResourceLoadResult> KisDitherWidget::prepareLinkedResources(const KisFilterConfiguration &config, const QString &prefix, KisResourcesInterfaceSP resourcesInterface)
{
    auto source = resourcesInterface->source<KoPattern>(ResourceType::Patterns);

    QString patternMD5 = config.getString(prefix + "md5sum");
    QString patternName = config.getString(prefix + "pattern");

    return {source.bestMatchLoadResult(patternMD5, "", patternName)};
}
