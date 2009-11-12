/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */


#ifdef _MSC_VER // this removes KDEWIN extensions to stdint.h: required by exiv2
#define KDEWIN_STDINT_H
#endif

#include "kis_jpeg_converter.h"

#include <stdio.h>
#include <stdint.h>

extern "C" {
#include <iccjpeg.h>
}

#include <exiv2/jpgimage.hpp>

#include <QFile>
#include <QBuffer>

#include <kapplication.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kde_file.h>

#include <kio/netaccess.h>
#include <kio/deletejob.h>

#include <KoDocumentInfo.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_iterators_pixel.h>
#include <kis_paint_layer.h>
#include <kis_group_layer.h>
#include <kis_meta_data_entry.h>
#include <kis_meta_data_value.h>
#include <kis_meta_data_store.h>
#include <kis_meta_data_io_backend.h>
#include <kis_paint_device.h>
#include <kis_transform_worker.h>
#include <kis_jpeg_source.h>
#include <kis_jpeg_destination.h>

#include <colorprofiles/KoIccColorProfile.h>

#define ICC_MARKER  (JPEG_APP0 + 2) /* JPEG marker code for ICC */
#define ICC_OVERHEAD_LEN  14    /* size of non-profile data in APP2 */
#define MAX_BYTES_IN_MARKER  65533  /* maximum data len of a JPEG marker */
#define MAX_DATA_BYTES_IN_MARKER  (MAX_BYTES_IN_MARKER - ICC_OVERHEAD_LEN)

const char photoshopMarker[] = "Photoshop 3.0\0";
const char photoshopBimId_[] = "8BIM";
const uint16_t photoshopIptc = 0x0404;
const char xmpMarker[] = "http://ns.adobe.com/xap/1.0/\0";
const QByteArray photoshopIptc_((char*)&photoshopIptc, 2);

namespace
{

J_COLOR_SPACE getColorTypeforColorSpace(const KoColorSpace * cs)
{
    if (KoID(cs->id()) == KoID("GRAYA") || KoID(cs->id()) == KoID("GRAYA16")) {
        return JCS_GRAYSCALE;
    }
    if (KoID(cs->id()) == KoID("RGBA") || KoID(cs->id()) == KoID("RGBA16")) {
        return JCS_RGB;
    }
    if (KoID(cs->id()) == KoID("CMYK") || KoID(cs->id()) == KoID("CMYK16")) {
        return JCS_CMYK;
    }
    KMessageBox::error(0, i18n("Cannot export images in %1.\n", cs->name())) ;
    return JCS_UNKNOWN;
}

QString getColorSpaceForColorType(J_COLOR_SPACE color_type)
{
    dbgFile << "color_type =" << color_type;
    if (color_type == JCS_GRAYSCALE) {
        return "GRAYA";
    } else if (color_type == JCS_RGB) {
        return "RGBA";
    } else if (color_type == JCS_CMYK) {
        return "CMYK";
    }
    return "";
}

}

KisJPEGConverter::KisJPEGConverter(KisDoc2 *doc, KisUndoAdapter *adapter)
{
    m_doc = doc;
    m_adapter = adapter;
    m_job = 0;
    m_stop = false;
}

KisJPEGConverter::~KisJPEGConverter()
{
}

