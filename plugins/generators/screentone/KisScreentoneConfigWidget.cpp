/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <KoColor.h>
#include <filter/kis_filter_configuration.h>
#include <KisGlobalResourcesInterface.h>

#include "KisScreentoneConfigWidget.h"
#include "KisScreentoneScreentoneFunctions.h"
#include "KisScreentoneConfigDefaults.h"

KisScreentoneConfigWidget::KisScreentoneConfigWidget(QWidget* parent, const KoColorSpace *cs)
        : KisConfigWidget(parent)
        , m_colorSpace(cs)
{
    m_ui.setupUi(this);

    setupPatternComboBox();
    setupShapeComboBox();
    setupInterpolationComboBox();

    m_ui.sliderForegroundOpacity->setRange(0, 100);
    m_ui.sliderForegroundOpacity->setPrefix(i18n("Opacity: "));
    m_ui.sliderForegroundOpacity->setSuffix(i18n("%"));
    m_ui.sliderBackgroundOpacity->setRange(0, 100);
    m_ui.sliderBackgroundOpacity->setPrefix(i18n("Opacity: "));
    m_ui.sliderBackgroundOpacity->setSuffix(i18n("%"));
    m_ui.sliderBrightness->setRange(0.0, 100.0, 2);
    m_ui.sliderBrightness->setSingleStep(1.0);
    m_ui.sliderBrightness->setSuffix(i18n("%"));
    m_ui.sliderContrast->setRange(0.0, 100.0, 2);
    m_ui.sliderContrast->setSingleStep(1.0);
    m_ui.sliderContrast->setSuffix(i18n("%"));

    m_ui.sliderPositionX->setRange(-1000.0, 1000.0, 2);
    m_ui.sliderPositionX->setSoftRange(-100.0, 100.0);
    m_ui.sliderPositionX->setPrefix(i18n("X: "));
    m_ui.sliderPositionX->setSuffix(i18n(" px"));
    m_ui.sliderPositionX->setSingleStep(1.0);
    m_ui.sliderPositionY->setRange(-1000.0, 1000.0, 2);
    m_ui.sliderPositionY->setSoftRange(-100.0, 100.0);
    m_ui.sliderPositionY->setPrefix(i18n("Y: "));
    m_ui.sliderPositionY->setSuffix(i18n(" px"));
    m_ui.sliderPositionY->setSingleStep(1.0);
    m_ui.sliderSizeX->setRange(1.0, 1000.0, 2);
    m_ui.sliderSizeX->setSoftRange(1.0, 100.0);
    m_ui.sliderSizeX->setPrefix(i18n("X: "));
    m_ui.sliderSizeX->setSuffix(i18n(" px"));
    m_ui.sliderSizeX->setSingleStep(1.0);
    m_ui.sliderSizeX->setExponentRatio(4.32);
    m_ui.sliderSizeY->setRange(1.0, 1000.0, 2);
    m_ui.sliderSizeY->setSoftRange(1.0, 100.0);
    m_ui.sliderSizeY->setPrefix(i18n("Y: "));
    m_ui.sliderSizeY->setSuffix(i18n(" px"));
    m_ui.sliderSizeY->setSingleStep(1.0);
    m_ui.sliderSizeY->setExponentRatio(4.32);
    m_ui.sliderShearX->setRange(-10.0, 10.0, 2);
    m_ui.sliderShearX->setSoftRange(-2.0, 2.0);
    m_ui.sliderShearX->setPrefix(i18n("X: "));
    m_ui.sliderShearX->setSingleStep(0.1);
    m_ui.sliderShearY->setRange(-10.0, 10.0, 2);
    m_ui.sliderShearY->setSoftRange(-2.0, 2.0);
    m_ui.sliderShearY->setPrefix(i18n("Y: "));
    m_ui.sliderShearY->setSingleStep(0.1);

    connect(m_ui.comboBoxPattern, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_comboBoxPattern_currentIndexChanged(int)));
    connect(m_ui.comboBoxShape, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_comboBoxShape_currentIndexChanged(int)));
    connect(m_ui.comboBoxInterpolation, SIGNAL(currentIndexChanged(int)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.buttonForegroundColor, SIGNAL(changed(const KoColor&)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.sliderForegroundOpacity, SIGNAL(valueChanged(int)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.buttonBackgroundColor, SIGNAL(changed(const KoColor&)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.sliderBackgroundOpacity, SIGNAL(valueChanged(int)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.checkBoxInvert, SIGNAL(toggled(bool)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.sliderBrightness, SIGNAL(valueChanged(qreal)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.sliderContrast, SIGNAL(valueChanged(qreal)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.sliderPositionX, SIGNAL(valueChanged(qreal)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.sliderPositionY, SIGNAL(valueChanged(qreal)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.sliderSizeX, SIGNAL(valueChanged(qreal)), this, SLOT(slot_sliderSizeX_valueChanged(qreal)));
    connect(m_ui.sliderSizeY, SIGNAL(valueChanged(qreal)), this, SLOT(slot_sliderSizeY_valueChanged(qreal)));
    connect(m_ui.buttonKeepSizeSquare, SIGNAL(keepAspectRatioChanged(bool)), this, SLOT(slot_buttonKeepSizeSquare_keepAspectRatioChanged(bool)));
    connect(m_ui.sliderShearX, SIGNAL(valueChanged(qreal)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.sliderShearY, SIGNAL(valueChanged(qreal)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_ui.angleSelectorRotation, SIGNAL(angleChanged(qreal)), this, SIGNAL(sigConfigurationUpdated()));
}

KisScreentoneConfigWidget::~KisScreentoneConfigWidget()
{}

void KisScreentoneConfigWidget::setConfiguration(const KisPropertiesConfigurationSP config)
{
    // The double slider spin boxes and the color buttons emit signals
    // when their value is set via code so we block signals here to 
    // prevent multiple sigConfigurationUpdated being called.
    // After the widgets are set up, unblock and emit sigConfigurationUpdated
    // just once 
    blockSignals(true);

    KoColor c;
    m_ui.comboBoxPattern->setCurrentIndex(config->getInt("pattern", KisScreentoneConfigDefaults::pattern()));
    m_ui.comboBoxShape->setCurrentIndex(config->getInt("shape", KisScreentoneConfigDefaults::shape()));
    m_ui.comboBoxInterpolation->setCurrentIndex(config->getInt("interpolation", KisScreentoneConfigDefaults::interpolation()));
    c = config->getColor("foreground_color", KisScreentoneConfigDefaults::foregroundColor());
    c.convertTo(m_colorSpace);
    c.setOpacity(1.0);
    m_ui.buttonForegroundColor->setColor(c);
    m_ui.sliderForegroundOpacity->setValue(config->getInt("foreground_opacity", KisScreentoneConfigDefaults::foregroundOpacity()));
    c = config->getColor("background_color", KisScreentoneConfigDefaults::backgroundColor());
    c.convertTo(m_colorSpace);
    c.setOpacity(1.0);
    m_ui.buttonBackgroundColor->setColor(c);
    m_ui.sliderBackgroundOpacity->setValue(config->getInt("background_opacity", KisScreentoneConfigDefaults::backgroundOpacity()));
    m_ui.checkBoxInvert->setChecked(config->getBool("invert", KisScreentoneConfigDefaults::invert()));
    m_ui.sliderBrightness->setValue(config->getDouble("brightness", KisScreentoneConfigDefaults::brightness()));
    m_ui.sliderContrast->setValue(config->getDouble("contrast", KisScreentoneConfigDefaults::contrast()));
    m_ui.sliderPositionX->setValue(config->getDouble("position_x", KisScreentoneConfigDefaults::positionX()));
    m_ui.sliderPositionY->setValue(config->getDouble("position_y", KisScreentoneConfigDefaults::positionY()));
    m_ui.buttonKeepSizeSquare->setKeepAspectRatio(config->getBool("keep_size_square", KisScreentoneConfigDefaults::keepSizeSquare()));
    m_ui.sliderSizeX->setValue(config->getDouble("size_x", KisScreentoneConfigDefaults::sizeX()));
    // Set the size y slider to the sithe y value only if the size must not be squared
    if (m_ui.buttonKeepSizeSquare->keepAspectRatio()) {
        m_ui.sliderSizeY->setValue(config->getDouble("size_x", KisScreentoneConfigDefaults::sizeX()));
    } else {
        m_ui.sliderSizeY->setValue(config->getDouble("size_y", KisScreentoneConfigDefaults::sizeY()));
    }
    m_ui.sliderShearX->setValue(config->getDouble("shear_x", KisScreentoneConfigDefaults::shearX()));
    m_ui.sliderShearY->setValue(config->getDouble("shear_y", KisScreentoneConfigDefaults::shearY()));
    m_ui.angleSelectorRotation->setAngle(config->getDouble("rotation", KisScreentoneConfigDefaults::rotation()));

    blockSignals(false);
    emit sigConfigurationUpdated();
}

KisPropertiesConfigurationSP KisScreentoneConfigWidget::configuration() const
{
    QVariant v;
    KoColor c;
    KisFilterConfigurationSP config = new KisFilterConfiguration("screentone", 1, KisGlobalResourcesInterface::instance());
    config->setProperty("pattern",  m_ui.comboBoxPattern->currentIndex());
    config->setProperty("shape",  m_ui.comboBoxShape->currentIndex());
    config->setProperty("interpolation",  m_ui.comboBoxInterpolation->currentIndex());
    c.fromKoColor(m_ui.buttonForegroundColor->color());
    v.setValue(c);
    config->setProperty("foreground_color", v);
    config->setProperty("foreground_opacity", m_ui.sliderForegroundOpacity->value());
    c.fromKoColor(m_ui.buttonBackgroundColor->color());
    v.setValue(c);
    config->setProperty("background_color", v);
    config->setProperty("background_opacity", m_ui.sliderBackgroundOpacity->value());
    config->setProperty("invert", m_ui.checkBoxInvert->isChecked());
    config->setProperty("brightness", m_ui.sliderBrightness->value());
    config->setProperty("contrast", m_ui.sliderContrast->value());
    config->setProperty("position_x", m_ui.sliderPositionX->value());
    config->setProperty("position_y", m_ui.sliderPositionY->value());
    config->setProperty("size_x", m_ui.sliderSizeX->value());
    config->setProperty("size_y", m_ui.sliderSizeY->value());
    config->setProperty("keep_size_square", m_ui.buttonKeepSizeSquare->keepAspectRatio());
    config->setProperty("shear_x", m_ui.sliderShearX->value());
    config->setProperty("shear_y", m_ui.sliderShearY->value());
    config->setProperty("rotation", m_ui.angleSelectorRotation->angle());
    return config;
}

void KisScreentoneConfigWidget::setupPatternComboBox()
{
    m_ui.comboBoxPattern->clear();
    m_ui.comboBoxPattern->addItems(screentonePatternNames());
}

void KisScreentoneConfigWidget::setupShapeComboBox()
{
    m_ui.comboBoxShape->clear();
    QStringList names = screentoneShapeNames(m_ui.comboBoxPattern->currentIndex());
    if (names.isEmpty()) {
        m_ui.labelShape->hide();
        m_ui.comboBoxShape->hide();
    } else {
        m_ui.comboBoxShape->addItems(names);
        m_ui.labelShape->show();
        m_ui.comboBoxShape->show();
    }
}

void KisScreentoneConfigWidget::setupInterpolationComboBox()
{
    m_ui.comboBoxInterpolation->clear();
    QStringList names =
        screentoneInterpolationNames(m_ui.comboBoxPattern->currentIndex(), m_ui.comboBoxShape->currentIndex());
    if (names.isEmpty()) {
        m_ui.labelInterpolation->hide();
        m_ui.comboBoxInterpolation->hide();
    } else {
        m_ui.comboBoxInterpolation->addItems(names);
        m_ui.labelInterpolation->show();
        m_ui.comboBoxInterpolation->show();
    }
}

void KisScreentoneConfigWidget::slot_comboBoxPattern_currentIndexChanged(int)
{
    m_ui.comboBoxShape->blockSignals(true);
    m_ui.comboBoxInterpolation->blockSignals(true);
    setupShapeComboBox();
    setupInterpolationComboBox();
    m_ui.comboBoxShape->blockSignals(false);
    m_ui.comboBoxInterpolation->blockSignals(false);
    emit sigConfigurationUpdated();
}

void KisScreentoneConfigWidget::slot_comboBoxShape_currentIndexChanged(int)
{
    m_ui.comboBoxInterpolation->blockSignals(true);
    setupInterpolationComboBox();
    m_ui.comboBoxInterpolation->blockSignals(false);
    emit sigConfigurationUpdated();
}

void KisScreentoneConfigWidget::slot_sliderSizeX_valueChanged(qreal value)
{
    if (m_ui.buttonKeepSizeSquare->keepAspectRatio()) {
        m_ui.sliderSizeY->blockSignals(true);
        m_ui.sliderSizeY->setValue(value);
        m_ui.sliderSizeY->blockSignals(false);
    }
    emit sigConfigurationUpdated();
}

void KisScreentoneConfigWidget::slot_sliderSizeY_valueChanged(qreal value)
{
    if (m_ui.buttonKeepSizeSquare->keepAspectRatio()) {
        m_ui.sliderSizeX->blockSignals(true);
        m_ui.sliderSizeX->setValue(value);
        m_ui.sliderSizeX->blockSignals(false);
    }
    emit sigConfigurationUpdated();
}

void KisScreentoneConfigWidget::slot_buttonKeepSizeSquare_keepAspectRatioChanged(bool keep)
{
    if (keep) {
        slot_sliderSizeX_valueChanged(m_ui.sliderSizeX->value());
    }
}
