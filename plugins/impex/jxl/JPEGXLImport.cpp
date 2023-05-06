/*
 *  SPDX-FileCopyrightText: 2021 the JPEG XL Project Authors
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "JPEGXLImport.h"

#include <KisGlobalResourcesInterface.h>

#include <jxl/decode_cxx.h>
#include <jxl/resizable_parallel_runner_cxx.h>
#include <jxl/types.h>
#include <kpluginfactory.h>

#include <QBuffer>
#include <algorithm>
#include <array>
#include <cstring>
#include <map>

#include <KisDocument.h>
#include <KisImportExportErrorCode.h>
#include <KoColorModelStandardIds.h>
#include <KoColorProfile.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorTransferFunctions.h>
#include <KoConfig.h>
#include <dialogs/kis_dlg_hlg_import.h>
#include <filter/kis_filter.h>
#include <filter/kis_filter_configuration.h>
#include <filter/kis_filter_registry.h>
#include <kis_assert.h>
#include <kis_debug.h>
#include <kis_group_layer.h>
#include <kis_image_animation_interface.h>
#include <kis_iterator_ng.h>
#include <kis_meta_data_backend_registry.h>
#include <kis_paint_layer.h>
#include <kis_raster_keyframe_channel.h>

K_PLUGIN_FACTORY_WITH_JSON(ImportFactory, "krita_jxl_import.json", registerPlugin<JPEGXLImport>();)

static constexpr std::array<char, 4> exifTag = {'e', 'x', 'i', 'f'};
static constexpr std::array<char, 4> xmpTag = {'x', 'm', 'l', ' '};

class Q_DECL_HIDDEN JPEGXLImportData
{
public:
    JxlBasicInfo m_info{};
    JxlExtraChannelInfo m_extra{};
    JxlPixelFormat m_pixelFormat{};
    JxlFrameHeader m_header{};
    std::vector<quint8> m_rawData{};
    KisPaintDeviceSP m_currentFrame{nullptr};
    uint32_t cmykChannelID = 0;
    int m_nextFrameTime{0};
    int m_durationFrameInTicks{0};
    KoID m_colorID;
    KoID m_depthID;
    bool haveSpecialChannels = false;
    bool isCMYK = false;
    bool applyOOTF = true;
    float displayGamma = 1.2f;
    float displayNits = 1000.0;
    LinearizePolicy linearizePolicy = LinearizePolicy::KeepTheSame;
    const KoColorSpace *cs = nullptr;
    std::vector<quint8> kPlane;
    QVector<qreal> lCoef;
};

struct JxlImpSpcChannels {
    uint32_t index;
    std::vector<quint8> rawData;
    QString chName;
    KisLayerSP chLayer{nullptr};

    JxlImpSpcChannels(uint32_t index, QString chName)
        : index(index)
        , chName(std::move(chName))
    {
    }
};

template<LinearizePolicy policy>
inline float linearizeValueAsNeeded(float value)
{
    if (policy == LinearizePolicy::LinearFromPQ) {
        return removeSmpte2048Curve(value);
    } else if (policy == LinearizePolicy::LinearFromHLG) {
        return removeHLGCurve(value);
    } else if (policy == LinearizePolicy::LinearFromSMPTE428) {
        return removeSMPTE_ST_428Curve(value);
    }
    return value;
}

template<LinearizePolicy policy, typename T, typename std::enable_if_t<std::numeric_limits<T>::is_integer, int> = 1>
inline float value(const T *src, size_t ch)
{
    float v = float(src[ch]) / float(std::numeric_limits<T>::max());

    return linearizeValueAsNeeded<policy>(v);
}

template<LinearizePolicy policy, typename T, typename std::enable_if_t<!std::numeric_limits<T>::is_integer, int> = 1>
inline float value(const T *src, size_t ch)
{
    float v = float(src[ch]);

    return linearizeValueAsNeeded<policy>(v);
}

template<typename channelsType, bool swap, LinearizePolicy policy, bool applyOOTF>
inline void imageOutCallback(JPEGXLImportData &d)
{
    const uint32_t width = d.m_header.layer_info.xsize;
    const uint32_t height = d.m_header.layer_info.ysize;
    KisHLineIteratorSP it = d.m_currentFrame->createHLineIteratorNG(0, 0, width);

    const auto *src = reinterpret_cast<const channelsType *>(d.m_rawData.data());
    const uint32_t channels = d.m_pixelFormat.num_channels;

    if (policy != LinearizePolicy::KeepTheSame) {
        const KoColorSpace *cs = d.cs;
        const double *lCoef = d.lCoef.constData();
        QVector<float> pixelValues(static_cast<int>(cs->channelCount()));
        float *tmp = pixelValues.data();
        const quint32 alphaPos = cs->alphaPos();

        for (size_t j = 0; j < height; j++) {
            for (size_t i = 0; i < width; i++) {
                for (size_t i = 0; i < channels; i++) {
                    tmp[i] = 1.0;
                }

                for (size_t ch = 0; ch < channels; ch++) {
                    if (ch == alphaPos) {
                        tmp[ch] = value<LinearizePolicy::KeepTheSame, channelsType>(src, ch);
                    } else {
                        tmp[ch] = value<policy, channelsType>(src, ch);
                    }
                }

                if (swap) {
                    std::swap(tmp[0], tmp[2]);
                }

                if (policy == LinearizePolicy::LinearFromHLG && applyOOTF) {
                    applyHLGOOTF(tmp, lCoef, d.displayGamma, d.displayNits);
                }

                cs->fromNormalisedChannelsValue(it->rawData(), pixelValues);

                src += d.m_pixelFormat.num_channels;

                it->nextPixel();
            }
            it->nextRow();
        }
    } else {
        for (size_t j = 0; j < height; j++) {
            for (size_t i = 0; i < width; i++) {
                auto *dst = reinterpret_cast<channelsType *>(it->rawData());

                std::memcpy(dst, src, channels * sizeof(channelsType));

                if (swap) {
                    std::swap(dst[0], dst[2]);
                } else if (d.isCMYK) {
                    // Swap alpha and key channel for CMYK
                    std::swap(dst[3], dst[4]);
                }

                src += d.m_pixelFormat.num_channels;

                it->nextPixel();
            }
            it->nextRow();
        }
    }
}

template<typename channelsType, bool swap, LinearizePolicy policy>
inline void generateCallbackWithPolicy(JPEGXLImportData &d)
{
    if (d.applyOOTF) {
        imageOutCallback<channelsType, swap, policy, true>(d);
    } else {
        imageOutCallback<channelsType, swap, policy, false>(d);
    }
}

template<typename channelsType, bool swap>
inline void generateCallbackWithSwap(JPEGXLImportData &d)
{
    switch (d.linearizePolicy) {
    case LinearizePolicy::LinearFromPQ:
        generateCallbackWithPolicy<channelsType, swap, LinearizePolicy::LinearFromPQ>(d);
        break;
    case LinearizePolicy::LinearFromHLG:
        generateCallbackWithPolicy<channelsType, swap, LinearizePolicy::LinearFromHLG>(d);
        break;
    case LinearizePolicy::LinearFromSMPTE428:
        generateCallbackWithPolicy<channelsType, swap, LinearizePolicy::LinearFromSMPTE428>(d);
        break;
    case LinearizePolicy::KeepTheSame:
    default:
        generateCallbackWithPolicy<channelsType, swap, LinearizePolicy::KeepTheSame>(d);
        break;
    };
}

template<typename channelsType>
inline void generateCallbackWithType(JPEGXLImportData &d)
{
    if (d.m_colorID == RGBAColorModelID
        && (d.m_depthID == Integer8BitsColorDepthID || d.m_depthID == Integer16BitsColorDepthID)
        && d.linearizePolicy == LinearizePolicy::KeepTheSame) {
        generateCallbackWithSwap<channelsType, true>(d);
    } else {
        generateCallbackWithSwap<channelsType, false>(d);
    }
}

inline void generateCallback(JPEGXLImportData &d)
{
    switch (d.m_pixelFormat.data_type) {
    case JXL_TYPE_FLOAT:
        return generateCallbackWithType<float>(d);
    case JXL_TYPE_UINT8:
        return generateCallbackWithType<uint8_t>(d);
    case JXL_TYPE_UINT16:
        return generateCallbackWithType<uint16_t>(d);
#ifdef HAVE_OPENEXR
    case JXL_TYPE_FLOAT16:
        return generateCallbackWithType<half>(d);
        break;
#endif
    default:
        KIS_ASSERT_X(false, "JPEGXL::generateCallback", "Unknown image format!");
    }
}

JPEGXLImport::JPEGXLImport(QObject *parent, const QVariantList &)
    : KisImportExportFilter(parent)
{
}

KisImportExportErrorCode
JPEGXLImport::convert(KisDocument *document, QIODevice *io, KisPropertiesConfigurationSP /*configuration*/)
{
    if (!io->isReadable()) {
        errFile << "Cannot read image contents";
        return ImportExportCodes::NoAccessToRead;
    }

    JPEGXLImportData d{};

    // Multi-threaded parallel runner.
    auto runner = JxlResizableParallelRunnerMake(nullptr);
    auto dec = JxlDecoderMake(nullptr);

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(runner && dec, ImportExportCodes::InternalError);

    // Set coalescing FALSE to enable layered JXL
    if (JXL_DEC_SUCCESS != JxlDecoderSetCoalescing(dec.get(), JXL_FALSE)) {
        errFile << "JxlDecoderSetCoalescing failed";
        return ImportExportCodes::InternalError;
    }
    bool decSetCoalescing = false;

    if (JXL_DEC_SUCCESS
        != JxlDecoderSubscribeEvents(dec.get(),
                                     JXL_DEC_BASIC_INFO | JXL_DEC_COLOR_ENCODING | JXL_DEC_FULL_IMAGE | JXL_DEC_BOX
                                         | JXL_DEC_FRAME)) {
        errFile << "JxlDecoderSubscribeEvents failed";
        return ImportExportCodes::InternalError;
    }

    if (JXL_DEC_SUCCESS != JxlDecoderSetParallelRunner(dec.get(), JxlResizableParallelRunner, runner.get())) {
        errFile << "JxlDecoderSetParallelRunner failed";
        return ImportExportCodes::InternalError;
    }

    const auto data = io->readAll();

    const auto validation =
        JxlSignatureCheck(reinterpret_cast<const uint8_t *>(data.constData()), static_cast<size_t>(data.size()));

    switch (validation) {
    case JXL_SIG_NOT_ENOUGH_BYTES:
        errFile << "Failed magic byte validation, not enough data";
        return ImportExportCodes::FileFormatIncorrect;
    case JXL_SIG_INVALID:
        errFile << "Failed magic byte validation, incorrect format";
        return ImportExportCodes::FileFormatIncorrect;
    default:
        break;
    }

    if (JXL_DEC_SUCCESS
        != JxlDecoderSetInput(dec.get(),
                              reinterpret_cast<const uint8_t *>(data.constData()),
                              static_cast<size_t>(data.size()))) {
        errFile << "JxlDecoderSetInput failed";
        return ImportExportCodes::InternalError;
    };
    JxlDecoderCloseInput(dec.get());
    if (JXL_DEC_SUCCESS != JxlDecoderSetDecompressBoxes(dec.get(), JXL_TRUE)) {
        errFile << "JxlDecoderSetDecompressBoxes failed";
        return ImportExportCodes::InternalError;
    };

    KisImageSP image{nullptr};
    KisLayerSP layer{nullptr};
    std::multimap<QByteArray, QByteArray> metadataBoxes;
    std::vector<JxlImpSpcChannels> specialChannels;
    std::vector<KisLayerSP> additionalLayers;
    bool bgLayerSet = false;
    QByteArray boxType(5, 0x0);
    QByteArray box(16384, 0x0);
    auto boxSize = box.size();

    // List of blend mode that we can currently support
    static constexpr std::array<JxlBlendMode, 2> supportedBlendMode = {JXL_BLEND_REPLACE, JXL_BLEND_BLEND};

    // Internal function to rewind decoder and enable coalescing
    auto rewindDecoderWithCoalesce = [&]() {
        JxlDecoderRewind(dec.get());
        if (JXL_DEC_SUCCESS != JxlDecoderSetCoalescing(dec.get(), JXL_TRUE)) {
            errFile << "JxlDecoderSetCoalescing failed";
            return false;
        }
        if (JXL_DEC_SUCCESS
            != JxlDecoderSubscribeEvents(dec.get(),
                                         JXL_DEC_BASIC_INFO | JXL_DEC_COLOR_ENCODING | JXL_DEC_FULL_IMAGE | JXL_DEC_BOX
                                             | JXL_DEC_FRAME)) {
            errFile << "JxlDecoderSubscribeEvents failed";
            return false;
        }
        if (JXL_DEC_SUCCESS != JxlDecoderSetParallelRunner(dec.get(), JxlResizableParallelRunner, runner.get())) {
            errFile << "JxlDecoderSetParallelRunner failed";
            return false;
        }
        if (JXL_DEC_SUCCESS
            != JxlDecoderSetInput(dec.get(),
                                  reinterpret_cast<const uint8_t *>(data.constData()),
                                  static_cast<size_t>(data.size()))) {
            errFile << "JxlDecoderSetInput failed";
            return false;
        };
        JxlDecoderCloseInput(dec.get());
        if (JXL_DEC_SUCCESS != JxlDecoderSetDecompressBoxes(dec.get(), JXL_TRUE)) {
            errFile << "JxlDecoderSetDecompressBoxes failed";
            return false;
        };
        decSetCoalescing = true;
        return true;
    };

    for (;;) {
        JxlDecoderStatus status = JxlDecoderProcessInput(dec.get());

        if (status == JXL_DEC_ERROR) {
            errFile << "Decoder error";
            return ImportExportCodes::InternalError;
        } else if (status == JXL_DEC_NEED_MORE_INPUT) {
            errFile << "Error, already provided all input";
            return ImportExportCodes::InternalError;
        } else if (status == JXL_DEC_BASIC_INFO) {
            if (JXL_DEC_SUCCESS != JxlDecoderGetBasicInfo(dec.get(), &d.m_info)) {
                errFile << "JxlDecoderGetBasicInfo failed";
                return ImportExportCodes::ErrorWhileReading;
            }
            // Coalesce frame on animation import
            if (d.m_info.have_animation && !decSetCoalescing) {
                if (!rewindDecoderWithCoalesce()) {
                    return ImportExportCodes::InternalError;
                } else {
                    continue;
                }
            }

            dbgFile << "Extra Channel[s] info:";
            for (uint32_t i = 0; i < d.m_info.num_extra_channels; i++) {
                if (JXL_DEC_SUCCESS != JxlDecoderGetExtraChannelInfo(dec.get(), i, &d.m_extra)) {
                    errFile << "JxlDecoderGetExtraChannelInfo failed";
                    break;
                }
                // Channel name references taken from libjxl repo:
                // https://github.com/libjxl/libjxl/blob/v0.8.0/lib/extras/enc/pnm.cc#L262
                // With added "JXL-" prefix to indicate that it comes from JXL image.
                const QString channelTypeString = [&]() {
                    switch (d.m_extra.type) {
                    case JXL_CHANNEL_ALPHA:
                        return QString("JXL-Alpha");
                    case JXL_CHANNEL_DEPTH:
                        return QString("JXL-Depth");
                    case JXL_CHANNEL_SPOT_COLOR:
                        return QString("JXL-SpotColor");
                    case JXL_CHANNEL_SELECTION_MASK:
                        return QString("JXL-SelectionMask");
                    case JXL_CHANNEL_BLACK:
                        return QString("JXL-Black");
                    case JXL_CHANNEL_CFA:
                        return QString("JXL-CFA");
                    case JXL_CHANNEL_THERMAL:
                        return QString("JXL-Thermal");
                    default:
                        return QString("JXL-UNKNOWN");
                    }
                }();

                // List all extra channels
                dbgFile << "index:" << i << " | type:" << channelTypeString;
                if (d.m_extra.type == JXL_CHANNEL_BLACK) {
                    d.isCMYK = true;
                    d.cmykChannelID = i;
                }
                if (d.m_extra.type != JXL_CHANNEL_BLACK && d.m_extra.type != JXL_CHANNEL_ALPHA) {
                    // Get channel name as well if it exists
                    if (d.m_extra.name_length) {
                        KIS_SAFE_ASSERT_RECOVER(d.m_extra.name_length < std::numeric_limits<int>::max())
                        {
                            document->setErrorMessage(i18nc("JPEG-XL", "Invalid length for extra channel %1 of type %2")
                                                          .arg(i)
                                                          .arg(channelTypeString));
                            return ImportExportCodes::FormatFeaturesUnsupported;
                        }
                        QByteArray rawNameExtra;
                        rawNameExtra.resize(static_cast<int>(d.m_extra.name_length + 1));
                        if (JXL_DEC_SUCCESS
                            != JxlDecoderGetExtraChannelName(dec.get(),
                                                             i,
                                                             rawNameExtra.data(),
                                                             static_cast<size_t>(rawNameExtra.size()))) {
                            errFile << "JxlDecoderGetExtraChannelName failed";
                            break;
                        }
                        dbgFile << "\tname:" << QString(rawNameExtra);
                        const QString fullNameString = channelTypeString + "/" + QString(rawNameExtra);
                        specialChannels.emplace_back(i, fullNameString);
                    } else {
                        specialChannels.emplace_back(i, channelTypeString);
                    }
                    d.haveSpecialChannels = true;
                }
            }

            dbgFile << "Info";
            dbgFile << "Size:" << d.m_info.xsize << "x" << d.m_info.ysize;
            dbgFile << "Depth:" << d.m_info.bits_per_sample << d.m_info.exponent_bits_per_sample;
            dbgFile << "Number of color channels:" << d.m_info.num_color_channels;
            dbgFile << "Number of extra channels:" << d.m_info.num_extra_channels;
            dbgFile << "Extra channels depth:" << d.m_info.alpha_bits << d.m_info.alpha_exponent_bits;
            dbgFile << "Has animation:" << d.m_info.have_animation << "loops:" << d.m_info.animation.num_loops
                    << "tick:" << d.m_info.animation.tps_numerator << d.m_info.animation.tps_denominator;
            JxlResizableParallelRunnerSetThreads(
                runner.get(),
                JxlResizableParallelRunnerSuggestThreads(d.m_info.xsize, d.m_info.ysize));

            if (d.m_info.exponent_bits_per_sample != 0) {
                if (d.m_info.bits_per_sample <= 16) {
                    d.m_pixelFormat.data_type = JXL_TYPE_FLOAT16;
                    d.m_depthID = Float16BitsColorDepthID;
                } else if (d.m_info.bits_per_sample <= 32) {
                    d.m_pixelFormat.data_type = JXL_TYPE_FLOAT;
                    d.m_depthID = Float32BitsColorDepthID;
                } else {
                    errFile << "Unsupported JPEG-XL input depth" << d.m_info.bits_per_sample
                            << d.m_info.exponent_bits_per_sample;
                    return ImportExportCodes::FormatFeaturesUnsupported;
                }
            } else if (d.m_info.bits_per_sample <= 8) {
                d.m_pixelFormat.data_type = JXL_TYPE_UINT8;
                d.m_depthID = Integer8BitsColorDepthID;
            } else if (d.m_info.bits_per_sample <= 16) {
                d.m_pixelFormat.data_type = JXL_TYPE_UINT16;
                d.m_depthID = Integer16BitsColorDepthID;
            } else {
                errFile << "Unsupported JPEG-XL input depth" << d.m_info.bits_per_sample
                        << d.m_info.exponent_bits_per_sample;
                return ImportExportCodes::FormatFeaturesUnsupported;
            }

            if (d.m_info.num_color_channels == 1) {
                // Grayscale
                d.m_pixelFormat.num_channels = 2;
                d.m_colorID = GrayAColorModelID;
            } else if (d.m_info.num_color_channels == 3 && !d.isCMYK) {
                // RGBA
                d.m_pixelFormat.num_channels = 4;
                d.m_colorID = RGBAColorModelID;
            } else if (d.m_info.num_color_channels == 3 && d.isCMYK) {
                // CMYKA
                d.m_pixelFormat.num_channels = 4;
                d.m_colorID = CMYKAColorModelID;
            } else {
                warnFile << "Forcing a RGBA conversion, unknown color space";
                d.m_pixelFormat.num_channels = 4;
                d.m_colorID = RGBAColorModelID;
            }
        } else if (status == JXL_DEC_COLOR_ENCODING) {
            // Determine color space information
            const KoColorProfile *profile = nullptr;

            // Chrome way of decoding JXL implies first scanning
            // the CICP encoding for HDR, and only afterwards
            // falling back to ICC.
            JxlColorEncoding colorEncoding{};
            if (JXL_DEC_SUCCESS
                == JxlDecoderGetColorAsEncodedProfile(dec.get(),
                                                      nullptr,
                                                      JXL_COLOR_PROFILE_TARGET_DATA,
                                                      &colorEncoding)) {
                const TransferCharacteristics transferFunction = [&]() {
                    switch (colorEncoding.transfer_function) {
                    case JXL_TRANSFER_FUNCTION_PQ: {
                        dbgFile << "linearizing from PQ";
                        d.linearizePolicy = LinearizePolicy::LinearFromPQ;
                        return TRC_LINEAR;
                    }
                    case JXL_TRANSFER_FUNCTION_HLG: {
                        dbgFile << "linearizing from HLG";
                        if (!document->fileBatchMode()) {
                            KisDlgHLGImport dlg(d.applyOOTF, d.displayGamma, d.displayNits);
                            dlg.exec();
                            d.applyOOTF = dlg.applyOOTF();
                            d.displayGamma = dlg.gamma();
                            d.displayNits = dlg.nominalPeakBrightness();
                        }
                        d.linearizePolicy = LinearizePolicy::LinearFromHLG;
                        return TRC_LINEAR;
                    }
                    case JXL_TRANSFER_FUNCTION_DCI: {
                        dbgFile << "linearizing from SMPTE 428";
                        d.linearizePolicy = LinearizePolicy::LinearFromSMPTE428;
                        return TRC_LINEAR;
                    }
                    case JXL_TRANSFER_FUNCTION_709:
                        return TRC_ITU_R_BT_709_5;
                    case JXL_TRANSFER_FUNCTION_SRGB:
                        return TRC_IEC_61966_2_1;
                    case JXL_TRANSFER_FUNCTION_GAMMA: {
                        // Using roughly the same logic in KoColorProfile.
                        const double estGamma = 1.0 / colorEncoding.gamma;
                        const double error = 0.0001;
                        // ICC v2 u8Fixed8Number calculation
                        // Or can be prequantized as 1.80078125, courtesy of Elle Stone
                        if ((std::fabs(estGamma - 1.8) < error) || (std::fabs(estGamma - (461.0 / 256.0)) < error)) {
                            return TRC_GAMMA_1_8;
                        } else if (std::fabs(estGamma - 2.2) < error) {
                            return TRC_ITU_R_BT_470_6_SYSTEM_M;
                        } else if (std::fabs(estGamma - (563.0 / 256.0)) < error) {
                            return TRC_A98;
                        } else if (std::fabs(estGamma - 2.4) < error) {
                            return TRC_GAMMA_2_4;
                        } else if (std::fabs(estGamma - 2.8) < error) {
                            return TRC_ITU_R_BT_470_6_SYSTEM_B_G;
                        } else {
                            warnFile << "Found custom estimated gamma value for JXL color space" << estGamma;
                            return TRC_UNSPECIFIED;
                        }
                    }
                    case JXL_TRANSFER_FUNCTION_LINEAR:
                        return TRC_LINEAR;
                    case JXL_TRANSFER_FUNCTION_UNKNOWN:
                    default:
                        warnFile << "Found unknown OETF";
                        return TRC_UNSPECIFIED;
                    }
                }();

                const ColorPrimaries colorPrimaries = [&]() {
                    switch (colorEncoding.primaries) {
                    case JXL_PRIMARIES_SRGB:
                        return PRIMARIES_ITU_R_BT_709_5;
                    case JXL_PRIMARIES_2100:
                        return PRIMARIES_ITU_R_BT_2020_2_AND_2100_0;
                    case JXL_PRIMARIES_P3:
                        return PRIMARIES_SMPTE_RP_431_2;
                    default:
                        return PRIMARIES_UNSPECIFIED;
                    }
                }();

                const QVector<double> colorants = [&]() -> QVector<double> {
                    if (colorEncoding.primaries != JXL_PRIMARIES_CUSTOM) {
                        return {};
                    } else {
                        return {colorEncoding.white_point_xy[0],
                                colorEncoding.white_point_xy[1],
                                colorEncoding.primaries_red_xy[0],
                                colorEncoding.primaries_red_xy[1],
                                colorEncoding.primaries_green_xy[0],
                                colorEncoding.primaries_green_xy[1],
                                colorEncoding.primaries_blue_xy[0],
                                colorEncoding.primaries_blue_xy[1]};
                    }
                }();

                profile = KoColorSpaceRegistry::instance()->profileFor(colorants, colorPrimaries, transferFunction);

                dbgFile << "CICP profile data:" << colorants << colorPrimaries << transferFunction;

                if (profile) {
                    dbgFile << "JXL CICP profile found" << profile->name();

                    if (d.linearizePolicy != LinearizePolicy::KeepTheSame) {
                        // Override output format!
                        d.m_depthID = Float32BitsColorDepthID;
                    }

                    d.cs = KoColorSpaceRegistry::instance()->colorSpace(d.m_colorID.id(), d.m_depthID.id(), profile);
                }
            }

            if (!d.cs) {
                dbgFile << "JXL CICP data couldn't be handled, falling back to ICC profile retrieval";
                size_t iccSize = 0;
                QByteArray iccProfile;
                if (JXL_DEC_SUCCESS
                    != JxlDecoderGetICCProfileSize(dec.get(), nullptr, JXL_COLOR_PROFILE_TARGET_DATA, &iccSize)) {
                    errFile << "ICC profile size retrieval failed";
                    document->setErrorMessage(i18nc("JPEG-XL errors", "Unable to read the image profile."));
                    return ImportExportCodes::ErrorWhileReading;
                }
                dbgFile << "JxlDecoderGetICCProfileSize succeeded, ICC profile available";
                iccProfile.resize(static_cast<int>(iccSize));
                if (JXL_DEC_SUCCESS
                    != JxlDecoderGetColorAsICCProfile(dec.get(),
                                                      nullptr,
                                                      JXL_COLOR_PROFILE_TARGET_DATA,
                                                      reinterpret_cast<uint8_t *>(iccProfile.data()),
                                                      static_cast<size_t>(iccProfile.size()))) {
                    document->setErrorMessage(i18nc("JPEG-XL errors", "Unable to read the image profile."));
                    return ImportExportCodes::ErrorWhileReading;
                }

                // With the profile in hand, now we can create the image.
                profile = KoColorSpaceRegistry::instance()->createColorProfile(d.m_colorID.id(),
                                                                               d.m_depthID.id(),
                                                                               iccProfile);
                d.cs = KoColorSpaceRegistry::instance()->colorSpace(d.m_colorID.id(), d.m_depthID.id(), profile);
            }

            dbgFile << "Color space:" << d.cs->name() << d.cs->colorModelId() << d.cs->colorDepthId();
            dbgFile << "JXL depth" << d.m_pixelFormat.data_type;

            d.lCoef = d.cs->lumaCoefficients();

            image = new KisImage(document->createUndoStore(),
                                 static_cast<int>(d.m_info.xsize),
                                 static_cast<int>(d.m_info.ysize),
                                 d.cs,
                                 "JPEG-XL image");

            layer = new KisPaintLayer(image, image->nextLayerName(), UCHAR_MAX);
            // Prepare layers for special channels
            for (JxlImpSpcChannels &spc : specialChannels) {
                spc.chLayer = new KisPaintLayer(image, spc.chName, UCHAR_MAX);
                // Set special channels to be hidden by default,
                // to make sure that the base image is displayed first when
                // opening a JXL file, just to prevent confusion.
                spc.chLayer->setVisible(false);
            }
        } else if (status == JXL_DEC_NEED_IMAGE_OUT_BUFFER) {
            d.m_currentFrame = new KisPaintDevice(image->colorSpace());

            // Use raw byte buffer instead of image callback
            size_t rawSize = 0;
            if (JXL_DEC_SUCCESS != JxlDecoderImageOutBufferSize(dec.get(), &d.m_pixelFormat, &rawSize)) {
                qWarning() << "JxlDecoderImageOutBufferSize failed";
                return ImportExportCodes::InternalError;
            }
            d.m_rawData.resize(rawSize);
            if (JXL_DEC_SUCCESS
                != JxlDecoderSetImageOutBuffer(dec.get(),
                                               &d.m_pixelFormat,
                                               reinterpret_cast<uint8_t *>(d.m_rawData.data()),
                                               static_cast<size_t>(d.m_rawData.size()))) {
                qWarning() << "JxlDecoderSetImageOutBuffer failed";
                return ImportExportCodes::InternalError;
            }

            if (d.isCMYK) {
                // Prepare planar buffer for key channel
                size_t bufferSize = 0;
                if (JXL_DEC_SUCCESS
                    != JxlDecoderExtraChannelBufferSize(dec.get(), &d.m_pixelFormat, &bufferSize, d.cmykChannelID)) {
                    errFile << "JxlDecoderExtraChannelBufferSize failed";
                    return ImportExportCodes::ErrorWhileReading;
                    break;
                }
                d.kPlane.resize(bufferSize);
                if (JXL_DEC_SUCCESS
                    != JxlDecoderSetExtraChannelBuffer(dec.get(),
                                                       &d.m_pixelFormat,
                                                       d.kPlane.data(),
                                                       bufferSize,
                                                       d.cmykChannelID)) {
                    errFile << "JxlDecoderSetExtraChannelBuffer failed";
                    return ImportExportCodes::ErrorWhileReading;
                    break;
                }
            }

            for (auto &spc : specialChannels) {
                size_t bufferSize = 0;
                if (JXL_DEC_SUCCESS
                    != JxlDecoderExtraChannelBufferSize(dec.get(), &d.m_pixelFormat, &bufferSize, spc.index)) {
                    errFile << "JxlDecoderExtraChannelBufferSize failed";
                    return ImportExportCodes::ErrorWhileReading;
                    break;
                }
                spc.rawData.resize(bufferSize);
                if (JXL_DEC_SUCCESS
                    != JxlDecoderSetExtraChannelBuffer(dec.get(),
                                                       &d.m_pixelFormat,
                                                       spc.rawData.data(),
                                                       bufferSize,
                                                       spc.index)) {
                    errFile << "JxlDecoderSetExtraChannelBuffer failed";
                    return ImportExportCodes::ErrorWhileReading;
                    break;
                }
            }
        } else if (status == JXL_DEC_FRAME) {
            if (JXL_DEC_SUCCESS != JxlDecoderGetFrameHeader(dec.get(), &d.m_header)) {
                errFile << "JxlDecoderGetFrameHeader failed";
                return ImportExportCodes::ErrorWhileReading;
            }

            const JxlBlendMode blendMode = d.m_header.layer_info.blend_info.blendmode;
            const bool isBlendSupported =
                std::find(supportedBlendMode.begin(), supportedBlendMode.end(), blendMode) != supportedBlendMode.end();

            if (!isBlendSupported && !decSetCoalescing) {
                warnFile << "Blending mode unsupported! Rewinding decoder with coalescing enabled";
                document->setWarningMessage(i18nc("JPEG-XL errors",
                                                  "Detected multi layer JPEG-XL image with unsupported blending mode, "
                                                  "importing flattened image."));
                if (!rewindDecoderWithCoalesce()) {
                    return ImportExportCodes::InternalError;
                } else {
                    additionalLayers.clear();
                    bgLayerSet = false;
                    if (d.haveSpecialChannels) {
                        specialChannels.clear();
                    }
                    continue;
                }
            }

            if (!d.m_info.have_animation) {
                QString layerName;
                QByteArray layerNameRaw;
                if (d.m_header.name_length) {
                    KIS_SAFE_ASSERT_RECOVER(d.m_header.name_length < std::numeric_limits<int>::max())
                    {
                        document->setErrorMessage(i18nc("JPEG-XL", "Invalid JPEG-XL layer name length"));
                        return ImportExportCodes::FormatFeaturesUnsupported;
                    }
                    layerNameRaw.resize(static_cast<int>(d.m_header.name_length + 1));
                    if (JXL_DEC_SUCCESS
                        != JxlDecoderGetFrameName(dec.get(),
                                                  layerNameRaw.data(),
                                                  static_cast<size_t>(layerNameRaw.size()))) {
                        errFile << "JxlDecoderGetFrameName failed";
                        break;
                    }
                    dbgFile << "\tlayer name:" << QString(layerNameRaw);
                    layerName = QString(layerNameRaw);
                } else {
                    layerName = QString("Layer");
                }
                // Set the first layer name (if any)
                if (!bgLayerSet) {
                    if (!layerNameRaw.isEmpty()) {
                        layer->setName(layerName);
                    }
                } else {
                    additionalLayers.emplace_back(new KisPaintLayer(image, layerName, UCHAR_MAX));
                }
            }
        } else if (status == JXL_DEC_FULL_IMAGE) {
            // Parse raw data using existing callback function
            generateCallback(d);
            const JxlLayerInfo layerInfo = d.m_header.layer_info;
            if (d.m_info.have_animation) {
                dbgFile << "Importing frame @" << d.m_nextFrameTime
                        << d.m_header.duration;

                if (d.m_nextFrameTime == 0) {
                    dbgFile << "Animation detected, ticks per second:"
                            << d.m_info.animation.tps_numerator
                            << d.m_info.animation.tps_denominator;
                    // XXX: How many ticks per second (FPS)?
                    // If > 240, block the derivation-- it's a stock JXL and
                    // Krita only supports up to 240 FPS.
                    // We'll try to derive the framerate from the first frame
                    // instead.
                    int framerate =
                        std::lround(d.m_info.animation.tps_numerator
                                    / static_cast<double>(
                                        d.m_info.animation.tps_denominator));
                    if (framerate > 240) {
                        warnFile << "JXL ticks per second value exceeds 240, "
                                    "approximating FPS from the duration of "
                                    "the first frame";
                        document->setWarningMessage(
                            i18nc("JPEG-XL errors",
                                  "The animation declares a frame rate of more "
                                  "than 240 FPS."));
                        const int approximatedFramerate = std::lround(
                            1000.0 / static_cast<double>(d.m_header.duration));
                        d.m_durationFrameInTicks =
                            static_cast<int>(d.m_header.duration);
                        framerate = std::max(approximatedFramerate, 1);
                    } else {
                        d.m_durationFrameInTicks = 1;
                    }
                    dbgFile << "Framerate:" << framerate;
                    layer->enableAnimation();
                    image->animationInterface()->setDocumentRangeStartFrame(0);
                    image->animationInterface()->setFramerate(framerate);
                }

                const int currentFrameTime = std::lround(
                    static_cast<double>(d.m_nextFrameTime)
                    / static_cast<double>(d.m_durationFrameInTicks));

                auto *channel = layer->getKeyframeChannel(KisKeyframeChannel::Raster.id(), true);
                auto *frame = dynamic_cast<KisRasterKeyframeChannel *>(channel);
                image->animationInterface()->setDocumentRangeEndFrame(
                    std::lround(static_cast<double>(d.m_nextFrameTime
                                                    + d.m_header.duration)
                                / static_cast<double>(d.m_durationFrameInTicks))
                    - 1);
                frame->importFrame(currentFrameTime, d.m_currentFrame, nullptr);
                d.m_nextFrameTime += static_cast<int>(d.m_header.duration);
            } else {
                if (d.isCMYK) {
                    QVector<quint8 *> planes = d.m_currentFrame->readPlanarBytes(0,
                                                                                 0,
                                                                                 static_cast<int>(layerInfo.xsize),
                                                                                 static_cast<int>(layerInfo.ysize));

                    // Planar buffer insertion for key channel
                    planes[3] = reinterpret_cast<quint8 *>(d.kPlane.data());
                    d.m_currentFrame->writePlanarBytes(planes,
                                                       0,
                                                       0,
                                                       static_cast<int>(layerInfo.xsize),
                                                       static_cast<int>(layerInfo.ysize));

                    // JPEG-XL decode outputs an inverted CMYK colors
                    // This one I took from kis_filter_test for inverting the colors..
                    const KisFilterSP f = KisFilterRegistry::instance()->value("invert");
                    KIS_ASSERT(f);
                    const KisFilterConfigurationSP kfc =
                        f->defaultConfiguration(KisGlobalResourcesInterface::instance());
                    KIS_ASSERT(kfc);
                    f->process(d.m_currentFrame,
                               {0, 0, static_cast<int>(layerInfo.xsize), static_cast<int>(layerInfo.ysize)},
                               kfc->cloneWithResourcesSnapshot());
                }
                if (d.haveSpecialChannels && !bgLayerSet) {
                    const quint8 *alphaRef =
                        d.m_currentFrame
                            ->readPlanarBytes(0, 0, static_cast<int>(d.m_info.xsize), static_cast<int>(d.m_info.ysize))
                            .last();
                    QVector<quint8 *> planes(static_cast<int>(image->colorSpace()->channelCount()));
                    for (auto &spc : specialChannels) {
                        if (d.m_colorID == RGBAColorModelID) {
                            planes[0] = reinterpret_cast<quint8 *>(spc.rawData.data());
                            planes[1] = reinterpret_cast<quint8 *>(spc.rawData.data());
                            planes[2] = reinterpret_cast<quint8 *>(spc.rawData.data());
                            planes[3] = const_cast<quint8 *>(alphaRef); /* inherit alpha */
                        } else if (d.m_colorID == GrayAColorModelID) {
                            planes[0] = reinterpret_cast<quint8 *>(spc.rawData.data());
                            planes[1] = const_cast<quint8 *>(alphaRef); /* inherit alpha */
                        } else if (d.m_colorID == CMYKAColorModelID) {
                            planes[0] = reinterpret_cast<quint8 *>(spc.rawData.data());
                            planes[1] = reinterpret_cast<quint8 *>(spc.rawData.data());
                            planes[2] = reinterpret_cast<quint8 *>(spc.rawData.data());
                            planes[3] = reinterpret_cast<quint8 *>(spc.rawData.data());
                            planes[4] = const_cast<quint8 *>(alphaRef); /* inherit alpha */
                        }
                        spc.chLayer->paintDevice()->writePlanarBytes(planes,
                                                                     0,
                                                                     0,
                                                                     static_cast<int>(d.m_info.xsize),
                                                                     static_cast<int>(d.m_info.ysize));
                    }
                }
                if (!bgLayerSet) {
                    layer->paintDevice()->makeCloneFrom(d.m_currentFrame, image->bounds());
                    bgLayerSet = true;
                } else {
                    additionalLayers.back()->paintDevice()->makeCloneFrom(d.m_currentFrame, image->bounds());
                    additionalLayers.back()->setX(static_cast<int>(layerInfo.crop_x0));
                    additionalLayers.back()->setY(static_cast<int>(layerInfo.crop_y0));
                }
            }
        } else if (status == JXL_DEC_SUCCESS || status == JXL_DEC_BOX) {
            if (std::strlen(boxType.data()) != 0) {
                // Release buffer and get its final size.
                const auto availOut = JxlDecoderReleaseBoxBuffer(dec.get());
                const int finalSize = box.size() - static_cast<int>(availOut);
                // Only resize and write boxes if it's not empty.
                // And only input metadata boxes while skipping other boxes.
                QByteArray type = boxType.toLower();
                if ((std::equal(exifTag.begin(), exifTag.end(), type.constBegin())
                     || std::equal(xmpTag.begin(), xmpTag.end(), type.constBegin()))
                    && finalSize != 0) {
                    metadataBoxes.emplace(type, QByteArray(box.data(), finalSize));
                }
                // Preemptively zero the box type out to prevent dangling
                // boxes.
                boxType.fill('\0');
            }
            if (status == JXL_DEC_SUCCESS) {
                // All decoding successfully finished.

                // Insert layer metadata if available (delayed
                // in case the boxes came before the BASIC_INFO event)
                for (auto &metaBox : metadataBoxes) {
                    const QByteArray &type = metaBox.first;
                    QByteArray &value = metaBox.second;
                    QBuffer buf(&value);
                    if (std::equal(exifTag.begin(), exifTag.end(), type.constBegin())) {
                        dbgFile << "Loading EXIF data. Size: " << value.size();

                        const auto *backend =
                            KisMetadataBackendRegistry::instance()->value(
                                "exif");

                        backend->loadFrom(layer->metaData(), &buf);
                    } else if (std::equal(xmpTag.begin(), xmpTag.end(), type.constBegin())) {
                        dbgFile << "Loading XMP or IPTC data. Size: " << value.size();

                        const auto *xmpBackend =
                            KisMetadataBackendRegistry::instance()->value(
                                "xmp");

                        if (!xmpBackend->loadFrom(layer->metaData(), &buf)) {
                            const KisMetaData::IOBackend *iptcBackend =
                                KisMetadataBackendRegistry::instance()->value(
                                    "iptc");
                            iptcBackend->loadFrom(layer->metaData(), &buf);
                        }
                    }
                }

                // It's not required to call JxlDecoderReleaseInput(dec.get()) here since
                // the decoder will be destroyed.
                image->addNode(layer, image->rootLayer().data());
                // Slip additional layers into layer stack
                for (const KisLayerSP &addLayer : additionalLayers) {
                    image->addNode(addLayer, image->rootLayer().data());
                }
                // Slip special channel layers into layer stack
                for (const JxlImpSpcChannels &spc : specialChannels) {
                    image->addNode(spc.chLayer, image->rootLayer().data());
                }
                document->setCurrentImage(image);
                return ImportExportCodes::OK;
            } else {
                if (JxlDecoderGetBoxType(dec.get(), boxType.data(), JXL_TRUE) != JXL_DEC_SUCCESS) {
                    errFile << "JxlDecoderGetBoxType failed";
                    return ImportExportCodes::ErrorWhileReading;
                }
                const QByteArray type = boxType.toLower();
                if (std::equal(exifTag.begin(), exifTag.end(), type.constBegin())
                    || std::equal(xmpTag.begin(), xmpTag.end(), type.constBegin())) {
                    if (JxlDecoderSetBoxBuffer(
                            dec.get(),
                            reinterpret_cast<uint8_t *>(box.data()),
                            static_cast<size_t>(box.size()))
                        != JXL_DEC_SUCCESS) {
                        errFile << "JxlDecoderSetBoxBuffer failed";
                        return ImportExportCodes::InternalError;
                    }
                } else {
                    dbgFile << "Skipping box" << boxType.data();
                }
            }
        } else if (status == JXL_DEC_BOX_NEED_MORE_OUTPUT) {
            // Update the box size if it was truncated in a previous buffering.
            boxSize = box.size();
            box.resize(boxSize * 2);
            // Release buffer before setting it up again
            JxlDecoderReleaseBoxBuffer(dec.get());
            if (JxlDecoderSetBoxBuffer(
                    dec.get(),
                    reinterpret_cast<uint8_t *>(box.data() + boxSize),
                    static_cast<size_t>(box.size() - boxSize))
                != JXL_DEC_SUCCESS) {
                errFile << "JxlDecoderGetBoxType failed";
                return ImportExportCodes::ErrorWhileReading;
            }
        } else {
            errFile << "Unknown decoder status" << status;
            return ImportExportCodes::InternalError;
        }
    }

    return ImportExportCodes::OK;
}

#include <JPEGXLImport.moc>
