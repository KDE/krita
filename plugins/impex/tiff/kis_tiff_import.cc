/*
 *  SPDX-FileCopyrightText: 2005 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tiff_import.h"
#include "KisImportExportErrorCode.h"
#include "kis_assert.h"

#include <QBuffer>
#include <QPair>
#include <QSharedPointer>
#include <QStack>

#include <array>

#include <exiv2/exiv2.hpp>
#include <kpluginfactory.h>
#ifdef Q_OS_WIN
#include <io.h>
#endif
#include <tiffio.h>

#include <KisDocument.h>
#include <KisImportExportAdditionalChecks.h>
#include <KisViewManager.h>
#include <KoColorProfile.h>
#include <KoDocumentInfo.h>
#include <KoUnit.h>
#include <KisExiv2IODevice.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_meta_data_backend_registry.h>
#include <kis_meta_data_tags.h>
#include <kis_paint_layer.h>
#include <kis_transform_worker.h>
#include <kis_transparency_mask.h>

#ifdef TIFF_HAS_PSD_TAGS
#include <psd_resource_block.h>

#include "kis_tiff_psd_layer_record.h"
#include "kis_tiff_psd_resource_record.h"

#include <KisImportUserFeedbackInterface.h>
#include <QMessageBox>
#endif /* TIFF_HAS_PSD_TAGS */

#ifdef HAVE_JPEG_TURBO
#include <turbojpeg.h>
#endif

#include "kis_buffer_stream.h"
#include "kis_tiff_logger.h"
#include "kis_tiff_reader.h"
#include "kis_tiff_ycbcr_reader.h"

enum class TiffResolution : quint8 {
    NONE = RESUNIT_NONE,
    INCH = RESUNIT_INCH,
    CM = RESUNIT_CENTIMETER,
};

struct KisTiffBasicInfo {
    uint32_t width{};
    uint32_t height{};
    float x{};
    float y{};
    float xres{};
    float yres{};
    uint16_t depth{};
    uint16_t sampletype{};
    uint16_t nbchannels{};
    uint16_t color_type{};
    uint16_t *sampleinfo = nullptr;
    uint16_t extrasamplescount = 0;
    const KoColorSpace *cs = nullptr;
    QPair<QString, QString> colorSpaceIdTag;
    KoColorTransformation *transform = nullptr;
    uint8_t dstDepth{};
    TiffResolution resolution = TiffResolution::NONE;
};

K_PLUGIN_FACTORY_WITH_JSON(TIFFImportFactory,
                           "krita_tiff_import.json",
                           registerPlugin<KisTIFFImport>();)

QPair<QString, QString> getColorSpaceForColorType(uint16_t sampletype,
                                                  uint16_t color_type,
                                                  uint16_t color_nb_bits,
                                                  TIFF *image,
                                                  uint16_t &nbchannels,
                                                  uint16_t &extrasamplescount,
                                                  uint8_t &destDepth)
{
    const int bits32 = 32;
    const int bits16 = 16;
    const int bits8 = 8;

    if (sampletype == SAMPLEFORMAT_INT) {
        dbgFile << "Detected signed TIFF image" << color_type << color_nb_bits;
    }

    if (color_type == PHOTOMETRIC_MINISWHITE
        || color_type == PHOTOMETRIC_MINISBLACK) {
        if (nbchannels == 0)
            nbchannels = 1;
        extrasamplescount =
            nbchannels - 1; // FIX the extrasamples count in case of
        if (sampletype == SAMPLEFORMAT_IEEEFP) {
            if (color_nb_bits == 16) {
#ifdef HAVE_OPENEXR
                destDepth = 16;
                return {GrayAColorModelID.id(), Float16BitsColorDepthID.id()};
#endif
            } else if (color_nb_bits == 32) {
                destDepth = 32;
                return {GrayAColorModelID.id(), Float32BitsColorDepthID.id()};
            }
            return {}; // sanity check; no support for float of
                       // higher or lower bit depth
        }
        if (color_nb_bits <= 8) {
            destDepth = 8;
            return {GrayAColorModelID.id(), Integer8BitsColorDepthID.id()};
        } else /* if (color_nb_bits == bits16) */ {
            destDepth = 16;
            return {GrayAColorModelID.id(), Integer16BitsColorDepthID.id()};
        }

    } else if (color_type == PHOTOMETRIC_RGB /*|| color_type == */) {
        if (nbchannels == 0)
            nbchannels = 3;
        extrasamplescount =
            nbchannels - 3; // FIX the extrasamples count in case of
        if (sampletype == SAMPLEFORMAT_IEEEFP) {
            if (color_nb_bits == 16) {
#ifdef HAVE_OPENEXR
                destDepth = 16;
                return {RGBAColorModelID.id(), Float16BitsColorDepthID.id()};
#endif
            } else if (color_nb_bits == 32) {
                destDepth = 32;
                return {RGBAColorModelID.id(), Float32BitsColorDepthID.id()};
            }
            return {}; // sanity check; no support for float of
                       // higher or lower bit depth
        } else {
            if (color_nb_bits <= 8) {
                destDepth = 8;
                return {RGBAColorModelID.id(), Integer8BitsColorDepthID.id()};
            } else /* if (color_nb_bits == bits16) */ {
                destDepth = 16;
                return {RGBAColorModelID.id(), Integer16BitsColorDepthID.id()};
            }
        }
    } else if (color_type == PHOTOMETRIC_YCBCR) {
        if (nbchannels == 0)
            nbchannels = 3;
        extrasamplescount =
            nbchannels - 3; // FIX the extrasamples count in case of
        if (sampletype == SAMPLEFORMAT_IEEEFP) {
            if (color_nb_bits == 16) {
#ifdef HAVE_OPENEXR
                destDepth = 16;
                return {YCbCrAColorModelID.id(), Float16BitsColorDepthID.id()};
#endif
            } else if (color_nb_bits == 32) {
                destDepth = 32;
                return {YCbCrAColorModelID.id(), Float32BitsColorDepthID.id()};
            }
            return {}; // sanity check; no support for float of
                       // higher or lower bit depth
        } else {
            if (color_nb_bits <= 8) {
                destDepth = 8;
                return {YCbCrAColorModelID.id(), Integer8BitsColorDepthID.id()};
            } else /* if (color_nb_bits == bits16) */ {
                destDepth = 16;
                return {YCbCrAColorModelID.id(),
                        Integer16BitsColorDepthID.id()};
            }
        }
        if (color_nb_bits <= 8) {
            destDepth = 8;
            return {YCbCrAColorModelID.id(), Integer8BitsColorDepthID.id()};
        } else if (color_nb_bits == bits16) {
            destDepth = 16;
            return {YCbCrAColorModelID.id(), Integer16BitsColorDepthID.id()};
        } else {
            return {}; // sanity check; no support
                       // integers of higher bit depth
        }
    } else if (color_type == PHOTOMETRIC_SEPARATED) {
        if (nbchannels == 0)
            nbchannels = 4;
        // SEPARATED is in general CMYK but not always, so we check
        uint16_t inkset = 0;
        if ((TIFFGetField(image, TIFFTAG_INKSET, &inkset) == 0)) {
            dbgFile << "Image does not define the inkset.";
            inkset = 2;
        }
        if (inkset != INKSET_CMYK) {
            dbgFile << "Unsupported inkset (right now, only CMYK is supported)";
            char **ink_names = nullptr;
            uint16_t numberofinks = 0;
            if (TIFFGetField(image, TIFFTAG_INKNAMES, &ink_names) == 1
                && TIFFGetField(image, TIFFTAG_NUMBEROFINKS, &numberofinks)
                    == 1) {
                dbgFile << "Inks are :";
                for (uint32_t i = 0; i < numberofinks; i++) {
                    dbgFile << ink_names[i];
                }
            } else {
                dbgFile << "inknames are not defined !";
                // To be able to read stupid adobe files, if there are no
                // information about inks and four channels, then it's a CMYK
                // file :
                if (nbchannels - extrasamplescount != 4) {
                    return {};
                }
                // else - assume it's CMYK and proceed
            }
        }
        if (sampletype == SAMPLEFORMAT_IEEEFP) {
            if (color_nb_bits == 16) {
#ifdef HAVE_OPENEXR
                destDepth = 16;
                return {CMYKAColorModelID.id(), Float16BitsColorDepthID.id()};
#endif
            } else if (color_nb_bits == 32) {
                destDepth = 32;
                return {CMYKAColorModelID.id(), Float32BitsColorDepthID.id()};
            }
            return {}; // sanity check; no support for float of
                       // higher or lower bit depth
        }
        if (color_nb_bits <= 8) {
            destDepth = 8;
            return {CMYKAColorModelID.id(), Integer8BitsColorDepthID.id()};
        } else if (color_nb_bits == 16) {
            destDepth = 16;
            return {CMYKAColorModelID.id(), Integer16BitsColorDepthID.id()};
        } else {
            return {}; // no support for other bit depths
        }
    } else if (color_type == PHOTOMETRIC_CIELAB
               || color_type == PHOTOMETRIC_ICCLAB) {
        if (nbchannels == 0)
            nbchannels = 3;
        extrasamplescount = nbchannels - 3; // FIX the extrasamples count

        switch (color_nb_bits) {
        case bits32: {
            destDepth = bits32;
            if (sampletype == SAMPLEFORMAT_IEEEFP) {
                return {LABAColorModelID.id(), Float32BitsColorDepthID.id()};
            } else {
                return {}; // no support for other bit depths
            }
        }
        case bits16: {
            destDepth = bits16;
            if (sampletype == SAMPLEFORMAT_IEEEFP) {
#ifdef HAVE_OPENEXR
                return {LABAColorModelID.id(), Float16BitsColorDepthID.id()};
#endif
            } else {
                return {LABAColorModelID.id(), Integer16BitsColorDepthID.id()};
            }
            return {}; // no support for other bit depths
        }
        case bits8: {
            destDepth = bits8;
            return {LABAColorModelID.id(), Integer8BitsColorDepthID.id()};
        }
        default: {
            return {};
        }
        }
    } else if (color_type == PHOTOMETRIC_PALETTE) {
        destDepth = 16;
        if (nbchannels == 0)
            nbchannels = 2;
        extrasamplescount = nbchannels - 2; // FIX the extrasamples count
        // <-- we will convert the index image to RGBA16 as the palette is
        // always on 16bits colors
        return {RGBAColorModelID.id(), Integer16BitsColorDepthID.id()};
    }
    return {};
}

