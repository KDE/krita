/*
 *  SPDX-FileCopyrightText: 2005 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tiff_export.h"

#include <QBuffer>
#include <QFileInfo>

#include <memory>

#include <exiv2/exiv2.hpp>
#include <kpluginfactory.h>
#ifdef Q_OS_WIN
#include <io.h>
#endif
#include <tiffio.h>

#include <KisDocument.h>
#include <KisExportCheckRegistry.h>
#include <KoColorModelStandardIds.h>
#include <KoDocumentInfo.h>
#include <KoUnit.h>
#include <kis_assert.h>
#include <kis_group_layer.h>
#include <kis_layer_utils.h>
#include <kis_meta_data_backend_registry.h>
#include <kis_paint_layer.h>
#include <kis_tiff_writer_visitor.h>
#include <KisExiv2IODevice.h>

#include <config-tiff.h>
#ifdef TIFF_CAN_WRITE_PSD_TAGS
#include "kis_tiff_psd_writer_visitor.h"
#endif

#include "kis_dlg_options_tiff.h"
#include "kis_tiff_converter.h"
#include "kis_tiff_logger.h"

K_PLUGIN_FACTORY_WITH_JSON(KisTIFFExportFactory, "krita_tiff_export.json", registerPlugin<KisTIFFExport>();)

KisTIFFExport::KisTIFFExport(QObject *parent, const QVariantList &)
    : KisImportExportFilter(parent)
    , oldErrHandler(TIFFSetErrorHandler(&KisTiffErrorHandler))
    , oldWarnHandler(TIFFSetWarningHandler(&KisTiffWarningHandler))
{
}

KisTIFFExport::~KisTIFFExport()
{
    TIFFSetErrorHandler(oldErrHandler);
    TIFFSetWarningHandler(oldWarnHandler);
}

KisImportExportErrorCode KisTIFFExport::convert(KisDocument *document, QIODevice */*io*/,  KisPropertiesConfigurationSP configuration)
{
    // If a configuration object was passed to the convert method, we use that, otherwise we load from the settings
    KisPropertiesConfigurationSP cfg(new KisPropertiesConfiguration());
    if (configuration) {
        cfg->fromXML(configuration->toXML());
    }
    else {
        cfg = lastSavedConfiguration(KisDocument::nativeFormatMimeType(), "image/tiff");
    }

    KisTIFFOptions options;
    options.fromProperties(configuration);

    if (!options.flatten && !options.saveAsPhotoshop) {
        const bool hasGroupLayers =
            KisLayerUtils::recursiveFindNode(document->savingImage()->root(),
                [] (KisNodeSP node) {
                    return node->parent() && node->inherits("KisGroupLayer");
                });
        options.flatten = hasGroupLayers;
    }

    KisImageSP kisimage = [&]() {
        if (options.flatten) {
            KisImageSP image =
                new KisImage(nullptr,
                             document->savingImage()->width(),
                             document->savingImage()->height(),
                             document->savingImage()->colorSpace(),
                             "");
            image->setResolution(document->savingImage()->xRes(),
                                 document->savingImage()->yRes());
            KisPaintDeviceSP pd = KisPaintDeviceSP(
                new KisPaintDevice(*document->savingImage()->projection()));
            KisPaintLayerSP l =
                KisPaintLayerSP(new KisPaintLayer(image.data(),
                                                  "projection",
                                                  OPACITY_OPAQUE_U8,
                                                  pd));
            image->addNode(KisNodeSP(l.data()), image->rootLayer().data());
            return image;
        } else {
            return document->savingImage();
        }
    }();

    dbgFile << "Start writing TIFF File";
    KIS_ASSERT_RECOVER_RETURN_VALUE(kisimage, ImportExportCodes::InternalError);

    QFile file(filename());
    if (!file.open(QFile::ReadWrite)) {
        return {KisImportExportErrorCannotRead(file.error())};
    }

    // Open file for writing
    const QByteArray encodedFilename = QFile::encodeName(filename());

    // https://gitlab.com/libtiff/libtiff/-/issues/173
#ifdef Q_OS_WIN
    const int handle = (int)(_get_osfhandle(file.handle()));
#else
    const int handle = file.handle();
#endif

    // NOLINTNEXTLINE(bugprone-narrowing-conversions, cppcoreguidelines-narrowing-conversions)
    std::unique_ptr<TIFF, decltype(&TIFFCleanup)> image(TIFFFdOpen(handle, encodedFilename.data(), "w"), &TIFFCleanup);

    if (!image) {
        dbgFile << "Could not open the file for writing" << filename();
        return ImportExportCodes::NoAccessToWrite;
    }

    // Set the document information
    KoDocumentInfo *info = document->documentInfo();
    QString title = info->aboutInfo("title");
    if (!title.isEmpty()) {
        if (!TIFFSetField(image.get(),
                          TIFFTAG_DOCUMENTNAME,
                          title.toLatin1().constData())) {
            return ImportExportCodes::ErrorWhileWriting;
        }
    }
    QString abstract = info->aboutInfo("description");
    if (!abstract.isEmpty()) {
        if (!TIFFSetField(image.get(),
                          TIFFTAG_IMAGEDESCRIPTION,
                          abstract.toLatin1().constData())) {
            return ImportExportCodes::ErrorWhileWriting;
        }
    }
    QString author = info->authorInfo("creator");
    if (!author.isEmpty()) {
        if (!TIFFSetField(image.get(),
                          TIFFTAG_ARTIST,
                          author.toLatin1().constData())) {
            return ImportExportCodes::ErrorWhileWriting;
        }
    }

    dbgFile << "xres: " << INCH_TO_POINT(kisimage->xRes())
            << " yres: " << INCH_TO_POINT(kisimage->yRes());
    if (!TIFFSetField(
            image.get(),
            TIFFTAG_XRESOLUTION,
            INCH_TO_POINT(kisimage->xRes()))) { // It is the "invert" macro
                                                // because we convert from
                                                // pointer-per-inch to points
        return ImportExportCodes::ErrorWhileWriting;
    }
    if (!TIFFSetField(image.get(),
                      TIFFTAG_YRESOLUTION,
                      INCH_TO_POINT(kisimage->yRes()))) {
        return ImportExportCodes::ErrorWhileWriting;
    }

    if (!TIFFSetField(image.get(), TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH)) {
        return ImportExportCodes::ErrorWhileWriting;
    }

    KisGroupLayer *root =
        dynamic_cast<KisGroupLayer *>(kisimage->rootLayer().data());
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(root,
                                         ImportExportCodes::InternalError);

#ifdef TIFF_CAN_WRITE_PSD_TAGS
    if (options.saveAsPhotoshop) {
        KisTiffPsdWriter writer(image.get(), &options);
        KisImportExportErrorCode result = writer.writeImage(root);
        if (!result.isOk()) {
            return result;
        }
    } else
#endif // TIFF_CAN_WRITE_PSD_TAGS
    {
        KisTIFFWriterVisitor visitor(image.get(), &options);
        if (!(visitor.visit(root))) {
            return ImportExportCodes::Failure;
        }
    }

    image.reset();
    file.close();

    if (!options.flatten && !options.saveAsPhotoshop) {
        // HACK!! Externally inject the Exif metadata
        // libtiff has no way to access the fields wholesale
        try {
            KisExiv2IODevice::ptr_type basicIoDevice(new KisExiv2IODevice(filename()));

#if EXIV2_TEST_VERSION(0,28,0)
            const std::unique_ptr<Exiv2::Image> img = Exiv2::ImageFactory::open(std::move(basicIoDevice));
#else
            const std::unique_ptr<Exiv2::Image> img(Exiv2::ImageFactory::open(basicIoDevice).release());
#endif

            img->readMetadata();

            Exiv2::ExifData &data = img->exifData();

            const KisMetaData::IOBackend *io =
                KisMetadataBackendRegistry::instance()->value("exif");

            // All IFDs are paint layer children of root
            KisNodeSP node = root->firstChild();

            QBuffer ioDevice;

            // Get layer
            KisLayer *layer = qobject_cast<KisLayer *>(node.data());
            Q_ASSERT(layer);

            // Inject the data as any other IOBackend
            io->saveTo(layer->metaData(), &ioDevice);

            Exiv2::ExifData dataToInject;

            // Reinterpret the blob we just got and inject its contents into
            // tempData
            Exiv2::ExifParser::decode(
                dataToInject,
                reinterpret_cast<const Exiv2::byte *>(ioDevice.data().data()),
                static_cast<uint32_t>(ioDevice.size()));

            for (const auto &v : dataToInject) {
                data[v.key()] = v.value();
            }
            // Write metadata
            img->writeMetadata();
#if EXIV2_TEST_VERSION(0,28,0)
        } catch (Exiv2::Error &e) {
            errFile << "Failed injecting TIFF metadata:" << Exiv2::Error(e.code()).what();
#else
        } catch (Exiv2::AnyError &e) {
            errFile << "Failed injecting TIFF metadata:" << e.code()
                    << e.what();
#endif
        }
    }
    return ImportExportCodes::OK;
}

