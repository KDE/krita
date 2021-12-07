/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <generator/kis_generator.h>
#include <generator/kis_generator_registry.h>
#include <KoColor.h>
#include <KisGlobalResourcesInterface.h>
#include <filter/kis_filter_configuration.h>
#include <kis_config_widget.h>
#include <kis_signals_blocker.h>

#include <QStringList>
#include <QScrollBar>
#include <QResizeEvent>

#include "KisHalftoneConfigPageWidget.h"

KisHalftoneConfigPageWidget::KisHalftoneConfigPageWidget(QWidget *parent, const KisPaintDeviceSP dev)
    : QWidget(parent)
    , m_paintDevice(dev)
    , m_generatorWidget(nullptr)
    , m_view(nullptr)
{
    Q_ASSERT(m_paintDevice);

    ui()->setupUi(this);

    m_generatorIds = KisGeneratorRegistry::instance()->keys();
    m_generatorIds.sort();

    ui()->comboBoxGenerator->addItem(i18n("None"));
    for (const QString &generatorId : m_generatorIds) {
        KisGeneratorSP generator = KisGeneratorRegistry::instance()->get(generatorId);
        ui()->comboBoxGenerator->addItem(generator->name());
    }

    QVBoxLayout *generatorContainerLayout = new QVBoxLayout(ui()->widgetGeneratorContainer);
    generatorContainerLayout->setContentsMargins(0, 0, 0, 0);

    ui()->sliderHardness->setRange(0.0, 100.0, 2);
    ui()->sliderHardness->setSingleStep(1.0);
    ui()->sliderHardness->setSuffix(i18n("%"));

    ui()->sliderForegroundOpacity->setRange(0, 100);
    ui()->sliderForegroundOpacity->setPrefix(i18n("Opacity: "));
    ui()->sliderForegroundOpacity->setSuffix(i18n("%"));
    ui()->sliderBackgroundOpacity->setRange(0, 100);
    ui()->sliderBackgroundOpacity->setPrefix(i18n("Opacity: "));
    ui()->sliderBackgroundOpacity->setSuffix(i18n("%"));

    connect(ui()->comboBoxGenerator, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_comboBoxGenerator_currentIndexChanged(int)));
    connect(ui()->sliderHardness, SIGNAL(valueChanged(qreal)), this, SIGNAL(signal_configurationUpdated()));
    connect(ui()->checkBoxInvert, SIGNAL(toggled(bool)), this, SIGNAL(signal_configurationUpdated()));
    connect(ui()->buttonForegroundColor, SIGNAL(changed(const KoColor&)), this, SIGNAL(signal_configurationUpdated()));
    connect(ui()->sliderForegroundOpacity, SIGNAL(valueChanged(int)), this, SIGNAL(signal_configurationUpdated()));
    connect(ui()->buttonBackgroundColor, SIGNAL(changed(const KoColor&)), this, SIGNAL(signal_configurationUpdated()));
    connect(ui()->sliderBackgroundOpacity, SIGNAL(valueChanged(int)), this, SIGNAL(signal_configurationUpdated()));
}

KisHalftoneConfigPageWidget::~KisHalftoneConfigPageWidget()
{}

const Ui_HalftoneConfigPageWidget * KisHalftoneConfigPageWidget::ui() const
{
    return &m_ui;
}

Ui_HalftoneConfigPageWidget * KisHalftoneConfigPageWidget::ui()
{
    return &m_ui;
}

void KisHalftoneConfigPageWidget::showColors()
{
    setColorsVisible(true);
}

void KisHalftoneConfigPageWidget::hideColors()
{
    setColorsVisible(false);
}

void KisHalftoneConfigPageWidget::setColorsVisible(bool show)
{
    ui()->labelForeground->setVisible(show);
    ui()->buttonForegroundColor->setVisible(show);
    ui()->sliderForegroundOpacity->setVisible(show);
    ui()->labelBackground->setVisible(show);
    ui()->buttonBackgroundColor->setVisible(show);
    ui()->sliderBackgroundOpacity->setVisible(show);
}