template<template<typename> class T>
QSharedPointer<KisTIFFPostProcessor>
makePostProcessor(uint32_t nbsamples, const QPair<QString, QString> &id)
{
    if (id.second == Integer8BitsColorDepthID.id()) {
        return QSharedPointer<T<uint8_t>>::create(nbsamples);
    } else if (id.second == Integer16BitsColorDepthID.id()) {
        return QSharedPointer<T<uint16_t>>::create(nbsamples);
#ifdef HAVE_OPENEXR
    } else if (id.second == Float16BitsColorDepthID.id()) {
        return QSharedPointer<T<half>>::create(nbsamples);
#endif
    } else if (id.second == Float32BitsColorDepthID.id()) {
        return QSharedPointer<T<float>>::create(nbsamples);
    } else {
        KIS_ASSERT(false && "TIFF does not support this bit depth!");
        return {};
    }
}

KisTIFFImport::KisTIFFImport(QObject *parent, const QVariantList &)
    : KisImportExportFilter(parent)
    , m_image(nullptr)
    , oldErrHandler(TIFFSetErrorHandler(&KisTiffErrorHandler))
    , oldWarnHandler(TIFFSetWarningHandler(&KisTiffWarningHandler))
{
}

KisTIFFImport::~KisTIFFImport()
{
    TIFFSetErrorHandler(oldErrHandler);
    TIFFSetWarningHandler(oldWarnHandler);
}

template<typename T, typename Deleter>
auto make_unique_with_deleter(T *data, Deleter d)
{
    return std::unique_ptr<T, Deleter>(data, d);
}

#ifdef TIFF_HAS_PSD_TAGS
KisImportExportErrorCode KisTIFFImport::readImageFromPsdRecords(
    KisDocument *m_doc,
    const KisTiffPsdLayerRecord &photoshopLayerRecord,
    KisTiffPsdResourceRecord &photoshopImageResourceRecord,
    QBuffer &photoshopLayerData,
    const KisTiffBasicInfo &basicInfo)
{
    QMap<KisTiffPsdResourceRecord::PSDResourceID, PSDResourceBlock *>
        &resources = photoshopImageResourceRecord.resources;

    const KoColorSpace *cs = basicInfo.cs;

    // Attempt to get the ICC profile from the image resource section
    if (resources.contains(KisTiffPsdResourceRecord::ICC_PROFILE)) {
        const KoColorProfile *profile = nullptr;

        // Use the color mode from the synthetic PSD header
        QPair<QString, QString> colorSpaceId =
            psd_colormode_to_colormodelid(photoshopLayerRecord.colorMode(),
                                          photoshopLayerRecord.channelDepth());

        if (colorSpaceId.first.isNull()) {
            dbgFile << "Inconsistent PSD metadata, the color space"
                    << photoshopLayerRecord.colorMode()
                    << photoshopLayerRecord.channelDepth()
                    << "does not exist; falling back to the synthetic header "
                       "information";
            colorSpaceId = basicInfo.colorSpaceIdTag;
        }

        ICC_PROFILE_1039 *iccProfileData = dynamic_cast<ICC_PROFILE_1039 *>(
            resources[KisTiffPsdResourceRecord::ICC_PROFILE]->resource);
        if (iccProfileData) {
            profile = KoColorSpaceRegistry::instance()->createColorProfile(
                colorSpaceId.first,
                colorSpaceId.second,
                iccProfileData->icc);
            dbgFile << "Loaded ICC profile from PSD" << profile->name();
            delete resources.take(KisTiffPsdResourceRecord::ICC_PROFILE);
        }

        if (profile) {
            const KoColorSpace *tempCs =
                KoColorSpaceRegistry::instance()->colorSpace(
                    colorSpaceId.first,
                    colorSpaceId.second,
                    profile);
            if (tempCs) {
                // Profile found, override the colorspace
                dbgFile << "TIFF: PSD metadata overrides the color space!"
                        << cs->name() << cs->profile()->name();
                cs = tempCs;
            }
        }
    }

    KisImageSP psdImage = new KisImage(m_doc->createUndoStore(),
                                       static_cast<qint32>(basicInfo.width),
                                       static_cast<qint32>(basicInfo.height),
                                       cs,
                                       "built image");
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(psdImage,
                                         ImportExportCodes::InsufficientMemory);

    psdImage->setResolution(
        POINT_TO_INCH(static_cast<qreal>(basicInfo.xres)),
        POINT_TO_INCH(static_cast<qreal>(
            basicInfo.yres))); // It is the "invert" macro because we convert
                               // from pointer-per-inch to points

    // set the correct resolution
    if (resources.contains(KisTiffPsdResourceRecord::RESN_INFO)) {
        RESN_INFO_1005 *resInfo = dynamic_cast<RESN_INFO_1005 *>(
            resources[KisTiffPsdResourceRecord::RESN_INFO]->resource);
        if (resInfo) {
            // check resolution size is not zero
            if (resInfo->hRes * resInfo->vRes > 0)
                psdImage->setResolution(POINT_TO_INCH(resInfo->hRes),
                                        POINT_TO_INCH(resInfo->vRes));
            // let's skip the unit for now; we can only set that on the
            // KisDocument, and krita doesn't use it.
            delete resources.take(KisTiffPsdResourceRecord::RESN_INFO);
        }
    }

    // Preserve all the annotations
    for (const auto &resourceBlock : resources.values()) {
        psdImage->addAnnotation(resourceBlock);
    }

    dbgFile << "Loading Photoshop layers";

    QStack<KisGroupLayerSP> groupStack;

    groupStack << psdImage->rootLayer().data();

    /**
     * PSD has a weird "optimization": if a group layer has only one
     * child layer, it omits it's 'psd_bounding_divider' section. So
     * fi you ever see an unbalanced layers group in PSD, most
     * probably, it is just a single layered group.
     */
    KisNodeSP lastAddedLayer;

    using LayerStyleMapping = QPair<QDomDocument, KisLayerSP>;
    QVector<LayerStyleMapping> allStylesXml;

    const std::shared_ptr<PSDLayerMaskSection> &layerSection =
        photoshopLayerRecord.record();

    KIS_SAFE_ASSERT_RECOVER(layerSection->nLayers != 0)
    {
        return ImportExportCodes::FileFormatIncorrect;
    }

    for (int i = 0; i != layerSection->nLayers; i++) {
        PSDLayerRecord *layerRecord = layerSection->layers.at(i);
        dbgFile << "Going to read channels for layer" << i
                << layerRecord->layerName;
        KisLayerSP newLayer;
        if (layerRecord->infoBlocks.keys.contains("lsct")
            && layerRecord->infoBlocks.sectionDividerType != psd_other) {
            if (layerRecord->infoBlocks.sectionDividerType
                    == psd_bounding_divider
                && !groupStack.isEmpty()) {
                KisGroupLayerSP groupLayer =
                    new KisGroupLayer(psdImage, "temp", OPACITY_OPAQUE_U8);
                psdImage->addNode(groupLayer, groupStack.top());
                groupStack.push(groupLayer);
                newLayer = groupLayer;
            } else if ((layerRecord->infoBlocks.sectionDividerType
                            == psd_open_folder
                        || layerRecord->infoBlocks.sectionDividerType
                            == psd_closed_folder)
                       && (groupStack.size() > 1
                           || (lastAddedLayer && !groupStack.isEmpty()))) {
                KisGroupLayerSP groupLayer;

                if (groupStack.size() <= 1) {
                    groupLayer =
                        new KisGroupLayer(psdImage, "temp", OPACITY_OPAQUE_U8);
                    psdImage->addNode(groupLayer, groupStack.top());
                    psdImage->moveNode(lastAddedLayer, groupLayer, KisNodeSP());
                } else {
                    groupLayer = groupStack.pop();
                }

                const QDomDocument &styleXml =
                    layerRecord->infoBlocks.layerStyleXml;

                if (!styleXml.isNull()) {
                    allStylesXml << LayerStyleMapping(styleXml, groupLayer);
                }

                groupLayer->setName(layerRecord->layerName);
                groupLayer->setVisible(layerRecord->visible);

                QString compositeOp = psd_blendmode_to_composite_op(
                    layerRecord->infoBlocks.sectionDividerBlendMode);

                // Krita doesn't support pass-through blend
                // mode. Instead it is just a property of a group
                // layer, so flip it
                if (compositeOp == COMPOSITE_PASS_THROUGH) {
                    compositeOp = COMPOSITE_OVER;
                    groupLayer->setPassThroughMode(true);
                }

                groupLayer->setCompositeOpId(compositeOp);

                newLayer = groupLayer;
            } else {
                /**
                 * In some files saved by PS CS6 the group layer sections seem
                 * to be unbalanced.  I don't know why it happens because the
                 * reporter didn't provide us an example file. So here we just
                 * check if the new layer was created, and if not, skip the
                 * initialization of masks.
                 *
                 * See bug: 357559
                 */

                warnKrita << "WARNING: Provided PSD has unbalanced group "
                          << "layer markers. Some masks and/or layers can "
                          << "be lost while loading this file. Please "
                          << "report a bug to Krita developers and attach "
                          << "this file to the bugreport\n"
                          << "    " << ppVar(layerRecord->layerName) << "\n"
                          << "    "
                          << ppVar(layerRecord->infoBlocks.sectionDividerType)
                          << "\n"
                          << "    " << ppVar(groupStack.size());
                continue;
            }
        } else {
            KisPaintLayerSP layer = new KisPaintLayer(psdImage,
                                                      layerRecord->layerName,
                                                      layerRecord->opacity);
            layer->setCompositeOpId(
                psd_blendmode_to_composite_op(layerRecord->blendModeKey));

            const QDomDocument &styleXml =
                layerRecord->infoBlocks.layerStyleXml;

            if (!styleXml.isNull()) {
                allStylesXml << LayerStyleMapping(styleXml, layer);
            }

            // XXX: does this require endianness handling?
            if (!layerRecord->readPixelData(photoshopLayerData,
                                            layer->paintDevice())) {
                dbgFile << "failed reading channels for layer: "
                        << layerRecord->layerName << layerRecord->error;
                return ImportExportCodes::FileFormatIncorrect;
            }

            if (!groupStack.isEmpty()) {
                psdImage->addNode(layer, groupStack.top());
            } else {
                psdImage->addNode(layer, psdImage->root());
            }
            layer->setVisible(layerRecord->visible);
            newLayer = layer;
        }

        for (ChannelInfo *channelInfo : layerRecord->channelInfoRecords) {
            if (channelInfo->channelId < -1) {
                KisTransparencyMaskSP mask =
                    new KisTransparencyMask(psdImage,
                                            i18n("Transparency Mask"));
                mask->initSelection(newLayer);
                if (!layerRecord->readMask(photoshopLayerData,
                                           mask->paintDevice(),
                                           channelInfo)) {
                    dbgFile << "failed reading masks for layer: "
                            << layerRecord->layerName << layerRecord->error;
                }
                psdImage->addNode(mask, newLayer);
            }
        }

        lastAddedLayer = newLayer;
    }

    // Only assign the image if the parsing was successful (for fallback
    // purposes)
    this->m_image = psdImage;
    // Photoshop images only have one IFD plus the layer blob
    // Ward off inconsistencies by blocking future attempts to parse them
    this->m_photoshopBlockParsed = true;

    return ImportExportCodes::OK;
}
#endif

