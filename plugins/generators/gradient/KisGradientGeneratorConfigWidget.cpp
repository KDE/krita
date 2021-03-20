/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 * SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QStringList>

#include <filter/kis_filter_configuration.h>
#include <KisGlobalResourcesInterface.h>
#include <kis_signals_blocker.h>
#include <kis_gradient_painter.h>
#include <KisViewManager.h>
#include <kis_canvas_resource_provider.h>

#include "KisGradientGeneratorConfiguration.h"
#include "KisGradientGeneratorConfigWidget.h"

KisGradientGeneratorConfigWidget::KisGradientGeneratorConfigWidget(QWidget* parent)
    : KisConfigWidget(parent)
    , m_view(nullptr)
{
    QStringList shapeNames =
        QStringList()
        << i18nc("the gradient will be drawn linearly", "Linear")
        << i18nc("the gradient will be drawn bilinearly", "Bi-Linear")
        << i18nc("the gradient will be drawn radially", "Radial")
        << i18nc("the gradient will be drawn in a square around a centre", "Square")
        << i18nc("the gradient will be drawn as an asymmetric cone", "Conical")
        << i18nc("the gradient will be drawn as a symmetric cone", "Conical Symmetric")
        << i18nc("the gradient will be drawn as a spiral", "Spiral")
        << i18nc("the gradient will be drawn as a reverse spiral", "Reverse Spiral")
        << i18nc("the gradient will be drawn in a selection outline", "Shaped");
    QStringList repeatNames =
        QStringList()
        << i18nc("The gradient will not repeat", "None")
        << i18nc("The gradient will repeat forwards", "Forwards")
        << i18nc("The gradient will repeat alternatingly", "Alternating");
    QStringList spatialUnitsNames =
        QStringList()
        << i18nc("The position will be set in pixels", "Pixels")
        << i18nc("The position will be a percentage of the width", "Percent of the Width")
        << i18nc("The position will be a percentage of the height", "Percent of the Height")
        << i18nc("The position will be a percentage of the longest image side", "Percent of the Longest Side")
        << i18nc("The position will be a percentage of the shortest image side", "Percent of the Shortest Side");
    QStringList positioningNames =
        QStringList()
        << i18nc("The position will be relative to the top-left corner of the image", "Absolute")
        << i18nc("The position will be relative to the start point", "Relative");

    m_ui.setupUi(this);

    m_ui.comboBoxShape->addItems(shapeNames);
    m_ui.comboBoxRepeat->addItems(repeatNames);
    m_ui.sliderAntiAliasThreshold->setRange(0, 1, 3);

    m_ui.comboBoxStartPositionXUnits->addItems(spatialUnitsNames);
    m_ui.comboBoxStartPositionYUnits->addItems(spatialUnitsNames);
    m_ui.comboBoxEndPositionXUnits->addItems(spatialUnitsNames);
    m_ui.comboBoxEndPositionYUnits->addItems(spatialUnitsNames);
    m_ui.comboBoxEndPositionXPositioning->addItems(positioningNames);
    m_ui.comboBoxEndPositionYPositioning->addItems(positioningNames);
    m_ui.comboBoxEndPositionDistanceUnits->addItems(spatialUnitsNames);

    m_ui.widgetGradientEditor->setContentsMargins(10, 10, 10, 10);
    m_ui.widgetGradientEditor->loadUISettings();

    connect(m_ui.comboBoxShape, SIGNAL(currentIndexChanged(int)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.comboBoxRepeat, SIGNAL(currentIndexChanged(int)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.sliderAntiAliasThreshold, SIGNAL(valueChanged(qreal)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.checkBoxDither, SIGNAL(toggled(bool)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.checkBoxReverse, SIGNAL(toggled(bool)), this, SIGNAL(sigConfigurationUpdated()));

    connect(m_ui.spinBoxStartPositionX, SIGNAL(valueChanged(double)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.spinBoxStartPositionY, SIGNAL(valueChanged(double)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.comboBoxStartPositionXUnits, SIGNAL(currentIndexChanged(int)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.comboBoxStartPositionYUnits, SIGNAL(currentIndexChanged(int)), this, SIGNAL(sigConfigurationUpdated()));

    connect(
        m_ui.radioButtonEndPositionCartesianCoordinates,
        SIGNAL(toggled(bool)),
        this,
        SLOT(slot_radioButtonEndPositionCartesianCoordinates_toggled(bool))
    );
    connect(
        m_ui.radioButtonEndPositionPolarCoordinates,
        SIGNAL(toggled(bool)),
        this,
        SLOT(slot_radioButtonEndPositionPolarCoordinates_toggled(bool))
    );
    connect(m_ui.spinBoxEndPositionX, SIGNAL(valueChanged(double)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.spinBoxEndPositionY, SIGNAL(valueChanged(double)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.comboBoxEndPositionXUnits, SIGNAL(currentIndexChanged(int)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.comboBoxEndPositionYUnits, SIGNAL(currentIndexChanged(int)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.comboBoxEndPositionXPositioning, SIGNAL(currentIndexChanged(int)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.comboBoxEndPositionYPositioning, SIGNAL(currentIndexChanged(int)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.angleSelectorEndPositionAngle, SIGNAL(angleChanged(qreal)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.spinBoxEndPositionDistance, SIGNAL(valueChanged(double)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.comboBoxEndPositionDistanceUnits, SIGNAL(currentIndexChanged(int)), this, SIGNAL(sigConfigurationUpdated()));

    connect(m_ui.widgetGradientEditor, SIGNAL(sigGradientChanged()), this, SIGNAL(sigConfigurationUpdated()));
}

KisGradientGeneratorConfigWidget::~KisGradientGeneratorConfigWidget()
{
    m_ui.widgetGradientEditor->saveUISettings();
}

void KisGradientGeneratorConfigWidget::setConfiguration(const KisPropertiesConfigurationSP config)
{
    const KisGradientGeneratorConfiguration *generatorConfig =
        dynamic_cast<const KisGradientGeneratorConfiguration*>(config.data());

    {
        KisSignalsBlocker signalsBlocker(this);

        m_ui.comboBoxShape->setCurrentIndex(generatorConfig->shape());
        m_ui.comboBoxRepeat->setCurrentIndex(generatorConfig->repeat());
        m_ui.sliderAntiAliasThreshold->setValue(generatorConfig->antiAliasThreshold());
        m_ui.checkBoxDither->setChecked(generatorConfig->dither());
        m_ui.checkBoxReverse->setChecked(generatorConfig->reverse());

        m_ui.spinBoxStartPositionX->setValue(generatorConfig->startPositionX());
        m_ui.spinBoxStartPositionY->setValue(generatorConfig->startPositionY());
        m_ui.comboBoxStartPositionXUnits->setCurrentIndex(generatorConfig->startPositionXUnits());
        m_ui.comboBoxStartPositionYUnits->setCurrentIndex(generatorConfig->startPositionYUnits());
        if (generatorConfig->endPositionCoordinateSystem() == KisGradientGeneratorConfiguration::CoordinateSystemCartesian) {
            m_ui.radioButtonEndPositionCartesianCoordinates->setChecked(true);
        } else {
            m_ui.radioButtonEndPositionPolarCoordinates->setChecked(true);
        }
        m_ui.spinBoxEndPositionX->setValue(generatorConfig->endPositionX());
        m_ui.spinBoxEndPositionY->setValue(generatorConfig->endPositionY());
        m_ui.comboBoxEndPositionXUnits->setCurrentIndex(generatorConfig->endPositionXUnits());
        m_ui.comboBoxEndPositionYUnits->setCurrentIndex(generatorConfig->endPositionYUnits());
        m_ui.comboBoxEndPositionXPositioning->setCurrentIndex(generatorConfig->endPositionXPositioning());
        m_ui.comboBoxEndPositionYPositioning->setCurrentIndex(generatorConfig->endPositionYPositioning());
        m_ui.angleSelectorEndPositionAngle->setAngle(generatorConfig->endPositionAngle());
        m_ui.spinBoxEndPositionDistance->setValue(generatorConfig->endPositionDistance());
        m_ui.comboBoxEndPositionDistanceUnits->setCurrentIndex(generatorConfig->endPositionDistanceUnits());

        m_ui.widgetGradientEditor->setGradient(generatorConfig->gradient());
    }

    emit sigConfigurationUpdated();
}

KisPropertiesConfigurationSP KisGradientGeneratorConfigWidget::configuration() const
{
    KisGradientGeneratorConfiguration *config = new KisGradientGeneratorConfiguration(KisGlobalResourcesInterface::instance());

    config->setShape(static_cast<KisGradientPainter::enumGradientShape>(m_ui.comboBoxShape->currentIndex()));
    config->setRepeat(static_cast<KisGradientPainter::enumGradientRepeat>(m_ui.comboBoxRepeat->currentIndex()));
    config->setAntiAliasThreshold(m_ui.sliderAntiAliasThreshold->value());
    config->setDither(m_ui.checkBoxDither->isChecked());
    config->setReverse(m_ui.checkBoxReverse->isChecked());

    config->setStartPositionX(m_ui.spinBoxStartPositionX->value());
    config->setStartPositionY(m_ui.spinBoxStartPositionY->value());
    config->setStartPositionXUnits(static_cast<KisGradientGeneratorConfiguration::SpatialUnits>(m_ui.comboBoxStartPositionXUnits->currentIndex()));
    config->setStartPositionYUnits(static_cast<KisGradientGeneratorConfiguration::SpatialUnits>(m_ui.comboBoxStartPositionYUnits->currentIndex()));
    if (m_ui.radioButtonEndPositionCartesianCoordinates->isChecked()) {
        config->setEndPositionCoordinateSystem(KisGradientGeneratorConfiguration::CoordinateSystemCartesian);
    } else {
        config->setEndPositionCoordinateSystem(KisGradientGeneratorConfiguration::CoordinateSystemPolar);
    }
    config->setEndPositionX(m_ui.spinBoxEndPositionX->value());
    config->setEndPositionY(m_ui.spinBoxEndPositionY->value());
    config->setEndPositionXUnits(static_cast<KisGradientGeneratorConfiguration::SpatialUnits>(m_ui.comboBoxEndPositionXUnits->currentIndex()));
    config->setEndPositionYUnits(static_cast<KisGradientGeneratorConfiguration::SpatialUnits>(m_ui.comboBoxEndPositionYUnits->currentIndex()));
    config->setEndPositionXPositioning(static_cast<KisGradientGeneratorConfiguration::Positioning>(m_ui.comboBoxEndPositionXPositioning->currentIndex()));
    config->setEndPositionYPositioning(static_cast<KisGradientGeneratorConfiguration::Positioning>(m_ui.comboBoxEndPositionYPositioning->currentIndex()));
    config->setEndPositionAngle(m_ui.angleSelectorEndPositionAngle->angle());
    config->setEndPositionDistance(m_ui.spinBoxEndPositionDistance->value());
    config->setEndPositionDistanceUnits(static_cast<KisGradientGeneratorConfiguration::SpatialUnits>(m_ui.comboBoxEndPositionDistanceUnits->currentIndex()));

    KoAbstractGradientSP gradient = m_ui.widgetGradientEditor->gradient();
    if (gradient && m_view) {
        KoCanvasResourcesInterfaceSP canvasResourcesInterface =
            m_view->canvasResourceProvider()->resourceManager()->canvasResourcesInterface();
        gradient->bakeVariableColors(canvasResourcesInterface);
    }
    config->setGradient(gradient);

    return config;
}

void KisGradientGeneratorConfigWidget::setView(KisViewManager *view)
{
    m_view = view;
    if (view) {
        KoCanvasResourcesInterfaceSP canvasResourcesInterface = m_view->canvasResourceProvider()->resourceManager()->canvasResourcesInterface();
        m_ui.widgetGradientEditor->setCanvasResourcesInterface(canvasResourcesInterface);
    } else {
        m_ui.widgetGradientEditor->setCanvasResourcesInterface(nullptr);
    }
}

void KisGradientGeneratorConfigWidget::slot_radioButtonEndPositionCartesianCoordinates_toggled(bool enabled)
{
    if (!enabled) {
        return;
    }

    m_ui.stackedWidgetEndPosition->setCurrentIndex(0);
    emit sigConfigurationUpdated();
}

void KisGradientGeneratorConfigWidget::slot_radioButtonEndPositionPolarCoordinates_toggled(bool enabled)
{
    if (!enabled) {
        return;
    }

    m_ui.stackedWidgetEndPosition->setCurrentIndex(1);
    emit sigConfigurationUpdated();
}
