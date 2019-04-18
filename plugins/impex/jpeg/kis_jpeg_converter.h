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

#include <QColor>
#include <QVector>

#include "kis_types.h"
#include "kis_annotation.h"
#include <KisImportExportErrorCodes.h>
class KisDocument;

namespace KisMetaData
{
class Filter;
}

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
    QColor transparencyFillColor;
    bool forceSRGB;
    bool saveProfile;
    bool storeDocumentMetaData; //this is for getting the metadata from the document info.
    bool storeAuthor; //this is for storing author data from the document info.
};

namespace KisMetaData
{
class Store;
}

class KisJPEGConverter : public QObject
{
    Q_OBJECT
public:
    KisJPEGConverter(KisDocument *doc, bool batchMode = false);
    ~KisJPEGConverter() override;
public:
    ImportExport::ErrorCode buildImage(QIODevice *io);
    ImportExport::ErrorCode buildFile(QIODevice *io, KisPaintLayerSP layer, KisJPEGOptions options, KisMetaData::Store* metaData);
    /** Retrieve the constructed image
    */
    KisImageSP image();
public Q_SLOTS:
    virtual void cancel();
private:
    ImportExport::ErrorCode decode(QIODevice *io);
private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif
