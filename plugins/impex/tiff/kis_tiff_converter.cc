/*
 *  SPDX-FileCopyrightText: 2005-2006 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tiff_converter.h"

#include <cstdio>
#include <exiv2/exif.hpp>
#include <exiv2/exiv2.hpp>
#include <exiv2/metadatum.hpp>
#include <exiv2/tags.hpp>
#include <map>

#include <QApplication>
#include <QBuffer>
#include <QFile>
#include <QFileInfo>
#include <QStack>

#include <KisDocument.h>

#include <KoColorModelStandardIds.h>
#include <KoColorSpace.h>
#include <KoColorSpaceConstants.h>
#include <KoColorSpaceRegistry.h>
#include <KoDocumentInfo.h>
#include <KoUnit.h>
#include <kis_assert.h>
#include <kis_buffer_stream.h>
#include <kis_debug.h>
#include <kis_global.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_meta_data_backend_registry.h>
#include <kis_meta_data_tags.h>
#include <kis_paint_layer.h>
#include <kis_transaction.h>
#include <kis_transform_worker.h>
#include <kis_transparency_mask.h>
#include <psd_resource_block.h>

#include "kis_tiff_psd_layer_record.h"
#include "kis_tiff_psd_resource_record.h"
#include "kis_tiff_psd_writer_visitor.h"
#include "kis_tiff_reader.h"
#include "kis_tiff_writer_visitor.h"
#include "kis_tiff_ycbcr_reader.h"

#if TIFFLIB_VERSION < 20111221
typedef size_t tmsize_t;
#endif

KisPropertiesConfigurationSP KisTIFFOptions::toProperties() const
{
    QHash<int, int> compToIndex;
    compToIndex[COMPRESSION_NONE] = 0;
    compToIndex[COMPRESSION_JPEG] = 1;
    compToIndex[COMPRESSION_DEFLATE] = 2;
    compToIndex[COMPRESSION_LZW] = 3;
    compToIndex[COMPRESSION_PIXARLOG] = 8;

    const QHash<quint16, int> psdCompToIndex = {
        {psd_compression_type::RLE, 0},
        {psd_compression_type::ZIP, 1},
    };

    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();

    cfg->setProperty("compressiontype", compToIndex.value(compressionType, 0));
    cfg->setProperty("predictor", predictor - 1);
    cfg->setProperty("alpha", alpha);
    cfg->setProperty("psdCompressionType", psdCompToIndex.value(psdCompressionType, 0));
    cfg->setProperty("saveAsPhotoshop", saveAsPhotoshop);
    cfg->setProperty("flatten", flatten);
    cfg->setProperty("quality", jpegQuality);
    cfg->setProperty("deflate", deflateCompress);
    cfg->setProperty("pixarlog", pixarLogCompress);
    cfg->setProperty("saveProfile", saveProfile);

    return cfg;
}

void KisTIFFOptions::fromProperties(KisPropertiesConfigurationSP cfg)
{
    QHash<int, int> indexToComp;
    indexToComp[0] = COMPRESSION_NONE;
    indexToComp[1] = COMPRESSION_JPEG;
    indexToComp[2] = COMPRESSION_DEFLATE;
    indexToComp[3] = COMPRESSION_LZW;
    indexToComp[4] = COMPRESSION_PIXARLOG;

    // old value that might be still stored in a config (remove after Krita 5.0 :) )
    indexToComp[8] = COMPRESSION_PIXARLOG;

    const QHash<int, quint16> psdIndexToComp = {
        {0, psd_compression_type::RLE},
        {1, psd_compression_type::ZIP},
    };

    compressionType = static_cast<quint16>(indexToComp.value(cfg->getInt("compressiontype", 0), COMPRESSION_NONE));

    predictor = static_cast<quint16>(cfg->getInt("predictor", 0)) + 1;
    alpha = cfg->getBool("alpha", true);
    saveAsPhotoshop = cfg->getBool("saveAsPhotoshop", false);
    psdCompressionType = psdIndexToComp.value(cfg->getInt("psdCompressionType", 0), psd_compression_type::RLE);
    flatten = cfg->getBool("flatten", true);
    jpegQuality = static_cast<quint16>(cfg->getInt("quality", 80));
    deflateCompress = static_cast<quint16>(cfg->getInt("deflate", 6));
    pixarLogCompress = static_cast<quint16>(cfg->getInt("pixarlog", 6));
    saveProfile = cfg->getBool("saveProfile", true);
}

KisTIFFConverter::KisTIFFConverter(KisDocument *doc)
    : m_doc(doc)
    , m_stop(false)
{
    TIFFSetWarningHandler(0);
    TIFFSetErrorHandler(0);
}

KisTIFFConverter::~KisTIFFConverter()
{
}

KisImageSP KisTIFFConverter::image()
{
    return m_image;
}

KisImportExportErrorCode KisTIFFConverter::buildFile(const QString &filename, KisImageSP kisimage, KisTIFFOptions options)
{
    dbgFile << "Start writing TIFF File";
    KIS_ASSERT_RECOVER_RETURN_VALUE(kisimage, ImportExportCodes::InternalError);

    // Open file for writing
    TIFF *image;
    if ((image = TIFFOpen(QFile::encodeName(filename), "w")) == 0) {
        dbgFile << "Could not open the file for writing" << filename;
        return ImportExportCodes::NoAccessToWrite;
    }

    // Set the document information
    KoDocumentInfo *info = m_doc->documentInfo();
    QString title = info->aboutInfo("title");
    if (!title.isEmpty()) {
        if (!TIFFSetField(image, TIFFTAG_DOCUMENTNAME, title.toLatin1().constData())) {
            TIFFClose(image);
            return ImportExportCodes::ErrorWhileWriting;
        }
    }
    QString abstract = info->aboutInfo("description");
    if (!abstract.isEmpty()) {
        if (!TIFFSetField(image, TIFFTAG_IMAGEDESCRIPTION, abstract.toLatin1().constData())) {
            TIFFClose(image);
            return ImportExportCodes::ErrorWhileWriting;
        }
    }
    QString author = info->authorInfo("creator");
    if (!author.isEmpty()) {
        if (!TIFFSetField(image, TIFFTAG_ARTIST, author.toLatin1().constData())) {
            TIFFClose(image);
            return ImportExportCodes::ErrorWhileWriting;
        }
    }

    dbgFile << "xres: " << INCH_TO_POINT(kisimage->xRes()) << " yres: " << INCH_TO_POINT(kisimage->yRes());
    if (!TIFFSetField(image, TIFFTAG_XRESOLUTION, INCH_TO_POINT(kisimage->xRes()))) { // It is the "invert" macro because we convert from pointer-per-inchs to points
        TIFFClose(image);
        return ImportExportCodes::ErrorWhileWriting;
    }
    if (!TIFFSetField(image, TIFFTAG_YRESOLUTION, INCH_TO_POINT(kisimage->yRes()))) {
        TIFFClose(image);
        return ImportExportCodes::ErrorWhileWriting;
    }

    KisGroupLayer *root = dynamic_cast<KisGroupLayer *>(kisimage->rootLayer().data());
    KIS_ASSERT_RECOVER(root)
    {
        TIFFClose(image);
        return ImportExportCodes::InternalError;
    }

#ifdef TIFF_CAN_WRITE_PSD_TAGS
    if (options.saveAsPhotoshop) {
        KisTiffPsdWriter writer(image, &options);
        KisImportExportErrorCode result = writer.writeImage(root);
        if (!result.isOk()) {
            TIFFClose(image);
            return result;
        }
    } else
#endif // TIFF_CAN_WRITE_PSD_TAGS
    {
        KisTIFFWriterVisitor *visitor = new KisTIFFWriterVisitor(image, &options);
        if (!(visitor->visit(root))) {
            TIFFClose(image);
            return ImportExportCodes::Failure;
        }
    }

    TIFFClose(image);

    if (!options.flatten && !options.saveAsPhotoshop) {
        // HACK!! Externally inject the Exif metadata
        // libtiff has no way to access the fields wholesale
        try {
            Exiv2::BasicIo::AutoPtr fileIo(new Exiv2::FileIo(QFile::encodeName(filename).toStdString()));

            Exiv2::Image::AutoPtr img(Exiv2::ImageFactory::open(fileIo));

            img->readMetadata();

            auto &data = img->exifData();

            const KisMetaData::IOBackend *io = KisMetadataBackendRegistry::instance()->value("exif");

            // All IFDs are paint layer children of root
            KisNodeSP node = root->firstChild();

            QBuffer ioDevice;

            // Get layer
            KisLayer *layer = qobject_cast<KisLayer *>(node.data());
            Q_ASSERT(layer);

            // Inject the data as any other IOBackend
            io->saveTo(layer->metaData(), &ioDevice);

            Exiv2::ExifData dataToInject;

            // Reinterpret the blob we just got and inject its contents into tempData
            Exiv2::ExifParser::decode(dataToInject,
                                        reinterpret_cast<const Exiv2::byte *>(ioDevice.data().data()),
                                        static_cast<uint32_t>(ioDevice.size()));

            for (const auto &v : dataToInject) {
                data[v.key()] = v.value();
            }
            // Write metadata
            img->writeMetadata();
        } catch (Exiv2::AnyError &e) {
            errFile << "Failed injecting TIFF metadata:" << e.code() << e.what();
        }
    }
    return ImportExportCodes::OK;
}

void KisTIFFConverter::cancel()
{
    m_stop = true;
}
