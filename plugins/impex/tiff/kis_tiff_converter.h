/*
 *  SPDX-FileCopyrightText: 2005-2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_TIFF_CONVERTER_H_
#define _KIS_TIFF_CONVERTER_H_

#include <cstdint>
#include <stdio.h>
#include <tiffio.h>

#include <QVector>

#include <KisImportExportErrorCode.h>
#include <kis_annotation.h>
#include <kis_global.h>
#include <kis_types.h>

class KoColorSpace;
class KoColorTransformation;
class KisDocument;
class KisTiffPsdLayerRecord;
class KisTiffPsdResourceRecord;
class QBuffer;

struct KisTIFFOptions {
    quint16 compressionType = 0;
    quint16 predictor = 1;
    bool alpha = true;
    bool saveAsPhotoshop = true;
    bool flatten = true;
    quint16 jpegQuality = 80;
    quint16 deflateCompress = 6;
    quint16 pixarLogCompress = 6;
    bool saveProfile = true;

    KisPropertiesConfigurationSP toProperties() const;
    void fromProperties(KisPropertiesConfigurationSP cfg);
};

struct KisTiffBasicInfo {
    uint32_t width;
    uint32_t height;
    float xres;
    float yres;
    uint16_t depth;
    uint16_t sampletype;
    uint16_t nbchannels;
    uint16_t color_type;
    uint16_t *sampleinfo = nullptr;
    uint16_t extrasamplescount = 0;
    const KoColorSpace *cs = nullptr;
    QPair<QString, QString> colorSpaceIdTag;
    KoColorTransformation *transform = nullptr;
    uint8_t dstDepth;
};

class KisTIFFConverter : public QObject
{
    Q_OBJECT
public:
    KisTIFFConverter(KisDocument *doc);
    ~KisTIFFConverter() override;
public:
    KisImportExportErrorCode buildImage(const QString &filename);
    KisImportExportErrorCode buildFile(const QString &filename, KisImageSP layer, KisTIFFOptions);
    /** Retrieve the constructed image
    */
    KisImageSP image();
public Q_SLOTS:
    virtual void cancel();
private:
    KisImportExportErrorCode decode(const QString &filename);

    KisImportExportErrorCode readTIFFDirectory(TIFF* image);

    /**
     * Imports the image from the PSD descriptor attached.
     * If this function is invoked, readTIFFDirectory will only
     * parse the first image descriptor.
     */
    KisImportExportErrorCode readImageFromPsd(const KisTiffPsdLayerRecord &photoshopLayerRecord,
                                              KisTiffPsdResourceRecord &photoshopImageResourceRecord,
                                              QBuffer &photoshopLayerData,
                                              const KisTiffBasicInfo &basicInfo);

    /**
     * Imports the image from the TIFF descriptor.
     */
    KisImportExportErrorCode readImageFromTiff(TIFF *image, KisTiffBasicInfo &basicInfo);

private:
    KisImageSP m_image;
    KisDocument *m_doc;
    bool m_stop;
    bool m_photoshopBlockParsed;
};

#endif
