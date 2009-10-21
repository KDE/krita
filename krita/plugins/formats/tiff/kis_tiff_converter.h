/*
 *  Copyright (c) 2005-2006 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_TIFF_CONVERTER_H_
#define _KIS_TIFF_CONVERTER_H_

#include <stdio.h>
#include <tiffio.h>

#include <QVector>

#include <kio/job.h>



#include "kis_types.h"
#include "kis_global.h"
#include "kis_annotation.h"
class KisDoc2;
class KisUndoAdapter;

/**
 * Image import/export plugins can use these results to report about success or failure.
 */
enum KisImageBuilder_Result {
    KisImageBuilder_RESULT_FAILURE = -400,
    KisImageBuilder_RESULT_NOT_EXIST = -300,
    KisImageBuilder_RESULT_NOT_LOCAL = -200,
    KisImageBuilder_RESULT_BAD_FETCH = -100,
    KisImageBuilder_RESULT_INVALID_ARG = -50,
    KisImageBuilder_RESULT_OK = 0,
    KisImageBuilder_RESULT_PROGRESS = 1,
    KisImageBuilder_RESULT_EMPTY = 100,
    KisImageBuilder_RESULT_BUSY = 150,
    KisImageBuilder_RESULT_NO_URI = 200,
    KisImageBuilder_RESULT_UNSUPPORTED = 300,
    KisImageBuilder_RESULT_INTR = 400,
    KisImageBuilder_RESULT_PATH = 500,
    KisImageBuilder_RESULT_UNSUPPORTED_COLORSPACE = 600
};

struct KisTIFFOptions {
    quint16 compressionType;
    quint16 predictor;
    bool alpha;
    bool flatten;
    quint16 jpegQuality;
    quint16 deflateCompress;
    quint16 faxMode;
    quint16 pixarLogCompress;
};

class KisTIFFConverter : public QObject
{
    Q_OBJECT
public:
    KisTIFFConverter(KisDoc2 *doc, KisUndoAdapter *adapter);
    virtual ~KisTIFFConverter();
public:
    KisImageBuilder_Result buildImage(const KUrl& uri);
    KisImageBuilder_Result buildFile(const KUrl& uri, KisImageWSP layer, KisTIFFOptions);
    /** Retrieve the constructed image
    */
    KisImageWSP image();
public slots:
    virtual void cancel();
private:
    KisImageBuilder_Result decode(const KUrl& uri);
    KisImageBuilder_Result readTIFFDirectory(TIFF* image);
private:
    KisImageWSP m_img;
    KisDoc2 *m_doc;
    KisUndoAdapter *m_adapter;
    bool m_stop;
    KIO::TransferJob *m_job;
};

#endif
