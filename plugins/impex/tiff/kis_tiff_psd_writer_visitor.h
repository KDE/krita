/*
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_TIFF_PSD_WRITER_VISITOR_H
#define _KIS_TIFF_PSD_WRITER_VISITOR_H

#include <tiffio.h>

#include <array>

#include <KisImportExportErrorCode.h>
#include <kis_types.h>

struct KisTIFFOptions;

class KisTiffPsdWriter : public QObject
{
    Q_OBJECT
public:
    KisTiffPsdWriter(TIFF *image, KisTIFFOptions *options);
    ~KisTiffPsdWriter() override;

    KisImportExportErrorCode writeImage(KisGroupLayerSP rootLayer);

private:
    inline TIFF *image()
    {
        return m_image;
    }

    bool copyDataToStrips(KisHLineConstIteratorSP it,
                          tdata_t buff,
                          uint32_t depth,
                          uint16_t sample_format,
                          uint8_t nbcolorssamples,
                          const std::array<quint8, 5> &poses);
    bool saveLayerProjection(KisLayer *);

    TIFF *m_image;
    KisTIFFOptions *m_options;
};

#endif // _KIS_TIFF_PSD_WRITER_VISITOR_H