void KisHalftoneConfigPageWidget::setConfiguration(const KisHalftoneFilterConfigurationSP config, const QString & prefix)
{
    {
        KisSignalsBlocker signalsBlocker(this, ui()->comboBoxGenerator);

        QString generatorId = config->generatorId(prefix);
        int generatorIndex = m_generatorIds.indexOf(generatorId);
        if (generatorIndex == -1) {
            ui()->comboBoxGenerator->setCurrentIndex(0);
            setGenerator("", nullptr);
        } else {
            ui()->comboBoxGenerator->setCurrentIndex(generatorIndex + 1);
            KisFilterConfigurationSP generatorConfiguration = config->generatorConfiguration(prefix);
            setGenerator(generatorId, generatorConfiguration);
        }

        ui()->sliderHardness->setValue(config->hardness(prefix));
        ui()->checkBoxInvert->setChecked(config->invert(prefix));
        ui()->buttonForegroundColor->setColor(config->foregroundColor(prefix));
        ui()->sliderForegroundOpacity->setValue(config->foregroundOpacity(prefix));
        ui()->buttonBackgroundColor->setColor(config->backgroundColor(prefix));
        ui()->sliderBackgroundOpacity->setValue(config->backgroundOpacity(prefix));
    }
    emit signal_configurationUpdated();
}

void KisHalftoneConfigPageWidget::configuration(KisHalftoneFilterConfigurationSP config, const QString & prefix) const
{
    if (ui()->comboBoxGenerator->currentIndex() == 0) {
        config->setGeneratorId(prefix, "");
    } else {
        QString generatorId = m_generatorIds.at(ui()->comboBoxGenerator->currentIndex() - 1);
        config->setGeneratorId(prefix, generatorId);
        if (m_generatorWidget) {
            config->setGeneratorConfiguration(
                prefix, dynamic_cast<KisFilterConfiguration*>(m_generatorWidget->configuration().data())
            );
        }
    }

    config->setHardness(prefix, ui()->sliderHardness->value());
    config->setInvert(prefix, ui()->checkBoxInvert->isChecked());
    config->setForegroundColor(prefix, ui()->buttonForegroundColor->color());
    config->setForegroundOpacity(prefix, ui()->sliderForegroundOpacity->value());
    config->setBackgroundColor(prefix, ui()->buttonBackgroundColor->color());
    config->setBackgroundOpacity(prefix, ui()->sliderBackgroundOpacity->value());
}

void KisHalftoneConfigPageWidget::setGenerator(const QString & generatorId, KisFilterConfigurationSP config)
{
    if (m_generatorWidget) {
        ui()->widgetGeneratorContainer->layout()->removeWidget(m_generatorWidget);
        delete m_generatorWidget;
        m_generatorWidget = nullptr;
    }

    KisGeneratorSP generator = KisGeneratorRegistry::instance()->get(generatorId);
    if (generator) {
        KisConfigWidget *generatorWidget = generator->createConfigurationWidget(this, m_paintDevice, false);
        if (generatorWidget) {
            ui()->widgetGeneratorContainer->layout()->addWidget(generatorWidget);

            if (m_view) {
                generatorWidget->setView(m_view);
            } else {
                generatorWidget->setCanvasResourcesInterface(m_canvasResourcesInterface);
            }

            if (config) {
                generatorWidget->setConfiguration(config);
            } else {
                KisFilterConfigurationSP generatorConfig =
                    generator->defaultConfiguration(KisGlobalResourcesInterface::instance());
                if (generatorId == "screentone") {
                    generatorConfig->setProperty("rotation", 45.0);
                    generatorConfig->setProperty("contrast", 50.0);
                }
                generatorWidget->setConfiguration(generatorConfig);
            }
            m_generatorWidget = generatorWidget;
            connect(generatorWidget, SIGNAL(sigConfigurationUpdated()), this, SIGNAL(signal_configurationUpdated()));
        }
    }
}

void KisHalftoneConfigPageWidget::setView(KisViewManager *view)
{
    m_view = view;

    if (m_generatorWidget) {
        m_generatorWidget->setView(m_view);
    }
}

void KisHalftoneConfigPageWidget::setCanvasResourcesInterface(KoCanvasResourcesInterfaceSP canvasResourcesInterface)
{
    m_canvasResourcesInterface = canvasResourcesInterface;

    if (m_generatorWidget) {
        m_generatorWidget->setCanvasResourcesInterface(canvasResourcesInterface);
    }
}

void KisHalftoneConfigPageWidget::slot_comboBoxGenerator_currentIndexChanged(int index)
{
    if (index < 0 || index > m_generatorIds.size()) {
        return;
    }

    if (index == 0) {
        setGenerator("", nullptr);
    } else {
        QString generatorId = m_generatorIds.at(index - 1);
        setGenerator(generatorId, nullptr);
    }

    emit signal_configurationUpdated();
}
