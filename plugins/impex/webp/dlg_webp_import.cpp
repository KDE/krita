/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <webp/encode.h>

#include <kis_properties_configuration.h>

#include "dlg_webp_import.h"

KisDlgWebPImport::KisDlgWebPImport()
    : KoDialog()
    , m_rawWidget(new DlgWebPImport(this))
    , m_keepAspect(false)
    , m_aspectRatio(1)
    , m_newWidth(0)
    , m_newHeight(0)
{
    enableButtonApply(false);

    setMainWidget(m_rawWidget.get());

    m_rawWidget->hasAnimation->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_rawWidget->hasTransparency->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_rawWidget->isLossless->setAttribute(Qt::WA_TransparentForMouseEvents);

    m_rawWidget->crop->setChecked(false);
    m_rawWidget->cropX->setUnit(KoUnit(KoUnit::Pixel));
    m_rawWidget->cropY->setUnit(KoUnit(KoUnit::Pixel));
    m_rawWidget->cropWidth->setUnit(KoUnit(KoUnit::Pixel));
    m_rawWidget->cropHeight->setUnit(KoUnit(KoUnit::Pixel));

    m_widthUnitManager = new KisDocumentAwareSpinBoxUnitManager(this);
    m_heightUnitManager = new KisDocumentAwareSpinBoxUnitManager(this, KisDocumentAwareSpinBoxUnitManager::PIX_DIR_Y);
    m_widthUnitManager->setApparentUnitFromSymbol("px");
    m_heightUnitManager->setApparentUnitFromSymbol("px");

    m_rawWidget->scale->setChecked(false);
    m_rawWidget->aspectRatioBtn->setKeepAspectRatio(m_keepAspect);
    m_rawWidget->constrainProportionsCkb->setChecked(m_keepAspect);
    m_rawWidget->newWidthDouble->setUnitManager(m_widthUnitManager);
    m_rawWidget->newHeightDouble->setUnitManager(m_heightUnitManager);
    m_rawWidget->newWidthDouble->setDecimals(2);
    m_rawWidget->newHeightDouble->setDecimals(2);
    m_rawWidget->newWidthDouble->setDisplayUnit(false);
    m_rawWidget->newHeightDouble->setDisplayUnit(false);
    m_rawWidget->widthUnit->setModel(m_widthUnitManager);
    m_rawWidget->heightUnit->setModel(m_heightUnitManager);
    m_rawWidget->widthUnit->setCurrentIndex(m_widthUnitManager->getsUnitSymbolList().indexOf(m_widthUnitManager->getApparentUnitSymbol()));
    m_rawWidget->heightUnit->setCurrentIndex(m_heightUnitManager->getsUnitSymbolList().indexOf(m_widthUnitManager->getApparentUnitSymbol()));

    connect(m_rawWidget->newWidthDouble, SIGNAL(valueChangedPt(double)), this, SLOT(slotWidthChanged(double)));
    connect(m_rawWidget->newHeightDouble, SIGNAL(valueChangedPt(double)), this, SLOT(slotHeightChanged(double)));
    connect(m_widthUnitManager, SIGNAL(unitChanged(int)), m_rawWidget->widthUnit, SLOT(setCurrentIndex(int)));
    connect(m_heightUnitManager, SIGNAL(unitChanged(int)), m_rawWidget->heightUnit, SLOT(setCurrentIndex(int)));
    connect(m_rawWidget->widthUnit, SIGNAL(currentIndexChanged(int)), m_widthUnitManager, SLOT(selectApparentUnitFromIndex(int)));
    connect(m_rawWidget->heightUnit, SIGNAL(currentIndexChanged(int)), m_heightUnitManager, SLOT(selectApparentUnitFromIndex(int)));
    connect(m_rawWidget->constrainProportionsCkb, SIGNAL(toggled(bool)), this, SLOT(slotAspectChanged(bool)));
    connect(m_rawWidget->aspectRatioBtn, SIGNAL(keepAspectRatioChanged(bool)), this, SLOT(slotAspectChanged(bool)));
}

void KisDlgWebPImport::slotAspectChanged(bool keep)
{
    m_rawWidget->aspectRatioBtn->blockSignals(true);
    m_rawWidget->constrainProportionsCkb->blockSignals(true);

    m_rawWidget->aspectRatioBtn->setKeepAspectRatio(keep);
    m_rawWidget->constrainProportionsCkb->setChecked(keep);

    m_rawWidget->aspectRatioBtn->blockSignals(false);
    m_rawWidget->constrainProportionsCkb->blockSignals(false);

    m_keepAspect = keep;
}

void KisDlgWebPImport::slotWidthChanged(double v)
{
    // this slot receiv values in pt from the unitspinbox, so just use the resolution.
    const double resValue = v * m_widthUnitManager->getConversionFactor(KisSpinBoxUnitManager::LENGTH, "px");
    m_newWidth = qRound(resValue);

    if (m_keepAspect) {
        m_newHeight = qRound(m_newWidth / m_aspectRatio);
        m_rawWidget->newHeightDouble->blockSignals(true);
        m_rawWidget->newHeightDouble->changeValue(v * m_aspectRatio);
        m_rawWidget->newHeightDouble->blockSignals(false);
    } else {
        m_aspectRatio = m_newWidth / m_newHeight;
    }
}

