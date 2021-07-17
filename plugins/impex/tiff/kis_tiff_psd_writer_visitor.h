/*
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_TIFF_PSD_WRITER_VISITOR_H
#define _KIS_TIFF_PSD_WRITER_VISITOR_H

#include <kis_types.h>
#include <tiffio.h>

#include "kis_tiff_converter.h"

class KisTiffPsdWriter : public QObject
{
    Q_OBJECT
public:
    KisTiffPsdWriter(TIFF *image, KisTIFFOptions *options);
    ~KisTiffPsdWriter() override;

public:
    KisImportExportErrorCode writeImage(KisGroupLayerSP rootLayer);

private:
    inline TIFF *image()
    {
        return m_image;
    }

    bool copyDataToStrips(KisHLineConstIteratorSP it, tdata_t buff, uint32_t depth, uint16_t sample_format, uint8_t nbcolorssamples, quint8 *poses);
    bool saveLayerProjection(KisLayer *);

private:
    TIFF *m_image;
    KisTIFFOptions *m_options;
};

#endif // _KIS_TIFF_PSD_WRITER_VISITOR_H
