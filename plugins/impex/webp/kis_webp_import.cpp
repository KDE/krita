/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <cstdint>
#include <memory>
#include <webp/decode.h>
#include <webp/types.h>

#include <KisDocument.h>
#include <KisImportExportErrorCode.h>
#include <KoDialog.h>
#include <kis_group_layer.h>
#include <kis_paint_layer.h>
#include <kis_properties_configuration.h>
#include <kpluginfactory.h>

#include "kis_webp_import.h"

K_PLUGIN_FACTORY_WITH_JSON(KisWebPImportFactory, "krita_webp_import.json", registerPlugin<KisWebPImport>();)

KisWebPImport::KisWebPImport(QObject *parent, const QVariantList &)
    : KisImportExportFilter(parent)
    , m_dialog(new KisDlgWebPImport())
{
}

KisWebPImport::~KisWebPImport()
{
}

KisPropertiesConfigurationSP KisWebPImport::defaultConfiguration(const QByteArray & /*from*/, const QByteArray & /*to*/) const
{
    KisPropertiesConfigurationSP cfg(new KisPropertiesConfiguration());

    WebPDecoderConfig preset;

    if (!WebPInitDecoderConfig(&preset)) {
        return cfg;
    }

    cfg->setProperty("alpha_dithering_strength", preset.options.alpha_dithering_strength == 1);
    cfg->setProperty("no_fancy_upsampling", preset.options.no_fancy_upsampling);
    cfg->setProperty("use_cropping", preset.options.use_cropping);
    cfg->setProperty("crop_left", preset.options.crop_left);
    cfg->setProperty("crop_top", preset.options.crop_top);
    cfg->setProperty("crop_width", preset.options.crop_width);
    cfg->setProperty("crop_height", preset.options.crop_height);
    cfg->setProperty("use_scaling", preset.options.use_scaling);
    cfg->setProperty("scaled_width", preset.options.scaled_width);
    cfg->setProperty("scaled_height", preset.options.scaled_height);
    cfg->setProperty("use_threads", preset.options.use_threads == 1);
    cfg->setProperty("use_dithering", false);
    cfg->setProperty("dithering_strength", preset.options.dithering_strength);
    cfg->setProperty("flip", preset.options.flip == 1);
    cfg->setProperty("alpha_dithering_strength", preset.options.alpha_dithering_strength);

    return cfg;
}