void KisDlgWebPImport::slotHeightChanged(double v)
{
    // this slot receiv values in pt from the unitspinbox, so just use the resolution.
    const double resValue = v * m_heightUnitManager->getConversionFactor(KisSpinBoxUnitManager::LENGTH, "px");
    m_newHeight = qRound(resValue);

    if (m_keepAspect) {
        m_newWidth = qRound(m_newHeight * m_aspectRatio);
        m_rawWidget->newWidthDouble->blockSignals(true);
        m_rawWidget->newWidthDouble->changeValue(v * m_aspectRatio);
        m_rawWidget->newWidthDouble->blockSignals(false);
    } else {
        m_aspectRatio = m_newWidth / m_newHeight;
    }
}

void KisDlgWebPImport::setConfiguration(const KisPropertiesConfigurationSP cfg)
{
    m_rawWidget->originalHeight->setText(i18n("%1 px", cfg->getInt("original_height", 0)));
    m_rawWidget->originalWidth->setText(i18n("%1 px", cfg->getInt("original_width", 0)));
    m_rawWidget->originalWidth->setText(i18n("%1 px", cfg->getInt("original_width", 0)));

    m_rawWidget->hasTransparency->setChecked(cfg->getBool("has_transparency", false));
    switch (cfg->getInt("format", 0)) {
        case 1:
            m_rawWidget->isLossless->setCheckState(Qt::Unchecked);
            break;
        case 2:
            m_rawWidget->isLossless->setCheckState(Qt::Checked);
            break;
        case 0:
        default:
            m_rawWidget->isLossless->setCheckState(Qt::PartiallyChecked);
            break;
    }
    m_rawWidget->hasAnimation->setChecked(cfg->getBool("has_animation", false));

    m_rawWidget->crop->setChecked(cfg->getBool("use_cropping", false));
    m_rawWidget->cropX->setValue(cfg->getDouble("crop_left", 0));
    m_rawWidget->cropY->setValue(cfg->getDouble("crop_top", 0));
    m_rawWidget->cropWidth->setValue(cfg->getDouble("crop_width", 0));
    m_rawWidget->cropHeight->setValue(cfg->getDouble("crop_height", 0));

    m_rawWidget->scale->setChecked(cfg->getBool("use_scaling", false));
    m_rawWidget->newWidthDouble->setValue(cfg->getDouble("scaled_width", 0));
    m_rawWidget->newHeightDouble->setValue(cfg->getDouble("scaled_height", 0));

    m_rawWidget->flip->setChecked(cfg->getBool("flip", false));

    m_rawWidget->dithering->setChecked(cfg->getBool("use_dithering", false));
    m_rawWidget->ditheringStrength->setValue(cfg->getInt("dithering_strength", 0));
    m_rawWidget->alphaDitheringStrength->setValue(cfg->getInt("alpha_dithering_strength", 0));

    m_rawWidget->noFancyUpsampling->setChecked(cfg->getBool("no_fancy_upsampling", false));
    m_rawWidget->useThreads->setChecked(cfg->getBool("use_threads", false));
}

KisPropertiesConfigurationSP KisDlgWebPImport::configuration() const
{
    KisPropertiesConfigurationSP cfg(new KisPropertiesConfiguration());

    const double conversionFactorW {m_widthUnitManager->getConversionFactor(KisSpinBoxUnitManager::LENGTH, "px")};

    const double conversionFactorH {m_heightUnitManager->getConversionFactor(KisSpinBoxUnitManager::LENGTH, "px")};

    cfg->setProperty("use_cropping", m_rawWidget->crop->isChecked());
    cfg->setProperty("crop_left", m_rawWidget->cropX->value());
    cfg->setProperty("crop_top", m_rawWidget->cropY->value());
    cfg->setProperty("crop_width", m_rawWidget->cropWidth->value());
    cfg->setProperty("crop_height", m_rawWidget->cropHeight->value());

    cfg->setProperty("use_scaling", m_rawWidget->scale->isChecked());
    cfg->setProperty("scaled_width", qRound(m_rawWidget->newWidthDouble->value() * conversionFactorW));
    cfg->setProperty("scaled_height", qRound(m_rawWidget->newHeightDouble->value() * conversionFactorH));

    cfg->setProperty("flip", m_rawWidget->flip->isChecked());

    cfg->setProperty("use_dithering", m_rawWidget->dithering->isChecked());
    cfg->setProperty("dithering_strength", m_rawWidget->ditheringStrength->value());
    cfg->setProperty("alpha_dithering_strength", m_rawWidget->alphaDitheringStrength->value());

    cfg->setProperty("no_fancy_upsampling", m_rawWidget->noFancyUpsampling->isChecked());
    cfg->setProperty("use_threads", m_rawWidget->useThreads->isChecked());

    return cfg;
}