KisImageBuilder_Result KisJPEGConverter::decode(const KUrl& uri)
{
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    Q_ASSERT(uri.isLocalFile());

    // open the file
    QFile file(QFile::encodeName(uri.toLocalFile()));
    if (!file.exists()) {
        return (KisImageBuilder_RESULT_NOT_EXIST);
    }
    if (!file.open(QIODevice::ReadOnly)) {
        return (KisImageBuilder_RESULT_BAD_FETCH);
    }
    
    KisJPEGSource::setSource(&cinfo, &file);

    jpeg_save_markers(&cinfo, JPEG_COM, 0xFFFF);
    /* Save APP0..APP15 markers */
    for (int m = 0; m < 16; m++)
        jpeg_save_markers(&cinfo, JPEG_APP0 + m, 0xFFFF);


//     setup_read_icc_profile(&cinfo);
    // read header
    jpeg_read_header(&cinfo, true);

    // start reading
    jpeg_start_decompress(&cinfo);

    // Get the colorspace
    QString csName = getColorSpaceForColorType(cinfo.out_color_space);
    if (csName.isEmpty()) {
        dbgFile << "unsupported colorspace :" << cinfo.out_color_space;
        jpeg_destroy_decompress(&cinfo);
        return KisImageBuilder_RESULT_UNSUPPORTED_COLORSPACE;
    }
    uchar* profile_data;
    uint profile_len;
    KoColorProfile* profile = 0;
    QByteArray profile_rawdata;
    if (read_icc_profile(&cinfo, &profile_data, &profile_len)) {
        profile_rawdata.resize(profile_len);
        memcpy(profile_rawdata.data(), profile_data, profile_len);
        cmsHPROFILE hProfile = cmsOpenProfileFromMem(profile_data, (DWORD)profile_len);

        if (hProfile != (cmsHPROFILE) NULL) {
            profile = new KoIccColorProfile(profile_rawdata);
            Q_CHECK_PTR(profile);
//             dbgFile <<"profile name:" << profile->productName() <<" profile description:" << profile->productDescription() <<" information sur le produit:" << profile->productInfo();
            if (!profile->isSuitableForOutput()) {
                dbgFile << "the profile is not suitable for output and therefore cannot be used in krita, we need to convert the image to a standard profile"; // TODO: in ko2 popup a selection menu to inform the user
            }
        }
    }

    // Retrieve a pointer to the colorspace
    const KoColorSpace* cs;
    if (profile && profile->isSuitableForOutput()) {
        dbgFile << "image has embedded profile:" << profile -> name() << "";
        cs = KoColorSpaceRegistry::instance()->colorSpace(csName, profile);
    } else
        cs = KoColorSpaceRegistry::instance()->colorSpace(KoID(csName, ""), "");

    if (cs == 0) {
        dbgFile << "unknown colorspace";
        jpeg_destroy_decompress(&cinfo);
        return KisImageBuilder_RESULT_UNSUPPORTED_COLORSPACE;
    }
    // TODO fixit
    // Create the cmsTransform if needed

    KoColorTransformation* transform = 0;
    if (profile && !profile->isSuitableForOutput()) {
        transform = KoColorSpaceRegistry::instance()->colorSpace(csName, profile)->createColorConverter(cs);
    }

    // Creating the KisImageWSP
    if (! m_img) {
        m_img = new KisImage(m_doc->undoAdapter(),  cinfo.image_width,  cinfo.image_height, cs, "built image");
        Q_CHECK_PTR(m_img);
        m_img->lock();
        if (profile && !profile->isSuitableForOutput()) {
            m_img -> addAnnotation(KisAnnotationSP(new KisAnnotation(profile->name(), "", profile_rawdata)));
        }
    }

    // Set resolution
    double xres = 72, yres = 72;
    if ( cinfo.density_unit == 1 )
    {
        xres = cinfo.X_density;
        yres = cinfo.Y_density;
    }
    else if ( cinfo.density_unit == 2 )
    {
        xres = cinfo.X_density * 2.54;
        yres = cinfo.Y_density * 2.54;
    }
    m_img->setResolution( POINT_TO_INCH(xres), POINT_TO_INCH(yres) ); // It is the "invert" macro because we convert from pointer-per-inchs to points
    
    // Create layer
    KisPaintLayerSP layer = KisPaintLayerSP(new KisPaintLayer(m_img.data(), m_img -> nextLayerName(), quint8_MAX));

    // Read data
    JSAMPROW row_pointer = new JSAMPLE[cinfo.image_width*cinfo.num_components];

    for (; cinfo.output_scanline < cinfo.image_height;) {
        KisHLineIterator it = layer->paintDevice()->createHLineIterator(0, cinfo.output_scanline, cinfo.image_width);
        jpeg_read_scanlines(&cinfo, &row_pointer, 1);
        quint8 *src = row_pointer;
        switch (cinfo.out_color_space) {
        case JCS_GRAYSCALE:
            while (!it.isDone()) {
                quint8 *d = it.rawData();
                d[0] = *(src++);
                if (transform) transform->transform(d, d, 1);
                d[1] = quint8_MAX;
                ++it;
            }
            break;
        case JCS_RGB:
            while (!it.isDone()) {
                quint8 *d = it.rawData();
                d[2] = *(src++);
                d[1] = *(src++);
                d[0] = *(src++);
                if (transform) transform->transform(d, d, 1);
                d[3] = quint8_MAX;
                ++it;
            }
            break;
        case JCS_CMYK:
            while (!it.isDone()) {
                quint8 *d = it.rawData();
                d[0] = quint8_MAX - *(src++);
                d[1] = quint8_MAX - *(src++);
                d[2] = quint8_MAX - *(src++);
                d[3] = quint8_MAX - *(src++);
                if (transform) transform->transform(d, d, 1);
                d[4] = quint8_MAX;
                ++it;
            }
            break;
        default:
            return KisImageBuilder_RESULT_UNSUPPORTED;
        }
    }

    m_img->addNode(KisNodeSP(layer.data()), m_img->rootLayer().data());
    layer->setDirty();

    // Read exif information

    dbgFile << "Looking for exif information";

    for (jpeg_saved_marker_ptr marker = cinfo.marker_list; marker != NULL; marker = marker->next) {
        dbgFile << "Marker is" << marker->marker;
        if (marker->marker != (JOCTET)(JPEG_APP0 + 1)
                || marker->data_length < 14) {
            continue; /* Exif data is in an APP1 marker of at least 14 octets */
        }

        if (GETJOCTET(marker->data[0]) != (JOCTET) 0x45 ||
                GETJOCTET(marker->data[1]) != (JOCTET) 0x78 ||
                GETJOCTET(marker->data[2]) != (JOCTET) 0x69 ||
                GETJOCTET(marker->data[3]) != (JOCTET) 0x66 ||
                GETJOCTET(marker->data[4]) != (JOCTET) 0x00 ||
                GETJOCTET(marker->data[5]) != (JOCTET) 0x00)
            continue; /* no Exif header */
        dbgFile << "Found exif information of length :" << marker->data_length;
        KisMetaData::IOBackend* exifIO = KisMetaData::IOBackendRegistry::instance()->value("exif");
        Q_ASSERT(exifIO);
        QByteArray byteArray((const char*)marker->data + 6, marker->data_length - 6);
        QBuffer buf(&byteArray);
        exifIO->loadFrom(layer->metaData(), &buf);
        // Interpret orientation tag
        if (layer->metaData()->containsEntry("http://ns.adobe.com/tiff/1.0/", "Orientation")) {
            KisMetaData::Entry& entry = layer->metaData()->getEntry("http://ns.adobe.com/tiff/1.0/", "Orientation");
            if (entry.value().type() == KisMetaData::Value::Variant) {
                switch (entry.value().asVariant().toInt()) {
                case 2:
                    KisTransformWorker::mirrorY(layer->paintDevice());
                    break;
                case 3:
                    image()->rotate(M_PI, 0);
                    break;
                case 4:
                    KisTransformWorker::mirrorX(layer->paintDevice());
                    break;
                case 5:
                    image()->rotate(M_PI / 2, 0);
                    KisTransformWorker::mirrorY(layer->paintDevice());
                    break;
                case 6:
                    image()->rotate(M_PI / 2, 0);
                    break;
                case 7:
                    image()->rotate(M_PI / 2, 0);
                    KisTransformWorker::mirrorX(layer->paintDevice());
                    break;
                case 8:
                    image()->rotate(-M_PI / 2 + M_PI*2, 0);
                    break;
                default:
                    break;
                }
            }
            entry.value().setVariant(1);
        }
        break;
    }

    dbgFile << "Looking for IPTC information";

    for (jpeg_saved_marker_ptr marker = cinfo.marker_list; marker != NULL; marker = marker->next) {
        dbgFile << "Marker is" << marker->marker;
        if (marker->marker != (JOCTET)(JPEG_APP0 + 13) ||  marker->data_length < 14) {
            continue; /* IPTC data is in an APP13 marker of at least 16 octets */
        }
        if (memcmp(marker->data, photoshopMarker, 14) != 0) {
            for (int i = 0; i < 14; i++) {
                dbgFile << (int)(*(marker->data + i)) << "" << (int)(photoshopMarker[i]);
            }
            dbgFile << "No photoshop marker";
            continue; /* No IPTC Header */
        }

        dbgFile << "Found Photoshop information of length :" << marker->data_length;
        KisMetaData::IOBackend* iptcIO = KisMetaData::IOBackendRegistry::instance()->value("iptc");
        Q_ASSERT(iptcIO);
        const Exiv2::byte *record = 0;
        uint32_t sizeIptc = 0;
        uint32_t sizeHdr = 0;
        // Find actual Iptc data within the APP13 segment
        if (!Exiv2::Photoshop::locateIptcIrb((Exiv2::byte*)(marker->data + 14),
                                             marker->data_length - 14, &record, &sizeHdr, &sizeIptc)) {
            if (sizeIptc) {
                // Decode the IPTC data
                QByteArray byteArray((const char*)(record + sizeHdr), sizeIptc);
                iptcIO->loadFrom(layer->metaData(), new QBuffer(&byteArray));
            } else {
                dbgFile << "IPTC Not found in Photoshop marker";
            }
        }
        break;
    }

    dbgFile << "Looking for XMP information";

    for (jpeg_saved_marker_ptr marker = cinfo.marker_list; marker != NULL; marker = marker->next) {
        dbgFile << "Marker is" << marker->marker;
        if (marker->marker != (JOCTET)(JPEG_APP0 + 1) || marker->data_length < 31) {
            continue; /* XMP data is in an APP1 marker of at least 31 octets */
        }
        if (memcmp(marker->data, xmpMarker, 29) != 0) {
            dbgFile << "Not XMP marker";
            continue; /* No xmp Header */
        }
        dbgFile << "Found XMP Marker of length " << marker->data_length;
        QByteArray byteArray((const char*)marker->data + 29, marker->data_length - 29);
        KisMetaData::IOBackend* xmpIO = KisMetaData::IOBackendRegistry::instance()->value("xmp");
        Q_ASSERT(xmpIO);
        xmpIO->loadFrom(layer->metaData(), new QBuffer(&byteArray));
        break;
    }

    // Dump loaded metadata
    layer->metaData()->debugDump();

    // Finish decompression
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    delete [] row_pointer;
    return KisImageBuilder_RESULT_OK;
}