KisImportExportErrorCode KisWebPImport::convert(KisDocument *document, QIODevice *io, KisPropertiesConfigurationSP)
{
    const QByteArray buf = io->readAll();

    if (buf.isEmpty()) {
        return ImportExportCodes::ErrorWhileReading;
    }

    const uint8_t *data = reinterpret_cast<const uint8_t *>(buf.constData());
    const size_t data_size = static_cast<size_t>(buf.size());

    WebPDecoderConfig config;
    if (!WebPInitDecoderConfig(&config)) {
        dbgPlugins << "WebP decode config initialization failure";
        return ImportExportCodes::InternalError;
    }

    // Check image file format.
    {
        const VP8StatusCode result =WebPGetFeatures(data, data_size, &config.input);
        dbgPlugins << "WebP import validation status: " << result;
        switch (result) {
        case VP8_STATUS_OK:
            break;
        case VP8_STATUS_OUT_OF_MEMORY:
            return ImportExportCodes::InsufficientMemory;
        case VP8_STATUS_INVALID_PARAM:
            return ImportExportCodes::InternalError;
        case VP8_STATUS_BITSTREAM_ERROR:
            return ImportExportCodes::FileFormatIncorrect;
        case VP8_STATUS_UNSUPPORTED_FEATURE:
            return ImportExportCodes::FormatFeaturesUnsupported;
        case VP8_STATUS_SUSPENDED:
            return ImportExportCodes::InternalError;
        case VP8_STATUS_USER_ABORT:
            return ImportExportCodes::InternalError;
        case VP8_STATUS_NOT_ENOUGH_DATA:
            return ImportExportCodes::FileFormatIncorrect;
        }
    }

    {
        KisPropertiesConfigurationSP cfg(defaultConfiguration(QByteArray(), QByteArray()));

        cfg->setProperty("original_width", config.input.width);
        cfg->setProperty("original_height", config.input.height);
        cfg->setProperty("crop_width", config.input.width);
        cfg->setProperty("crop_height", config.input.height);
        cfg->setProperty("scaled_width", config.input.width);
        cfg->setProperty("scaled_height", config.input.height);
        cfg->setProperty("has_transparency", config.input.has_alpha);
        cfg->setProperty("format", config.input.format);
        cfg->setProperty("has_animation", config.input.has_animation);

        m_dialog->setConfiguration(cfg.data());
    }

    {
        if (m_dialog->exec() != QDialog::Accepted) {
            return ImportExportCodes::Cancelled;
        }

        KisPropertiesConfigurationSP cfg(m_dialog->configuration());

        // Krita follows BGRA layout (checks RgbU8).
        config.output.colorspace = MODE_BGRA;
        config.options.bypass_filtering = cfg->getBool("bypass_filtering", false) ? 1 : 0;
        config.options.no_fancy_upsampling = cfg->getBool("no_fancy_upsampling", false) ? 1 : 0;
        if (cfg->getBool("use_cropping", false)) {
            config.options.use_cropping = 1;
            config.options.crop_left = cfg->getInt("crop_left", 0);
            config.options.crop_top = cfg->getInt("crop_top", 0);
            config.options.crop_width = cfg->getInt("crop_width", config.input.width);
            config.options.crop_height = cfg->getInt("crop_height", config.input.height);
        }

        if (cfg->getBool("use_scaling", false)) {
            config.options.scaled_width = cfg->getInt("scaled_width", config.input.height);
            config.options.scaled_height = cfg->getInt("scaled_height", config.input.height);
        }

        config.options.use_threads = cfg->getBool("use_threads", false) ? 1 : 0;

        if (cfg->getBool("use_dithering", false)) {
            config.options.dithering_strength = cfg->getInt("dithering_strength", 0);
            config.options.alpha_dithering_strength = cfg->getInt("alpha_dithering_strength", 0);
        }

        config.options.flip = cfg->getBool("flip", false) ? 1 : 0;
    }

    {
        const VP8StatusCode result = WebPDecode(data, data_size, &config);
        dbgPlugins << "WebP import status: " << result;
        switch (result) {
        case VP8_STATUS_OK:
            break;
        case VP8_STATUS_OUT_OF_MEMORY:
            return ImportExportCodes::InsufficientMemory;
        case VP8_STATUS_INVALID_PARAM:
            return ImportExportCodes::InternalError;
        case VP8_STATUS_BITSTREAM_ERROR:
            return ImportExportCodes::FileFormatIncorrect;
        case VP8_STATUS_UNSUPPORTED_FEATURE:
            return ImportExportCodes::FormatFeaturesUnsupported;
        case VP8_STATUS_SUSPENDED:
            return ImportExportCodes::InternalError;
        case VP8_STATUS_USER_ABORT:
            return ImportExportCodes::InternalError;
        case VP8_STATUS_NOT_ENOUGH_DATA:
            return ImportExportCodes::FileFormatIncorrect;
        }
    }

    const KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(document->createUndoStore(), config.output.width, config.output.height, colorSpace, i18n("WebP Image"));

    KisPaintLayerSP layer(new KisPaintLayer(image, image->nextLayerName(), 255));
    layer->paintDevice()->writeBytes(config.output.u.RGBA.rgba, 0, 0, config.output.width, config.output.height);
    image->addNode(layer.data(), image->rootLayer().data());

    document->setCurrentImage(image);
    WebPFreeDecBuffer(&config.output);
    return ImportExportCodes::OK;
}

#include "kis_webp_import.moc"