KisPropertiesConfigurationSP KisTIFFExport::defaultConfiguration(const QByteArray &/*from*/, const QByteArray &/*to*/) const
{
    KisTIFFOptions options;
    return options.toProperties();
}

KisConfigWidget *KisTIFFExport::createConfigurationWidget(QWidget *parent, const QByteArray &/*from*/, const QByteArray &/*to*/) const
{
    return new KisTIFFOptionsWidget(parent);
}

void KisTIFFExport::initializeCapabilities()
{
    addCapability(KisExportCheckRegistry::instance()->get("MultiLayerCheck")->create(KisExportCheckBase::SUPPORTED));
    addCapability(KisExportCheckRegistry::instance()->get("LayerOpacityCheck")->create(KisExportCheckBase::PARTIALLY));
    addCapability(KisExportCheckRegistry::instance()->get("sRGBProfileCheck")->create(KisExportCheckBase::SUPPORTED));
    addCapability(KisExportCheckRegistry::instance()->get("ExifCheck")->create(KisExportCheckBase::SUPPORTED));
    addCapability(KisExportCheckRegistry::instance()
                      ->get("TiffExifCheck")
                      ->create(KisExportCheckBase::PARTIALLY));
    addCapability(
        KisExportCheckRegistry::instance()->get("ColorModelHomogenousCheck")->create(KisExportCheckBase::SUPPORTED));

    QList<QPair<KoID, KoID>> supportedColorModels = {
        {},
        {RGBAColorModelID, Integer8BitsColorDepthID},
        {RGBAColorModelID, Integer16BitsColorDepthID},
        {RGBAColorModelID, Float16BitsColorDepthID},
        {RGBAColorModelID, Float32BitsColorDepthID},
        {GrayAColorModelID, Integer8BitsColorDepthID},
        {GrayAColorModelID, Integer16BitsColorDepthID},
        {CMYKAColorModelID, Integer8BitsColorDepthID},
        {CMYKAColorModelID, Integer16BitsColorDepthID},
        {YCbCrAColorModelID, Integer8BitsColorDepthID},
        {YCbCrAColorModelID, Integer16BitsColorDepthID},
        {YCbCrAColorModelID, Float16BitsColorDepthID},
        {YCbCrAColorModelID, Float32BitsColorDepthID},
        {LABAColorModelID, Integer8BitsColorDepthID},
        {LABAColorModelID, Integer16BitsColorDepthID},
        {LABAColorModelID, Float16BitsColorDepthID},
        {LABAColorModelID, Float32BitsColorDepthID}};
    addSupportedColorModels(supportedColorModels, "TIFF");

}

#include <kis_tiff_export.moc>

