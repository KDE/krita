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

#ifndef _KIS_JPEG_CONVERTER_H_
#define _KIS_JPEG_CONVERTER_H_

#include <stdio.h>

extern "C" {
#include <jpeglib.h>
}

#include <QVector>

#include <kio/job.h>



#include "kis_types.h"
#include "kis_annotation.h"
class KisDoc2;
class KisUndoAdapter;
class KisExifInfo;

namespace KisMetaData
{
class Filter;
}

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

struct KisJPEGOptions {
    int quality;
    bool progressive;
    bool optimize;
    int smooth;
    bool baseLineJPEG;
    int subsampling;
    bool exif;
    bool iptc;
    bool xmp;
    QList<const KisMetaData::Filter*> filters;
};

namespace KisMetaData
{
class Store;
}

class KisJPEGConverter : public QObject
{
    Q_OBJECT
public:
    KisJPEGConverter(KisDoc2 *doc, KisUndoAdapter *adapter);
    virtual ~KisJPEGConverter();
public:
    KisImageBuilder_Result buildImage(const KUrl& uri);
    KisImageBuilder_Result buildFile(const KUrl& uri, KisPaintLayerSP layer, vKisAnnotationSP_it annotationsStart, vKisAnnotationSP_it annotationsEnd, KisJPEGOptions options, KisMetaData::Store* metaData);
    /** Retrieve the constructed image
    */
    KisImageWSP image();
public slots:
    virtual void cancel();
private:
    KisImageBuilder_Result decode(const KUrl& uri);
private:
    KisImageWSP m_image;
    KisDoc2 *m_doc;
    KisUndoAdapter *m_adapter;
    bool m_stop;
    KIO::TransferJob *m_job;
};

#endif
