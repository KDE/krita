/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <webp/encode.h>

#include <kis_properties_configuration.h>

#include "dlg_webp_export.h"

KisDlgWebPExport::KisDlgWebPExport(QWidget *parent)
    : KisConfigWidget(parent)
{
    setupUi(this);

#if WEBP_ENCODER_ABI_VERSION < 0x020f
    qMin->setEnabled(false);
    qMax->setEnabled(false);
#endif

    preset->addItem(i18nc("WebP presets", "Default"), WEBP_PRESET_DEFAULT);
    preset->addItem(i18nc("WebP presets", "Portrait"), WEBP_PRESET_PICTURE);
    preset->addItem(i18nc("WebP presets", "Outdoor photo"), WEBP_PRESET_PHOTO);
    preset->addItem(i18nc("WebP presets", "Line drawing"), WEBP_PRESET_DRAWING);
    preset->addItem(i18nc("WebP presets", "Icon"), WEBP_PRESET_ICON);
    preset->addItem(i18nc("WebP presets", "Text"), WEBP_PRESET_TEXT);

    filterType->addItem(i18nc("WebP filters", "Simple"), 0);
    filterType->addItem(i18nc("WebP filters", "Strong"), 1);

    alphaCompression->addItem(i18nc("WebP alpha plane compression", "None"), 0);
    alphaCompression->addItem(i18nc("WebP alpha plane compression", "Lossless"), 1);

    preprocessing->addItem(i18nc("WebP preprocessing filters", "None"), 0);
    preprocessing->addItem(i18nc("WebP preprocessing filters", "Segment-smooth"), 1);
    preprocessing->addItem(i18nc("WebP preprocessing filters", "Pseudo-random dithering"), 2);

    targetPSNR->setDisplayUnit(false);
    targetPSNR->setSuffix(" dB");

    connect(preset, SIGNAL(currentIndexChanged(int)), this, SLOT(changePreset(void)));
    connect(lossless, SIGNAL(toggled(bool)), this, SLOT(changePreset(void)));
}

void KisDlgWebPExport::setConfiguration(const KisPropertiesConfigurationSP cfg)
{
    preset->setCurrentIndex(cfg->getInt("preset", 0));
    lossless->setChecked(cfg->getBool("lossless", true));
    quality->setValue(cfg->getDouble("quality", 75.0));
    tradeoff->setValue(cfg->getInt("method", 4));
    dithering->setChecked(cfg->getBool("dithering", true));

    targetSize->setValue(cfg->getInt("target_size", 0));
    targetPSNR->setValue(cfg->getDouble("target_PSNR", 0.0));
    segments->setValue(cfg->getInt("segments", 4));
    snsStrength->setValue(cfg->getInt("sns_strength", 50));
    filterStrength->setValue(cfg->getInt("filter_strength", 60));
    filterSharpness->setValue(cfg->getInt("filter_sharpness", 0));
    filterType->setCurrentIndex(cfg->getInt("filter_type", 1));
    autofilter->setChecked(cfg->getBool("autofilter", false));
    alphaCompression->setCurrentIndex(cfg->getInt("alpha_compression", 1));
    alphaFiltering->setValue(cfg->getInt("alpha_filtering", 1));
    alphaQuality->setValue(cfg->getInt("alpha_quality", 100));
    pass->setValue(cfg->getInt("pass", 1));
    showCompressed->setChecked(cfg->getBool("show_compressed", false));
    preprocessing->setCurrentIndex(cfg->getInt("preprocessing", 0));
    partitions->setValue(cfg->getInt("partitions", 0));
    partitionLimit->setValue(cfg->getInt("partition_limit", 0));
    emulateJPEGSize->setChecked(cfg->getBool("emulate_jpeg_size", false));
    threadLevel->setChecked(cfg->getBool("thread_level", false));
    lowMemory->setChecked(cfg->getBool("low_memory", false));
    nearLossless->setValue(cfg->getInt("near_lossless", 100));
    exact->setChecked(cfg->getBool("exact", 0));
    useSharpYUV->setChecked(cfg->getBool("use_sharp_yuv", false));
#if WEBP_ENCODER_ABI_VERSION >= 0x020f
    qMin->setValue(cfg->getInt("qmin", 0));
    qMax->setValue(cfg->getInt("qmax", 100));
#endif
}

