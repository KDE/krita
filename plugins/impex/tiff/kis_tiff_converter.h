/*
 *  SPDX-FileCopyrightText: 2005-2006 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
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

#include <config-tiff.h>

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
    bool saveAsPhotoshop = false;
    quint16 psdCompressionType = 0;
    bool flatten = true;
    quint16 jpegQuality = 80;
    quint16 deflateCompress = 6;
    quint16 pixarLogCompress = 6;
    bool saveProfile = true;

    KisPropertiesConfigurationSP toProperties() const;
    void fromProperties(KisPropertiesConfigurationSP cfg);
};

class KisTIFFConverter : public QObject
{
    Q_OBJECT
public:
    KisTIFFConverter(KisDocument *doc);
    ~KisTIFFConverter() override;

public:
    KisImportExportErrorCode buildFile(const QString &filename, KisImageSP layer, KisTIFFOptions);
    /** Retrieve the constructed image
    */
    KisImageSP image();
public Q_SLOTS:
    virtual void cancel();

private:
    KisImageSP m_image;
    KisDocument *m_doc;
    bool m_stop;
};

#endif