KisImportExportErrorCode
KisTIFFImport::readImageFromTiff(KisDocument *m_doc,
                                 TIFF *image,
                                 KisTiffBasicInfo &basicInfo)
{
    uint32_t &width = basicInfo.width;
    uint32_t &height = basicInfo.height;
    float &xres = basicInfo.xres;
    float &yres = basicInfo.yres;
    uint16_t &depth = basicInfo.depth;
    uint16_t &sampletype = basicInfo.sampletype;
    uint16_t &nbchannels = basicInfo.nbchannels;
    uint16_t &color_type = basicInfo.color_type;
    uint16_t *&sampleinfo = basicInfo.sampleinfo;
    uint16_t &extrasamplescount = basicInfo.extrasamplescount;
    const KoColorSpace *&cs = basicInfo.cs;
    QPair<QString, QString> &colorSpaceIdTag = basicInfo.colorSpaceIdTag;
    KoColorTransformation *&transform = basicInfo.transform;
    uint8_t &dstDepth = basicInfo.dstDepth;

    // Check if there is an alpha channel
    int32_t alphapos = -1; // <- no alpha
    bool hasPremultipliedAlpha = false;
    // Check which extra is alpha if any
    dbgFile << "There are" << nbchannels << " channels and" << extrasamplescount
            << " extra channels";
    if (sampleinfo) { // index images don't have any sampleinfo, and therefore
                      // sampleinfo == 0
        for (uint16_t i = 0; i < extrasamplescount; i++) {
            dbgFile << "sample" << i << "extra sample count"
                    << extrasamplescount << "color channel count"
                    << (cs->colorChannelCount()) << "Number of channels"
                    << nbchannels << "sample info" << sampleinfo[i];
            switch (sampleinfo[i]) {
            case EXTRASAMPLE_ASSOCALPHA:
                // The color values are already multiplied with the alpha value.
                // This is reversed in the postprocessor.
                dbgPlugins << "Detected associated alpha @ " << i;
                hasPremultipliedAlpha = true;
                alphapos = static_cast<int32_t>(extrasamplescount
                                                - 1U); // nbsamples - 1
                break;
            case EXTRASAMPLE_UNASSALPHA:
                // color values are not premultiplied with alpha, and can be
                // used as they are.
                alphapos = i;
                break;
            case EXTRASAMPLE_UNSPECIFIED:
            default:
                qWarning() << "Extra sample type not defined for this file, "
                              "assuming unassociated alpha.";
                alphapos = i;
                break;
            }

            if (sampleinfo[i] == EXTRASAMPLE_UNASSALPHA) {
                // color values are not premultiplied with alpha, and can be
                // used as they are.
                alphapos = i;
            }
        }
    }

    dbgFile << "Alpha pos:" << alphapos;

    // Read META Information
    KoDocumentInfo *info = m_doc->documentInfo();
    char *text = nullptr;
    if (TIFFGetField(image, TIFFTAG_ARTIST, &text) == 1) {
        info->setAuthorInfo("creator", text);
    }
    if (TIFFGetField(image, TIFFTAG_DOCUMENTNAME, &text) == 1) {
        info->setAboutInfo("title", text);
    }
    if (TIFFGetField(image, TIFFTAG_IMAGEDESCRIPTION, &text) == 1) {
        info->setAboutInfo("description", text);
    }

    uint16_t orientation = ORIENTATION_TOPLEFT;
    if (TIFFGetField(image, TIFFTAG_ORIENTATION, &orientation) == 0) {
        dbgFile << "Orientation not defined, assuming top left";
    }

    dbgFile << "Orientation:" << orientation;

    // Try to get IPTC metadata
    uint32_t iptc_profile_size = 0;
    uint32_t *iptc_profile_data = nullptr;
    if (TIFFGetField(image,
                     TIFFTAG_RICHTIFFIPTC,
                     &iptc_profile_size,
                     &iptc_profile_data)
        == 0) {
        dbgFile << "IPTC metadata not found!";
    }

    // Try to get XMP metadata
    uint32_t xmp_size = 0;
    uint8_t *xmp_data = nullptr;
    if (TIFFGetField(image, TIFFTAG_XMLPACKET, &xmp_size, &xmp_data) == 0) {
        dbgFile << "XML metadata not found!";
    }

    // Get the planar configuration
    uint16_t planarconfig = PLANARCONFIG_CONTIG;
    if (TIFFGetField(image, TIFFTAG_PLANARCONFIG, &planarconfig) == 0) {
        dbgFile << "Planar configuration is not defined";
        return ImportExportCodes::FileFormatIncorrect;
    }
    // Creating the KisImageSP
    if (!m_image) {
        m_image = new KisImage(m_doc->createUndoStore(),
                               static_cast<qint32>(width),
                               static_cast<qint32>(height),
                               cs,
                               "built image");
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(
            m_image,
            ImportExportCodes::InsufficientMemory);
        // It is the "invert" macro because we
        // convert from pointer-per-unit to points
        if (basicInfo.resolution == TiffResolution::INCH) {
            m_image->setResolution(POINT_TO_INCH(static_cast<qreal>(xres)), POINT_TO_INCH(static_cast<qreal>(yres)));
        } else {
            m_image->setResolution(POINT_TO_CM(static_cast<qreal>(xres)), POINT_TO_CM(static_cast<qreal>(yres)));
        }
    } else {
        if (m_image->width() < static_cast<qint32>(width)
            || m_image->height() < static_cast<qint32>(height)) {
            qint32 newwidth = (m_image->width() < static_cast<qint32>(width))
                ? static_cast<qint32>(width)
                : m_image->width();
            qint32 newheight = (m_image->height() < static_cast<qint32>(height))
                ? static_cast<qint32>(height)
                : m_image->height();
            m_image->resizeImage(QRect(0, 0, newwidth, newheight));
        }
    }
    KisPaintLayer *layer =
        new KisPaintLayer(m_image, m_image->nextLayerName(), quint8_MAX, cs);
    std::unique_ptr<std::remove_pointer_t<tdata_t>, decltype(&_TIFFfree)> buf(
        nullptr,
        &_TIFFfree);
    // used only for planar configuration separated
    auto ps_buf = make_unique_with_deleter(new QVector<uint8_t *>(),
                                           [](QVector<uint8_t *> *buf) {
                                               for (uint8_t *p : *buf)
                                                   _TIFFfree(p);
                                               delete buf;
                                           });

    QSharedPointer<KisBufferStreamBase> tiffstream = nullptr;
    QSharedPointer<KisTIFFReaderBase> tiffReader = nullptr;

    // Configure poses
    uint16_t nbcolorsamples = nbchannels - extrasamplescount;
    const auto poses = [&]() -> std::array<quint8, 5> {
        switch (color_type) {
        case PHOTOMETRIC_MINISWHITE:
        case PHOTOMETRIC_MINISBLACK:
            return {0, 1};
        case PHOTOMETRIC_CIELAB:
        case PHOTOMETRIC_ICCLAB:
            return {0, 1, 2, 3};
        case PHOTOMETRIC_RGB:
            if (sampletype == SAMPLEFORMAT_IEEEFP) {
                return {0, 1, 2, 3};
            } else {
                return {2, 1, 0, 3};
            }
        case PHOTOMETRIC_SEPARATED:
            return {0, 1, 2, 3, 4};
        default:
            return {};
        }
    }();

    auto postprocessor = [&]() -> QSharedPointer<KisTIFFPostProcessor> {
        switch (color_type) {
        case PHOTOMETRIC_MINISWHITE:
            return makePostProcessor<KisTIFFPostProcessorInvert>(
                nbcolorsamples,
                colorSpaceIdTag);
        case PHOTOMETRIC_MINISBLACK:
            return makePostProcessor<KisTIFFPostProcessorDummy>(
                nbcolorsamples,
                colorSpaceIdTag);
        case PHOTOMETRIC_CIELAB:
            return makePostProcessor<KisTIFFPostProcessorCIELABtoICCLAB>(
                nbcolorsamples,
                colorSpaceIdTag);
        case PHOTOMETRIC_ICCLAB:
        case PHOTOMETRIC_RGB:
        case PHOTOMETRIC_SEPARATED:
            return makePostProcessor<KisTIFFPostProcessorDummy>(
                nbcolorsamples,
                colorSpaceIdTag);
        default:
            return {};
        }
    }();

    // Initialize tiffReader
    QVector<uint16_t> lineSizeCoeffs(nbchannels, 1);
    uint16_t vsubsampling = 1;
    uint16_t hsubsampling = 1;
    if (color_type == PHOTOMETRIC_PALETTE) {
        uint16_t *red =
            nullptr; // No need to free them they are free by libtiff
        uint16_t *green = nullptr;
        uint16_t *blue = nullptr;
        if ((TIFFGetField(image, TIFFTAG_COLORMAP, &red, &green, &blue)) == 0) {
            dbgFile << "Indexed image does not define a palette";
            return ImportExportCodes::FileFormatIncorrect;
        }

        tiffReader =
            QSharedPointer<KisTIFFReaderFromPalette>::create(layer->paintDevice(),
                                                       red,
                                                       green,
                                                       blue,
                                                       poses,
                                                       alphapos,
                                                       depth,
                                                       sampletype,
                                                       nbcolorsamples,
                                                       extrasamplescount,
                                                       hasPremultipliedAlpha,
                                                       transform,
                                                       postprocessor);
    } else if (color_type == PHOTOMETRIC_YCBCR) {
        TIFFGetFieldDefaulted(image,
                              TIFFTAG_YCBCRSUBSAMPLING,
                              &hsubsampling,
                              &vsubsampling);
        lineSizeCoeffs[1] = hsubsampling;
        lineSizeCoeffs[2] = hsubsampling;
        dbgFile << "Subsampling" << 4 << hsubsampling << vsubsampling;
        if (dstDepth == 8) {
            tiffReader = QSharedPointer<KisTIFFYCbCrReader<uint8_t>>::create(
                layer->paintDevice(),
                static_cast<quint32>(layer->image()->width()),
                static_cast<quint32>(layer->image()->height()),
                poses,
                alphapos,
                depth,
                sampletype,
                nbcolorsamples,
                extrasamplescount,
                hasPremultipliedAlpha,
                transform,
                postprocessor,
                hsubsampling,
                vsubsampling);
        } else if (dstDepth == 16) {
            if (sampletype == SAMPLEFORMAT_IEEEFP) {
#ifdef HAVE_OPENEXR
                tiffReader = QSharedPointer<KisTIFFYCbCrReader<half>>::create(
                    layer->paintDevice(),
                    static_cast<quint32>(layer->image()->width()),
                    static_cast<quint32>(layer->image()->height()),
                    poses,
                    alphapos,
                    depth,
                    sampletype,
                    nbcolorsamples,
                    extrasamplescount,
                    hasPremultipliedAlpha,
                    transform,
                    postprocessor,
                    hsubsampling,
                    vsubsampling);
#endif
            } else {
                tiffReader =
                    QSharedPointer<KisTIFFYCbCrReader<uint16_t>>::create(
                    layer->paintDevice(),
                    static_cast<quint32>(layer->image()->width()),
                    static_cast<quint32>(layer->image()->height()),
                    poses,
                    alphapos,
                    depth,
                    sampletype,
                    nbcolorsamples,
                    extrasamplescount,
                    hasPremultipliedAlpha,
                    transform,
                    postprocessor,
                    hsubsampling,
                    vsubsampling);
            }
        } else if (dstDepth == 32) {
            if (sampletype == SAMPLEFORMAT_IEEEFP) {
                tiffReader = QSharedPointer<KisTIFFYCbCrReader<float>>::create(
                    layer->paintDevice(),
                    static_cast<quint32>(layer->image()->width()),
                    static_cast<quint32>(layer->image()->height()),
                    poses,
                    alphapos,
                    depth,
                    sampletype,
                    nbcolorsamples,
                    extrasamplescount,
                    hasPremultipliedAlpha,
                    transform,
                    postprocessor,
                    hsubsampling,
                    vsubsampling);
            } else {
                tiffReader =
                    QSharedPointer<KisTIFFYCbCrReader<uint32_t>>::create(
                    layer->paintDevice(),
                    static_cast<quint32>(layer->image()->width()),
                    static_cast<quint32>(layer->image()->height()),
                    poses,
                    alphapos,
                    depth,
                    sampletype,
                    nbcolorsamples,
                    extrasamplescount,
                    hasPremultipliedAlpha,
                    transform,
                    postprocessor,
                    hsubsampling,
                    vsubsampling);
            }
        }
    } else if (dstDepth == 8) {
        tiffReader = QSharedPointer<KisTIFFReaderTarget<uint8_t>>::create(
            layer->paintDevice(),
            poses,
            alphapos,
            depth,
            sampletype,
            nbcolorsamples,
            extrasamplescount,
            hasPremultipliedAlpha,
            transform,
            postprocessor,
            quint8_MAX);
    } else if (dstDepth == 16) {
        if (sampletype == SAMPLEFORMAT_IEEEFP) {
#ifdef HAVE_OPENEXR
            tiffReader = QSharedPointer<KisTIFFReaderTarget<half>>::create(
                layer->paintDevice(),
                poses,
                alphapos,
                depth,
                sampletype,
                nbcolorsamples,
                extrasamplescount,
                hasPremultipliedAlpha,
                transform,
                postprocessor,
                1.0);
#endif
        } else {
            tiffReader = QSharedPointer<KisTIFFReaderTarget<uint16_t>>::create(
                layer->paintDevice(),
                poses,
                alphapos,
                depth,
                sampletype,
                nbcolorsamples,
                extrasamplescount,
                hasPremultipliedAlpha,
                transform,
                postprocessor,
                quint16_MAX);
        }
    } else if (dstDepth == 32) {
        if (sampletype == SAMPLEFORMAT_IEEEFP) {
            tiffReader = QSharedPointer<KisTIFFReaderTarget<float>>::create(
                layer->paintDevice(),
                poses,
                alphapos,
                depth,
                sampletype,
                nbcolorsamples,
                extrasamplescount,
                hasPremultipliedAlpha,
                transform,
                postprocessor,
                1.0f);
        } else {
            tiffReader = QSharedPointer<KisTIFFReaderTarget<uint32_t>>::create(
                layer->paintDevice(),
                poses,
                alphapos,
                depth,
                sampletype,
                nbcolorsamples,
                extrasamplescount,
                hasPremultipliedAlpha,
                transform,
                postprocessor,
                std::numeric_limits<uint32_t>::max());
        }
    }

    if (!tiffReader) {
        dbgFile << "Image has an invalid/unsupported color type: "
                << color_type;
        return ImportExportCodes::FileFormatIncorrect;
    }

    uint32_t compression = COMPRESSION_NONE;
    TIFFGetFieldDefaulted(image, TIFFTAG_COMPRESSION, &compression, COMPRESSION_NONE);

#ifdef HAVE_JPEG_TURBO
    uint32_t hasSplitTables = 0;
    uint8_t *tables = nullptr;
    uint32_t sz = 0;
    QVector<unsigned char> jpegBuf;

    auto handle = [&]() -> std::unique_ptr<void, decltype(&tjDestroy)> {
        if (planarconfig == PLANARCONFIG_CONTIG
            && color_type == PHOTOMETRIC_YCBCR
            && compression == COMPRESSION_JPEG) {
            return {tjInitDecompress(), &tjDestroy};
        } else {
            return {nullptr, &tjDestroy};
        }
    }();

    if (color_type == PHOTOMETRIC_YCBCR && compression == COMPRESSION_JPEG
        && hsubsampling != 1 && vsubsampling != 1) {
        dbgFile << "Setting up libjpeg-turbo for handling subsampled JPEG...";
        if (!TIFFGetFieldDefaulted(image,
                                   TIFFTAG_JPEGTABLESMODE,
                                   &hasSplitTables)) {
            errFile << "Error when detecting the JPEG coefficient "
                       "table mode";
            return ImportExportCodes::FileFormatIncorrect;
        }
        if (hasSplitTables) {
            if (!TIFFGetField(image, TIFFTAG_JPEGTABLES, &sz, &tables)) {
                errFile << "Unable to retrieve the JPEG abbreviated datastream";
                return ImportExportCodes::FileFormatIncorrect;
            }
        }

        {
            int width = 0;
            int height = 0;

            if (hasSplitTables
                && tjDecompressHeader(handle.get(), tables, sz, &width, &height)
                    != 0) {
                errFile << tjGetErrorStr2(handle.get());
                m_doc->setErrorMessage(
                    i18nc("TIFF errors",
                          "This TIFF file is compressed with JPEG, but "
                          "libjpeg-turbo could not load its coefficient "
                          "quantization and/or Huffman coding tables. "
                          "Please upgrade your version of libjpeg-turbo "
                          "and try again."));
                return ImportExportCodes::FileFormatIncorrect;
            }
        }
    }
#endif

    if (TIFFIsTiled(image)) {
        dbgFile << "tiled image";
        uint32_t tileWidth = 0;
        uint32_t tileHeight = 0;
        uint32_t x = 0;
        uint32_t y = 0;
        TIFFGetField(image, TIFFTAG_TILEWIDTH, &tileWidth);
        TIFFGetField(image, TIFFTAG_TILELENGTH, &tileHeight);
        tmsize_t tileSize = TIFFTileSize(image);

        if (planarconfig == PLANARCONFIG_CONTIG
            && !(color_type == PHOTOMETRIC_YCBCR
                 && compression == COMPRESSION_JPEG && hsubsampling != 1
                 && vsubsampling != 1)) {
            buf.reset(_TIFFmalloc(tileSize));
            if (depth < 16) {
                tiffstream =
                    QSharedPointer<KisBufferStreamContigBelow16>::create(
                        static_cast<uint8_t *>(buf.get()),
                        depth,
                        tileSize / tileHeight);
            } else if (depth >= 16 && depth < 32) {
                tiffstream =
                    QSharedPointer<KisBufferStreamContigBelow32>::create(
                        static_cast<uint8_t *>(buf.get()),
                        depth,
                        tileSize / tileHeight);
            } else {
                tiffstream =
                    QSharedPointer<KisBufferStreamContigAbove32>::create(
                        static_cast<uint8_t *>(buf.get()),
                        depth,
                        tileSize / tileHeight);
            }
        } else if (planarconfig == PLANARCONFIG_CONTIG
                   && color_type == PHOTOMETRIC_YCBCR
                   && compression == COMPRESSION_JPEG) {
#ifdef HAVE_JPEG_TURBO
            jpegBuf.resize(tileSize);
            ps_buf->resize(nbchannels);
            TIFFReadRawTile(image, 0, jpegBuf.data(), tileSize);

            int width = tileWidth;
            int height = tileHeight;
            int jpegSubsamp = TJ_444;
            int jpegColorspace = TJCS_YCbCr;

            if (tjDecompressHeader3(handle.get(),
                                    jpegBuf.data(),
                                    tileSize,
                                    &width,
                                    &height,
                                    &jpegSubsamp,
                                    &jpegColorspace)
                != 0) {
                errFile << tjGetErrorStr2(handle.get());
                return ImportExportCodes::FileFormatIncorrect;
            }

            QVector<tsize_t> lineSizes(nbchannels);
            for (uint32_t i = 0; i < nbchannels; i++) {
                const unsigned long uncompressedTileSize =
                    tjPlaneSizeYUV(i, width, 0, height, jpegSubsamp);
                KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(
                    uncompressedTileSize != (unsigned long)-1,
                    ImportExportCodes::FileFormatIncorrect);
                dbgFile << QString("Uncompressed tile size (plane %1): %2")
                               .arg(i)
                               .arg(uncompressedTileSize)
                               .toStdString()
                               .c_str();
                tsize_t scanLineSize = uncompressedTileSize / tileHeight;
                dbgFile << QString("scan line size (plane %1): %2")
                               .arg(i)
                               .arg(scanLineSize)
                               .toStdString()
                               .c_str();
                (*ps_buf)[i] =
                    static_cast<uint8_t *>(_TIFFmalloc(uncompressedTileSize));
                lineSizes[i] = scanLineSize;
            }
            tiffstream =
                QSharedPointer<KisBufferStreamInterleaveUpsample>::create(
                    ps_buf->data(),
                    nbchannels,
                    depth,
                    lineSizes.data(),
                    hsubsampling,
                    vsubsampling);
#else
            m_doc->setErrorMessage(
                i18nc("TIFF",
                      "Subsampled YCbCr TIFF files compressed with JPEG cannot "
                      "be loaded."));
            return ImportExportCodes::FileFormatIncorrect;
#endif
        } else {
            ps_buf->resize(nbchannels);
            tsize_t scanLineSize = tileSize / tileHeight;
            dbgFile << " scanLineSize for each plan =" << scanLineSize;
            QVector<tsize_t> lineSizes(nbchannels);
            for (uint32_t i = 0; i < nbchannels; i++) {
                (*ps_buf)[i] = static_cast<uint8_t *>(_TIFFmalloc(tileSize));
                lineSizes[i] = scanLineSize / lineSizeCoeffs[i];
            }
            tiffstream = QSharedPointer<KisBufferStreamSeparate>::create(
                ps_buf->data(),
                nbchannels,
                depth,
                lineSizes.data());
        }
        dbgFile << "Scanline size =" << TIFFRasterScanlineSize(image)
                << " / tile size =" << TIFFTileSize(image)
                << " / tile width =" << tileWidth
                << " tileSize/tileHeight =" << tileSize / tileHeight;

        dbgFile << " NbOfTiles =" << TIFFNumberOfTiles(image)
                << " tileWidth =" << tileWidth << " tileSize =" << tileSize;

        for (y = 0; y < height; y += tileHeight) {
            for (x = 0; x < width; x += tileWidth) {
                dbgFile << "Reading tile x =" << x << " y =" << y;
#ifdef HAVE_JPEG_TURBO
                if (planarconfig == PLANARCONFIG_CONTIG
                    && !(color_type == PHOTOMETRIC_YCBCR
                         && compression == COMPRESSION_JPEG && hsubsampling != 1
                         && vsubsampling != 1)) {
#else
                if (planarconfig == PLANARCONFIG_CONTIG) {
#endif
                    TIFFReadTile(image, buf.get(), x, y, 0, (tsample_t)-1);
#ifdef HAVE_JPEG_TURBO
                } else if (planarconfig == PLANARCONFIG_CONTIG
                           && (color_type == PHOTOMETRIC_YCBCR
                               && compression == COMPRESSION_JPEG)) {
                    uint32_t tile =
                        TIFFComputeTile(image, x, y, 0, (tsample_t)-1);
                    TIFFReadRawTile(image, tile, jpegBuf.data(), tileSize);

                    int width = tileWidth;
                    int height = tileHeight;
                    int jpegSubsamp = TJ_444;
                    int jpegColorspace = TJCS_YCbCr;

                    if (tjDecompressHeader3(handle.get(),
                                            jpegBuf.data(),
                                            tileSize,
                                            &width,
                                            &height,
                                            &jpegSubsamp,
                                            &jpegColorspace)
                        != 0) {
                        errFile << tjGetErrorStr2(handle.get());
                        return ImportExportCodes::FileFormatIncorrect;
                    }

                    if (tjDecompressToYUVPlanes(handle.get(),
                                                jpegBuf.data(),
                                                tileSize,
                                                ps_buf->data(),
                                                width,
                                                nullptr,
                                                height,
                                                0)
                        != 0) {
                        errFile << tjGetErrorStr2(handle.get());
                        return ImportExportCodes::FileFormatIncorrect;
                    }
#endif
                } else {
                    for (uint16_t i = 0; i < nbchannels; i++) {
                        TIFFReadTile(image, (*ps_buf)[i], x, y, 0, i);
                    }
                }
                uint32_t realTileWidth =
                    (x + tileWidth) < width ? tileWidth : width - x;
                for (uint32_t yintile = 0;
                     yintile < tileHeight && y + yintile < height;) {
                    uint32_t linesread =
                        tiffReader->copyDataToChannels(x,
                                                       y + yintile,
                                                       realTileWidth,
                                                       tiffstream);
                    yintile += linesread;
                    tiffstream->moveToLine(yintile);
                }
                tiffstream->restart();
            }
        }
    } else {
        dbgFile << "striped image";
        tsize_t stripsize = TIFFStripSize(image);
        uint32_t rowsPerStrip = 0;
        TIFFGetFieldDefaulted(image, TIFFTAG_ROWSPERSTRIP, &rowsPerStrip);
        dbgFile << rowsPerStrip << "" << height;
        rowsPerStrip =
            qMin(rowsPerStrip,
                 height); // when TIFFNumberOfStrips(image) == 1 it might happen
                          // that rowsPerStrip is incorrectly set
        if (planarconfig == PLANARCONFIG_CONTIG
            && !(color_type == PHOTOMETRIC_YCBCR
                 && compression == COMPRESSION_JPEG && hsubsampling != 1
                 && vsubsampling != 1)) {
            buf.reset(_TIFFmalloc(stripsize));
            if (depth < 16) {
                tiffstream =
                    QSharedPointer<KisBufferStreamContigBelow16>::create(
                    static_cast<uint8_t *>(buf.get()),
                    depth,
                    stripsize / rowsPerStrip);
            } else if (depth < 32) {
                tiffstream =
                    QSharedPointer<KisBufferStreamContigBelow32>::create(
                    static_cast<uint8_t *>(buf.get()),
                    depth,
                    stripsize / rowsPerStrip);
            } else {
                tiffstream =
                    QSharedPointer<KisBufferStreamContigAbove32>::create(
                    static_cast<uint8_t *>(buf.get()),
                    depth,
                    stripsize / rowsPerStrip);
            }
        } else if (planarconfig == PLANARCONFIG_CONTIG
                   && color_type == PHOTOMETRIC_YCBCR
                   && compression == COMPRESSION_JPEG) {
#ifdef HAVE_JPEG_TURBO
            jpegBuf.resize(stripsize);
            ps_buf->resize(nbchannels);
            TIFFReadRawStrip(image, 0, jpegBuf.data(), stripsize);

            int width = basicInfo.width;
            int height = rowsPerStrip;
            int jpegSubsamp = TJ_444;
            int jpegColorspace = TJCS_YCbCr;

            if (tjDecompressHeader3(handle.get(),
                                    jpegBuf.data(),
                                    stripsize,
                                    &width,
                                    &height,
                                    &jpegSubsamp,
                                    &jpegColorspace)
                != 0) {
                errFile << tjGetErrorStr2(handle.get());
                return ImportExportCodes::FileFormatIncorrect;
            }

            QVector<tsize_t> lineSizes(nbchannels);
            for (uint32_t i = 0; i < nbchannels; i++) {
                const unsigned long uncompressedStripsize =
                    tjPlaneSizeYUV(i, width, 0, height, jpegSubsamp);
                KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(
                    uncompressedStripsize != (unsigned long)-1,
                    ImportExportCodes::FileFormatIncorrect);
                dbgFile << QString("Uncompressed strip size (plane %1): %2")
                               .arg(i)
                               .arg(uncompressedStripsize);
                tsize_t scanLineSize = uncompressedStripsize / rowsPerStrip;
                dbgFile << QString("scan line size (plane %1): %2")
                               .arg(i)
                               .arg(scanLineSize);
                (*ps_buf)[i] = static_cast<uint8_t*>(_TIFFmalloc(uncompressedStripsize));
                lineSizes[i] = scanLineSize;
            }
            tiffstream = QSharedPointer<KisBufferStreamInterleaveUpsample>::create(
                ps_buf->data(),
                nbchannels,
                depth,
                lineSizes.data(),
                hsubsampling,
                vsubsampling);
#else
            m_doc->setErrorMessage(
                i18nc("TIFF",
                      "Subsampled YCbCr TIFF files compressed with JPEG cannot "
                      "be loaded."));
            return ImportExportCodes::FileFormatIncorrect;
#endif
        } else {
            ps_buf->resize(nbchannels);
            tsize_t scanLineSize = stripsize / rowsPerStrip;
            dbgFile << " scanLineSize for each plan =" << scanLineSize;
            QVector<tsize_t> lineSizes(nbchannels);
            for (uint32_t i = 0; i < nbchannels; i++) {
                (*ps_buf)[i] = static_cast<uint8_t*>(_TIFFmalloc(stripsize));
                lineSizes[i] = scanLineSize / lineSizeCoeffs[i];
            }
            tiffstream = QSharedPointer<KisBufferStreamSeparate>::create(
                ps_buf->data(),
                nbchannels,
                depth,
                lineSizes.data());
        }

        dbgFile << "Scanline size =" << TIFFRasterScanlineSize(image)
                << " / strip size =" << TIFFStripSize(image)
                << " / rowsPerStrip =" << rowsPerStrip
                << " stripsize/rowsPerStrip =" << stripsize / rowsPerStrip;
        uint32_t y = 0;
        dbgFile << " NbOfStrips =" << TIFFNumberOfStrips(image)
                << " rowsPerStrip =" << rowsPerStrip
                << " stripsize =" << stripsize;

        for (uint32_t strip = 0; y < height; strip++) {
#ifdef HAVE_JPEG_TURBO
            if (planarconfig == PLANARCONFIG_CONTIG
                && !(color_type == PHOTOMETRIC_YCBCR
                     && compression == COMPRESSION_JPEG && hsubsampling != 1
                     && vsubsampling != 1)) {
#else
            if (planarconfig == PLANARCONFIG_CONTIG) {
#endif
                TIFFReadEncodedStrip(image,
                                     TIFFComputeStrip(image, y, 0),
                                     buf.get(),
                                     (tsize_t)-1);
#ifdef HAVE_JPEG_TURBO
            } else if (planarconfig == PLANARCONFIG_CONTIG
                       && (color_type == PHOTOMETRIC_YCBCR
                           && compression == COMPRESSION_JPEG)) {
                TIFFReadRawStrip(image, strip, jpegBuf.data(), stripsize);

                int width = basicInfo.width;
                int height = rowsPerStrip;
                int jpegSubsamp = TJ_444;
                int jpegColorspace = TJCS_YCbCr;

                if (tjDecompressHeader3(handle.get(),
                                        jpegBuf.data(),
                                        stripsize,
                                        &width,
                                        &height,
                                        &jpegSubsamp,
                                        &jpegColorspace)
                    != 0) {
                    errFile << tjGetErrorStr2(handle.get());
                    return ImportExportCodes::FileFormatIncorrect;
                }

                if (tjDecompressToYUVPlanes(
                        handle.get(),
                        jpegBuf.data(),
                        stripsize,
                        ps_buf->data(),
                        width,
                        nullptr,
                        height,
                        0)
                    != 0) {
                    errFile << tjGetErrorStr2(handle.get());
                    return ImportExportCodes::FileFormatIncorrect;
                }
#endif
            } else {
                for (uint16_t i = 0; i < nbchannels; i++) {
                    TIFFReadEncodedStrip(image,
                                         TIFFComputeStrip(image, y, i),
                                         (*ps_buf)[i],
                                         (tsize_t)-1);
                }
            }
            for (uint32_t yinstrip = 0;
                 yinstrip < rowsPerStrip && y < height;) {
                uint32_t linesread =
                    tiffReader->copyDataToChannels(0, y, width, tiffstream);
                y += linesread;
                yinstrip += linesread;
                tiffstream->moveToLine(yinstrip);
            }
            tiffstream->restart();
        }
    }
    tiffReader->finalize();
    tiffReader.reset();
    tiffstream.reset();
    ps_buf.reset();

    m_image->addNode(KisNodeSP(layer), m_image->rootLayer().data());

    layer->paintDevice()->setX(static_cast<int>(basicInfo.x * basicInfo.xres));
    layer->paintDevice()->setY(static_cast<int>(basicInfo.y * basicInfo.yres));

    // Process rotation before handing image over
    // https://developer.apple.com/documentation/imageio/cgimagepropertyorientation
    switch (orientation) {
    case ORIENTATION_TOPRIGHT:
        KisTransformWorker::mirrorX(layer->paintDevice());
        break;
    case ORIENTATION_BOTRIGHT:
        m_image->rotateImage(M_PI);
        break;
    case ORIENTATION_BOTLEFT:
        KisTransformWorker::mirrorY(layer->paintDevice());
        break;
    case ORIENTATION_LEFTTOP:
        m_image->rotateImage(M_PI / 2);
        KisTransformWorker::mirrorY(layer->paintDevice());
        break;
    case ORIENTATION_RIGHTTOP:
        m_image->rotateImage(M_PI / 2);
        break;
    case ORIENTATION_RIGHTBOT:
        m_image->rotateImage(M_PI / 2);
        KisTransformWorker::mirrorX(layer->paintDevice());
        break;
    case ORIENTATION_LEFTBOT:
        m_image->rotateImage(-M_PI / 2 + M_PI * 2);
        break;
    default:
        break;
    }

    // Process IPTC metadata
    if (iptc_profile_size > 0 && iptc_profile_data != nullptr) {
        dbgFile << "Loading IPTC profile. Size: "
                << sizeof(uint32_t) * iptc_profile_size;

        // warning: profile is an array of uint32_t's
        if (TIFFIsByteSwapped(image) != 0) {
            TIFFSwabArrayOfLong(iptc_profile_data,
                                iptc_profile_size / sizeof(uint32_t));
        }

        KisMetaData::IOBackend *iptcIO =
            KisMetadataBackendRegistry::instance()->value("iptc");

        // Copy the xmp data into the byte array
        QByteArray ba(reinterpret_cast<const char *>(iptc_profile_data),
                      static_cast<int>(iptc_profile_size));
        QBuffer buf(&ba);
        iptcIO->loadFrom(layer->metaData(), &buf);
    }

    // Process XMP metadata
    if (xmp_size > 0 && xmp_data != nullptr) {
        dbgFile << "Loading XMP data. Size: " << xmp_size;

        KisMetaData::IOBackend *xmpIO =
            KisMetadataBackendRegistry::instance()->value("xmp");

        // Copy the xmp data into the byte array
        QByteArray ba(reinterpret_cast<char *>(xmp_data),
                      static_cast<int>(xmp_size));
        QBuffer buf(&ba);
        xmpIO->loadFrom(layer->metaData(), &buf);
    }

    return ImportExportCodes::OK;
}

KisImportExportErrorCode KisTIFFImport::readImageFromPsd(KisDocument *m_doc, TIFF *image, KisTiffBasicInfo &basicInfo)
{
#ifdef TIFF_HAS_PSD_TAGS
  // Attempt to parse Photoshop metadata
    // if it succeeds, divert and load as PSD

    if (!m_photoshopBlockParsed) {
        QBuffer photoshopLayerData;

        KisTiffPsdLayerRecord photoshopLayerRecord(TIFFIsBigEndian(image),
                                                   basicInfo.width,
                                                   basicInfo.height,
                                                   basicInfo.depth,
                                                   basicInfo.nbchannels,
                                                   basicInfo.color_type);

        KisTiffPsdResourceRecord photoshopImageResourceRecord;

        {
            // Determine if we have Photoshop metadata
            uint32_t length{0};
            uint8_t *data{nullptr};

            if (TIFFGetField(image, TIFFTAG_IMAGESOURCEDATA, &length, &data)
                == 1) {
                dbgFile << "There are Photoshop layers, processing them now. "
                           "Section size: "
                        << length;

                QByteArray buf(reinterpret_cast<char *>(data),
                               static_cast<int>(length));
                photoshopLayerData.setData(buf);
                photoshopLayerData.open(QIODevice::ReadOnly);

                if (!photoshopLayerRecord.read(photoshopLayerData)) {
                    dbgFile << "TIFF: failed reading Photoshop layer metadata: "
                            << photoshopLayerRecord.record()->error;
                }
            }
        }

        {
            // Determine if we have Photoshop metadata
            uint32_t length{0};
            uint8_t *data{nullptr};

            if (TIFFGetField(image, TIFFTAG_PHOTOSHOP, &length, &data) == 1
                && data != nullptr) {
                dbgFile << "There is Photoshop metadata, processing it now. "
                           "Section size: "
                        << length;

                QByteArray photoshopImageResourceData(
                    reinterpret_cast<char *>(data),
                    static_cast<int>(length));

                QBuffer buf(&photoshopImageResourceData);
                buf.open(QIODevice::ReadOnly);

                if (!photoshopImageResourceRecord.read(buf)) {
                    dbgFile << "TIFF: failed reading Photoshop image metadata: "
                            << photoshopImageResourceRecord.error;
                }
            }
        }

        if (photoshopLayerRecord.valid()
            && photoshopImageResourceRecord.valid()) {

            if (importUserFeedBackInterface()) {

                bool usePsd = true;
                importUserFeedBackInterface()->askUser([&] (QWidget *parent) {
                    usePsd = QMessageBox::question(parent, i18nc("@title:window", "TIFF image with PSD data"),
                                            i18nc("the choice for the user on loading a TIFF file",
                                                "The TIFF image contains valid PSD data embedded. "
                                                "Would you like to use PSD data instead of normal TIFF data?"))
                        == QMessageBox::Yes;

                    return true;
                });

                if (!usePsd) {
                    return ImportExportCodes::Cancelled;
                }
            }

            KisImportExportErrorCode result =
                readImageFromPsdRecords(m_doc,
                                        photoshopLayerRecord,
                                        photoshopImageResourceRecord,
                                        photoshopLayerData,
                                        basicInfo);

            if (!result.isOk()) {
                dbgFile << "Photoshop import failed";
            }
            return result;
        }
    }

    return ImportExportCodes::FormatFeaturesUnsupported;

#else
    return ImportExportCodes::FormatFeaturesUnsupported;
#endif
}

KisImportExportErrorCode KisTIFFImport::readTIFFDirectory(KisDocument *m_doc,
                                                          TIFF *image)
{
    // Read information about the tiff

    KisTiffBasicInfo basicInfo;

    if (TIFFGetField(image, TIFFTAG_IMAGEWIDTH, &basicInfo.width) == 0) {
        dbgFile << "Image does not define its width";
        return ImportExportCodes::FileFormatIncorrect;
    }

    if (TIFFGetField(image, TIFFTAG_IMAGELENGTH, &basicInfo.height) == 0) {
        dbgFile << "Image does not define its height";
        return ImportExportCodes::FileFormatIncorrect;
    }

    if (TIFFGetField(image, TIFFTAG_XRESOLUTION, &basicInfo.xres) == 0 || basicInfo.xres == 0) {
        dbgFile << "Image does not define x resolution";
        // but we don't stop
        basicInfo.xres = 100;
    }

    if (TIFFGetField(image, TIFFTAG_YRESOLUTION, &basicInfo.yres) == 0 || basicInfo.yres == 0) {
        dbgFile << "Image does not define y resolution";
        // but we don't stop
        basicInfo.yres = 100;
    }

    if (TIFFGetField(image, TIFFTAG_RESOLUTIONUNIT, &basicInfo.resolution) == 0) {
        dbgFile << "Image does not define resolution unit";
        // but we don't stop
        basicInfo.resolution = TiffResolution::INCH;
    }

    if (TIFFGetField(image, TIFFTAG_XPOSITION, &basicInfo.x) == 0) {
        dbgFile << "Image does not define a horizontal offset";
        basicInfo.x = 0;
    }

    if (TIFFGetField(image, TIFFTAG_YPOSITION, &basicInfo.y) == 0) {
        dbgFile << "Image does not define a vertical offset";
        basicInfo.y = 0;
    }

    if ((TIFFGetField(image, TIFFTAG_BITSPERSAMPLE, &basicInfo.depth) == 0)) {
        dbgFile << "Image does not define its depth";
        basicInfo.depth = 1;
    }

    if ((TIFFGetField(image, TIFFTAG_SAMPLEFORMAT, &basicInfo.sampletype)
         == 0)) {
        dbgFile << "Image does not define its sample type";
        basicInfo.sampletype = SAMPLEFORMAT_UINT;
    }

    // Determine the number of channels (useful to know if a file has an alpha
    // or not
    if (TIFFGetField(image, TIFFTAG_SAMPLESPERPIXEL, &basicInfo.nbchannels)
        == 0) {
        dbgFile << "Image has an undefined number of samples per pixel";
        basicInfo.nbchannels = 0;
    }

    // Get the number of extrasamples and information about them
    if (TIFFGetField(image,
                     TIFFTAG_EXTRASAMPLES,
                     &basicInfo.extrasamplescount,
                     &basicInfo.sampleinfo)
        == 0) {
        basicInfo.extrasamplescount = 0;
    }

    // Determine the colorspace
    if (TIFFGetField(image, TIFFTAG_PHOTOMETRIC, &basicInfo.color_type) == 0) {
        dbgFile << "Image has an undefined photometric interpretation";
        basicInfo.color_type = PHOTOMETRIC_MINISWHITE;
    }

    basicInfo.colorSpaceIdTag =
        getColorSpaceForColorType(basicInfo.sampletype,
                                  basicInfo.color_type,
                                  basicInfo.depth,
                                  image,
                                  basicInfo.nbchannels,
                                  basicInfo.extrasamplescount,
                                  basicInfo.dstDepth);

    if (basicInfo.colorSpaceIdTag.first.isEmpty()) {
        dbgFile << "Image has an unsupported colorspace :"
                << basicInfo.color_type
                << " for this depth :" << basicInfo.depth;
        return ImportExportCodes::FormatColorSpaceUnsupported;
    }
    dbgFile << "Color space is :" << basicInfo.colorSpaceIdTag.first
            << basicInfo.colorSpaceIdTag.second << " with a depth of"
            << basicInfo.depth << " and with a nb of channels of"
            << basicInfo.nbchannels;

    // Read image profile
    dbgFile << "Reading profile";
    const KoColorProfile *profile = nullptr;
    quint32 EmbedLen = 0;
    uint8_t *EmbedBuffer = nullptr;

    if (TIFFGetField(image, TIFFTAG_ICCPROFILE, &EmbedLen, &EmbedBuffer) == 1) {
        dbgFile << "Profile found";
        QByteArray rawdata(reinterpret_cast<char *>(EmbedBuffer),
                           static_cast<int>(EmbedLen));
        profile = KoColorSpaceRegistry::instance()->createColorProfile(
            basicInfo.colorSpaceIdTag.first,
            basicInfo.colorSpaceIdTag.second,
            rawdata);
    }

    const QString colorSpaceId = KoColorSpaceRegistry::instance()->colorSpaceId(
        basicInfo.colorSpaceIdTag.first,
        basicInfo.colorSpaceIdTag.second);

    // Check that the profile is used by the color space
    if (profile) {
            if (!KoColorSpaceRegistry::instance()->profileIsCompatible(profile, colorSpaceId)
            ||  !(profile->isSuitableForInput() || profile->isSuitableForOutput())) {
            dbgFile << "The profile " << profile->name()
                    << " is not compatible with the color space model "
                    << basicInfo.colorSpaceIdTag.first << " "
                    << basicInfo.colorSpaceIdTag.second;
            profile = nullptr;
        }
    }

    // Do not use the linear gamma profile for 16 bits/channel by default, tiff
    // files are usually created with gamma correction. XXX: Should we ask the
    // user?
    if (!profile) {
        dbgFile << "No profile found; trying to assign a default one.";
        if (basicInfo.colorSpaceIdTag.first == RGBAColorModelID.id()) {
            profile = KoColorSpaceRegistry::instance()->profileByName(
                "sRGB-elle-V2-srgbtrc.icc");
        } else if (basicInfo.colorSpaceIdTag.first == GrayAColorModelID.id()) {
            profile = KoColorSpaceRegistry::instance()->profileByName(
                "Gray-D50-elle-V2-srgbtrc.icc");
        } else if (basicInfo.colorSpaceIdTag.first == CMYKAColorModelID.id()) {
            profile = KoColorSpaceRegistry::instance()->profileByName(
                "Chemical proof");
        } else if (basicInfo.colorSpaceIdTag.first == LABAColorModelID.id()) {
            profile = KoColorSpaceRegistry::instance()->profileByName(
                "Lab identity built-in");
        } else if (basicInfo.colorSpaceIdTag.first == YCbCrAColorModelID.id()) {
            profile = KoColorSpaceRegistry::instance()->profileByName(
                "ITU-R BT.709-6 YCbCr ICC V4 profile");
        }
        if (!profile) {
            dbgFile << "No suitable default profile found.";
        }
    }

    // Retrieve a pointer to the colorspace
    if (profile && profile->isSuitableForWorkspace()) {
        dbgFile << "image has embedded profile:" << profile->name() << "";
        basicInfo.cs = KoColorSpaceRegistry::instance()->colorSpace(
            basicInfo.colorSpaceIdTag.first,
            basicInfo.colorSpaceIdTag.second,
            profile);
    } else {
        // Ensure an empty profile name is supplied so that the fallback logic
        // in KoColorSpaceRegistry is triggered. BUG:464848
        basicInfo.cs = KoColorSpaceRegistry::instance()->colorSpace(basicInfo.colorSpaceIdTag.first,
                                                                    basicInfo.colorSpaceIdTag.second,
                                                                    "");
    }

    if (basicInfo.cs == nullptr) {
        dbgFile << "Color space" << basicInfo.colorSpaceIdTag.first
                << basicInfo.colorSpaceIdTag.second
                << " is not available, please check your installation.";
        return ImportExportCodes::FormatColorSpaceUnsupported;
    }

  // Create the cmsTransform if needed
  if (profile && !profile->isSuitableForWorkspace() && profile->isSuitableForInput()) {
      dbgFile << "The profile can't be used in krita, need conversion";
      basicInfo.transform =
          KoColorSpaceRegistry::instance()
              ->colorSpace(basicInfo.colorSpaceIdTag.first,
                           basicInfo.colorSpaceIdTag.second,
                           profile)
              ->createColorConverter(
                  basicInfo.cs,
                  KoColorConversionTransformation::internalRenderingIntent(),
                  KoColorConversionTransformation::internalConversionFlags());
    }

    KisImportExportErrorCode result = readImageFromPsd(m_doc, image, basicInfo);
    if (!result.isOk()) {
        result = readImageFromTiff(m_doc, image, basicInfo);
    }

    return result;
}

KisImportExportErrorCode
KisTIFFImport::convert(KisDocument *document,
                       QIODevice * /*io*/,
                       KisPropertiesConfigurationSP /*configuration*/)
{
    dbgFile << "Start decoding TIFF File";

    if (!KisImportExportAdditionalChecks::doesFileExist(filename())) {
        return ImportExportCodes::FileNotExist;
    }
    if (!KisImportExportAdditionalChecks::isFileReadable(filename())) {
        return ImportExportCodes::NoAccessToRead;
    }

    QFile file(filename());
    if (!file.open(QFile::ReadOnly)) {
        return KisImportExportErrorCode(KisImportExportErrorCannotRead(file.error()));
    }

    // Open the TIFF file
    const QByteArray encodedFilename = QFile::encodeName(filename());

    // https://gitlab.com/libtiff/libtiff/-/issues/173
#ifdef Q_OS_WIN
    const intptr_t handle = _get_osfhandle(file.handle());
#else
    const int handle = file.handle();
#endif

    std::unique_ptr<TIFF, decltype(&TIFFCleanup)> image(TIFFFdOpen(handle, encodedFilename.data(), "r"), &TIFFCleanup);

    if (!image) {
        dbgFile << "Could not open the file, either it does not exist, either "
                   "it is not a TIFF :"
                << filename();
        return (ImportExportCodes::FileFormatIncorrect);
    }
    dbgFile << "Reading first image descriptor";
    KisImportExportErrorCode result = readTIFFDirectory(document, image.get());
    if (!result.isOk()) {
        return result;
    }

    if (!m_photoshopBlockParsed) {
        // Photoshop images only have one IFD plus the layer blob
        // Ward off inconsistencies by blocking future attempts to parse them
        m_photoshopBlockParsed = true;
        while (TIFFReadDirectory(image.get())) {
            result = readTIFFDirectory(document, image.get());
            if (!result.isOk()) {
                return result;
            }
        }
    }
    // Freeing memory
    image.reset();
    file.close();

    {
        // HACK!! Externally parse the Exif metadata
        // libtiff has no way to access the fields wholesale
        try {
            KisExiv2IODevice::ptr_type basicIoDevice(new KisExiv2IODevice(filename()));

#if EXIV2_TEST_VERSION(0,28,0)
            const std::unique_ptr<Exiv2::Image> readImg = Exiv2::ImageFactory::open(std::move(basicIoDevice));
#else
            const std::unique_ptr<Exiv2::Image> readImg(Exiv2::ImageFactory::open(basicIoDevice).release());
#endif

            readImg->readMetadata();

            const KisMetaData::IOBackend *io =
                KisMetadataBackendRegistry::instance()->value("exif");

            // All IFDs are paint layer children of root
            KisNodeSP node = m_image->rootLayer()->firstChild();

            QBuffer ioDevice;

            {
                // Synthesize the Exif blob
                Exiv2::ExifData tempData;
                Exiv2::Blob tempBlob;

                // NOTE: do not use std::copy_if, auto_ptrs beware
                for (const Exiv2::Exifdatum &i : readImg->exifData()) {
                    const uint16_t tag = i.tag();

                    if (tag == Exif::Image::ImageWidth
                        || tag == Exif::Image::ImageLength
                        || tag == Exif::Image::BitsPerSample
                        || tag == Exif::Image::Compression
                        || tag == Exif::Image::PhotometricInterpretation
                        || tag == Exif::Image::Orientation
                        || tag == Exif::Image::SamplesPerPixel
                        || tag == Exif::Image::PlanarConfiguration
                        || tag == Exif::Image::YCbCrSubSampling
                        || tag == Exif::Image::YCbCrPositioning
                        || tag == Exif::Image::XResolution
                        || tag == Exif::Image::YResolution
                        || tag == Exif::Image::ResolutionUnit
                        || tag == Exif::Image::TransferFunction
                        || tag == Exif::Image::WhitePoint
                        || tag == Exif::Image::PrimaryChromaticities
                        || tag == Exif::Image::YCbCrCoefficients
                        || tag == Exif::Image::ReferenceBlackWhite
                        || tag == Exif::Image::InterColorProfile) {
                        dbgMetaData << "Ignoring TIFF-specific"
                                    << i.key().c_str();
                        continue;
                    }

                    tempData.add(i);
                }

                // Encode into temporary blob
                Exiv2::ExifParser::encode(tempBlob,
                                          Exiv2::littleEndian,
                                          tempData);

                // Reencode into Qt land
                ioDevice.setData(reinterpret_cast<char *>(tempBlob.data()),
                                 static_cast<int>(tempBlob.size()));
            }

            // Get layer
            KisLayer *layer = qobject_cast<KisLayer *>(node.data());
            KIS_ASSERT_RECOVER(layer)
            {
                errFile << "Attempted to import metadata on an empty document";
                return ImportExportCodes::InternalError;
            }

            // Inject the data as any other IOBackend
            io->loadFrom(layer->metaData(), &ioDevice);
#if EXIV2_TEST_VERSION(0,28,0)
        } catch (Exiv2::Error &e) {
            errFile << "Failed metadata import:" << Exiv2::Error(e.code()).what();
#else
        } catch (Exiv2::AnyError &e) {
            errFile << "Failed metadata import:" << e.code() << e.what();
#endif
        }
    }

    document->setCurrentImage(m_image);

    m_image = nullptr;
    m_photoshopBlockParsed = false;

    return ImportExportCodes::OK;
}

#include <kis_tiff_import.moc>