void KisDlgWebPExport::changePreset()
{
    WebPConfig preset;

    if (!WebPConfigPreset(&preset, static_cast<WebPPreset>(this->preset->currentData().value<int>()), static_cast<float>(this->quality->value()))) {
        return;
    }

    if (this->lossless->isChecked() && !WebPConfigLosslessPreset(&preset, this->tradeoff->value())) {
        return;
    }

    quality->setValue(static_cast<double>(preset.quality));
    tradeoff->setValue(preset.method);

    targetSize->setValue(preset.target_size);
    targetPSNR->setValue(static_cast<double>(preset.target_PSNR));
    segments->setValue(preset.segments);
    snsStrength->setValue(preset.sns_strength);
    filterStrength->setValue(preset.filter_strength);
    filterSharpness->setValue(preset.filter_sharpness);
    filterType->setCurrentIndex(preset.filter_type);
    autofilter->setChecked(preset.autofilter == 1);
    alphaCompression->setCurrentIndex(alphaCompression->findData(preset.alpha_compression));
    alphaFiltering->setValue(preset.alpha_filtering);
    alphaQuality->setValue(preset.alpha_quality);
    pass->setValue(preset.pass);
    showCompressed->setChecked(preset.show_compressed == 1);
    preprocessing->setCurrentIndex(preprocessing->findData(preset.preprocessing));
    partitions->setValue(preset.partitions);
    partitionLimit->setValue(preset.partition_limit);
    emulateJPEGSize->setChecked(preset.emulate_jpeg_size == 1);
    threadLevel->setChecked(preset.thread_level > 0);
    lowMemory->setChecked(preset.low_memory == 1);
    nearLossless->setValue(preset.near_lossless);
    exact->setChecked(preset.exact == 1);
    useSharpYUV->setChecked(preset.use_sharp_yuv);
#if WEBP_ENCODER_ABI_VERSION >= 0x020f
    qMin->setValue(preset.qmin);
    qMax->setValue(preset.qmax);
#endif
}

KisPropertiesConfigurationSP KisDlgWebPExport::configuration() const
{
    KisPropertiesConfigurationSP cfg(new KisPropertiesConfiguration());

    cfg->setProperty("preset", preset->currentIndex());
    cfg->setProperty("lossless", lossless->isChecked());
    cfg->setProperty("quality", quality->value());
    cfg->setProperty("method", tradeoff->value());
    cfg->setProperty("dithering", dithering->isChecked());

    cfg->setProperty("target_size", targetSize->value());
    cfg->setProperty("target_PSNR", targetPSNR->value());
    cfg->setProperty("segments", segments->value());
    cfg->setProperty("sns_strength", snsStrength->value());
    cfg->setProperty("filter_strength", filterStrength->value());
    cfg->setProperty("filter_sharpness", filterSharpness->value());
    cfg->setProperty("filter_type", filterType->currentData().value<int>());
    cfg->setProperty("autofilter", autofilter->isChecked());
    cfg->setProperty("alpha_compression", alphaCompression->currentData().value<int>());
    cfg->setProperty("alpha_filtering", alphaFiltering->value());
    cfg->setProperty("alpha_quality", alphaQuality->value());
    cfg->setProperty("pass", pass->value());
    cfg->setProperty("show_compressed", showCompressed->isChecked());
    cfg->setProperty("preprocessing", preprocessing->currentData().value<int>());
    cfg->setProperty("partitions", partitions->value());
    cfg->setProperty("partition_limit", partitionLimit->value());
    cfg->setProperty("emulate_jpeg_size", emulateJPEGSize->isChecked());
    cfg->setProperty("thread_level", threadLevel->isChecked());
    cfg->setProperty("low_memory", lowMemory->isChecked());
    cfg->setProperty("near_lossless", nearLossless->value());
    cfg->setProperty("exact", exact->isChecked());
    cfg->setProperty("use_sharp_yuv", useSharpYUV->isChecked());
#if WEBP_ENCODER_ABI_VERSION >= 0x020f
    cfg->setProperty("qmin", qMin->value());
    cfg->setProperty("qmax", qMax->value());
#endif

    return cfg;
}