KisImageBuilder_Result KisJPEGConverter::buildImage(const KUrl& uri)
{
    if (uri.isEmpty())
        return KisImageBuilder_RESULT_NO_URI;

    if (!KIO::NetAccess::exists(uri, KIO::NetAccess::SourceSide, QApplication::activeWindow())) {
        return KisImageBuilder_RESULT_NOT_EXIST;
    }

    // We're not set up to handle asynchronous loading at the moment.
    KisImageBuilder_Result result = KisImageBuilder_RESULT_FAILURE;
    QString tmpFile;

    if (KIO::NetAccess::download(uri, tmpFile, QApplication::activeWindow())) {
        KUrl uriTF;
        uriTF.setPath(tmpFile);
        result = decode(uriTF);
        KIO::NetAccess::removeTempFile(tmpFile);
    }

    return result;
}


KisImageWSP KisJPEGConverter::image()
{
    return m_img;
}


KisImageBuilder_Result KisJPEGConverter::buildFile(const KUrl& uri, KisPaintLayerSP layer, vKisAnnotationSP_it annotationsStart, vKisAnnotationSP_it annotationsEnd, KisJPEGOptions options, KisMetaData::Store* metaData)
{
    if (!layer)
        return KisImageBuilder_RESULT_INVALID_ARG;

    KisImageWSP img = KisImageWSP(layer -> image());
    if (!img)
        return KisImageBuilder_RESULT_EMPTY;

    if (uri.isEmpty())
        return KisImageBuilder_RESULT_NO_URI;

    if (!uri.isLocalFile())
        return KisImageBuilder_RESULT_NOT_LOCAL;

    // Open file for writing
    QFile file(QFile::encodeName(uri.toLocalFile()));
    if (!file.open(QIODevice::WriteOnly)) {
        return (KisImageBuilder_RESULT_FAILURE);
    }

    uint height = img->height();
    uint width = img->width();
    // Initialize structure
    struct jpeg_compress_struct cinfo;
    jpeg_create_compress(&cinfo);
    // Initialize error output
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    // Initialize output stream
    KisJPEGDestination::setDestination(&cinfo, &file);

    const KoColorSpace * cs = img->colorSpace();

    cinfo.image_width = width;  // image width and height, in pixels
    cinfo.image_height = height;
    cinfo.input_components = cs->colorChannelCount(); // number of color channels per pixel */
    J_COLOR_SPACE color_type = getColorTypeforColorSpace(cs);
    if (color_type == JCS_UNKNOWN) {
        (void)file.remove();
        return KisImageBuilder_RESULT_UNSUPPORTED_COLORSPACE;
    }
    cinfo.in_color_space = color_type;   // colorspace of input image

    // Set default compression parameters
    jpeg_set_defaults(&cinfo);
    // Customize them
    jpeg_set_quality(&cinfo, options.quality, options.baseLineJPEG);

    if (options.progressive) {
        jpeg_simple_progression(&cinfo);
    }
    // Optimize ?
    cinfo.optimize_coding = options.optimize;

    // Smoothing
    cinfo.smoothing_factor = options.smooth;

    // Subsampling
    switch (options.subsampling) {
    default:
    case 0: {
        cinfo.comp_info[0].h_samp_factor = 2;
        cinfo.comp_info[0].v_samp_factor = 2;
        cinfo.comp_info[1].h_samp_factor = 1;
        cinfo.comp_info[1].v_samp_factor = 1;
        cinfo.comp_info[2].h_samp_factor = 1;
        cinfo.comp_info[2].v_samp_factor = 1;

    }
    break;
    case 1: {
        cinfo.comp_info[0].h_samp_factor = 2;
        cinfo.comp_info[0].v_samp_factor = 1;
        cinfo.comp_info[1].h_samp_factor = 1;
        cinfo.comp_info[1].v_samp_factor = 1;
        cinfo.comp_info[2].h_samp_factor = 1;
        cinfo.comp_info[2].v_samp_factor = 1;
    }
    break;
    case 2: {
        cinfo.comp_info[0].h_samp_factor = 1;
        cinfo.comp_info[0].v_samp_factor = 2;
        cinfo.comp_info[1].h_samp_factor = 1;
        cinfo.comp_info[1].v_samp_factor = 1;
        cinfo.comp_info[2].h_samp_factor = 1;
        cinfo.comp_info[2].v_samp_factor = 1;
    }
    break;
    case 3: {
        cinfo.comp_info[0].h_samp_factor = 1;
        cinfo.comp_info[0].v_samp_factor = 1;
        cinfo.comp_info[1].h_samp_factor = 1;
        cinfo.comp_info[1].v_samp_factor = 1;
        cinfo.comp_info[2].h_samp_factor = 1;
        cinfo.comp_info[2].v_samp_factor = 1;
    }
    break;
    }

    // Save resolution
    cinfo.X_density = INCH_TO_POINT(img->xRes()); // It is the "invert" macro because we convert from pointer-per-inchs to points
    cinfo.Y_density = INCH_TO_POINT(img->yRes()); // It is the "invert" macro because we convert from pointer-per-inchs to points
    cinfo.density_unit = 1;
    cinfo.write_JFIF_header = 1;

    // Start compression
    jpeg_start_compress(&cinfo, true);
    // Save exif and iptc information if any available

    if (metaData && !metaData->empty()) {
        metaData->applyFilters(options.filters);
        // Save EXIF
        if (options.exif) {
            dbgFile << "Trying to save exif information";

            KisMetaData::IOBackend* exifIO = KisMetaData::IOBackendRegistry::instance()->value("exif");
            Q_ASSERT(exifIO);

            QBuffer buffer;
            exifIO->saveTo(metaData, &buffer, KisMetaData::IOBackend::JpegHeader);

            dbgFile << "Exif information size is" << buffer.data().size();
            QByteArray data = buffer.data();
            if (data.size() < MAX_DATA_BYTES_IN_MARKER) {
                jpeg_write_marker(&cinfo, JPEG_APP0 + 1, (const JOCTET*)data.data(), data.size());
            } else {
                dbgFile << "EXIF information could not be saved."; // TODO: warn the user ?
            }
        }
        // Save IPTC
        if (options.iptc) {
            dbgFile << "Trying to save exif information";
            KisMetaData::IOBackend* iptcIO = KisMetaData::IOBackendRegistry::instance()->value("iptc");
            Q_ASSERT(iptcIO);

            QBuffer buffer;
            iptcIO->saveTo(metaData, &buffer, KisMetaData::IOBackend::JpegHeader);

            dbgFile << "IPTC information size is" << buffer.data().size();
            QByteArray data = buffer.data();
            if (data.size() < MAX_DATA_BYTES_IN_MARKER) {
                jpeg_write_marker(&cinfo, JPEG_APP0 + 13, (const JOCTET*)data.data(), data.size());
            } else {
                dbgFile << "IPTC information could not be saved."; // TODO: warn the user ?
            }
        }
        // Save XMP
        if (options.xmp) {
            dbgFile << "Trying to save XMP information";
            KisMetaData::IOBackend* xmpIO = KisMetaData::IOBackendRegistry::instance()->value("xmp");
            Q_ASSERT(xmpIO);

            QBuffer buffer;
            xmpIO->saveTo(metaData, &buffer, KisMetaData::IOBackend::JpegHeader);

            dbgFile << "XMP information size is" << buffer.data().size();
            QByteArray data = buffer.data();
            if (data.size() < MAX_DATA_BYTES_IN_MARKER) {
                jpeg_write_marker(&cinfo, JPEG_APP0 + 14, (const JOCTET*)data.data(), data.size());
            } else {
                dbgFile << "XMP information could not be saved."; // TODO: warn the user ?
            }
        }
    }

    // Save annotation
    vKisAnnotationSP_it it = annotationsStart;
    while (it != annotationsEnd) {
        if (!(*it) || (*it)->type().isEmpty()) {
            dbgFile << "Warning: empty annotation";
            ++it;
            continue;
        }

        dbgFile << "Trying to store annotation of type" << (*it) -> type() << " of size" << (*it) -> annotation() . size();

        if ((*it) -> type().startsWith(QLatin1String("krita_attribute:"))) { // Attribute
            // FIXME
            dbgFile << "cannot save this annotation :" << (*it) -> type();
        } else { // Profile
            write_icc_profile(& cinfo, (uchar*)(*it)->annotation().data(), (*it)->annotation().size());
        }
        ++it;
    }


    // Write data information

    JSAMPROW row_pointer = new JSAMPLE[width*cinfo.input_components];
    int color_nb_bits = 8 * layer->paintDevice()->pixelSize() / layer->paintDevice()->channelCount();

    for (; cinfo.next_scanline < height;) {
        KisHLineConstIterator it = layer->paintDevice()->createHLineConstIterator(0, cinfo.next_scanline, width);
        quint8 *dst = row_pointer;
        switch (color_type) {
        case JCS_GRAYSCALE:
            if (color_nb_bits == 16) {
                while (!it.isDone()) {
                    //const quint16 *d = reinterpret_cast<const quint16 *>(it.rawData());
                    const quint8 *d = it.rawData();
                    *(dst++) = cs->scaleToU8(d, 0);//d[0] / quint8_MAX;
                    ++it;
                }
            } else {
                while (!it.isDone()) {
                    const quint8 *d = it.rawData();
                    *(dst++) = d[0];
                    ++it;
                }
            }
            break;
        case JCS_RGB:
            if (color_nb_bits == 16) {
                while (!it.isDone()) {
                    //const quint16 *d = reinterpret_cast<const quint16 *>(it.rawData());
                    const quint8 *d = it.rawData();
                    *(dst++) = cs->scaleToU8(d, 2); //d[2] / quint8_MAX;
                    *(dst++) = cs->scaleToU8(d, 1); //d[1] / quint8_MAX;
                    *(dst++) = cs->scaleToU8(d, 0); //d[0] / quint8_MAX;
                    ++it;
                }
            } else {
                while (!it.isDone()) {
                    const quint8 *d = it.rawData();
                    *(dst++) = d[2];
                    *(dst++) = d[1];
                    *(dst++) = d[0];
                    ++it;
                }
            }
            break;
        case JCS_CMYK:
            if (color_nb_bits == 16) {
                while (!it.isDone()) {
                    //const quint16 *d = reinterpret_cast<const quint16 *>(it.rawData());
                    const quint8 *d = it.rawData();
                    *(dst++) = cs->scaleToU8(d, 0);//quint8_MAX - d[0] / quint8_MAX;
                    *(dst++) = cs->scaleToU8(d, 1);//quint8_MAX - d[1] / quint8_MAX;
                    *(dst++) = cs->scaleToU8(d, 2);//quint8_MAX - d[2] / quint8_MAX;
                    *(dst++) = cs->scaleToU8(d, 3);//quint8_MAX - d[3] / quint8_MAX;
                    ++it;
                }
            } else {
                while (!it.isDone()) {
                    const quint8 *d = it.rawData();
                    *(dst++) = quint8_MAX - d[0];
                    *(dst++) = quint8_MAX - d[1];
                    *(dst++) = quint8_MAX - d[2];
                    *(dst++) = quint8_MAX - d[3];
                    ++it;
                }
            }
            break;
        default:
            KIO::del(uri); // asynchronous, but I guess that's ok
            delete [] row_pointer;
            jpeg_destroy_compress(&cinfo);
            return KisImageBuilder_RESULT_UNSUPPORTED;
        }
        jpeg_write_scanlines(&cinfo, &row_pointer, 1);
    }


    // Writing is over
    jpeg_finish_compress(&cinfo);
    file.close();

    delete [] row_pointer;
    // Free memory
    jpeg_destroy_compress(&cinfo);

    return KisImageBuilder_RESULT_OK;
}


void KisJPEGConverter::cancel()
{
    m_stop = true;
}

#include "kis_jpeg_converter.moc"

