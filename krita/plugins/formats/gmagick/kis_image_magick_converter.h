/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
#ifndef KIS_IMAGE_MAGICK_CONVERTER_H_
#define KIS_IMAGE_MAGICK_CONVERTER_H_

#include <qobject.h>
#include <QVector>

#include <kio/job.h>

#include <kis_global.h>

#include <kis_types.h>

class QString;
class KUrl;
class KisDoc2;
class KisNameServer;
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



/**
 * Build a KisImage representation of an image file.
 */
class KisImageMagickConverter : public QObject
{
    Q_OBJECT

public:
    KisImageMagickConverter(KisDoc2 *doc, KisUndoAdapter *adapter);
    virtual ~KisImageMagickConverter();

public slots:
    virtual void cancel();

public:
    KisImageBuilder_Result buildImage(const KUrl& uri);
    KisImageBuilder_Result buildFile(const KUrl& uri, KisPaintLayerSP layer, vKisAnnotationSP_it annotationsStart, vKisAnnotationSP_it annotationsEnd);
    KisImageWSP image();

public:
    static QString readFilters();
    static QString writeFilters();

private slots:
    void ioData(KIO::Job *job, const QByteArray& data);
    void ioResult(KIO::Job *job);
    void ioTotalSize(KIO::Job *job, KIO::filesize_t size);

private:
    KisImageMagickConverter(const KisImageMagickConverter&);
    KisImageMagickConverter& operator=(const KisImageMagickConverter&);
    void init(KisDoc2 *doc, KisUndoAdapter *adapter);
    KisImageBuilder_Result decode(const KUrl& uri, bool isBlob);

private:
    KisImageWSP m_img;
    KisDoc2 *m_doc;
    KisUndoAdapter *m_adapter;
    QVector<quint8> m_data;
    KIO::TransferJob *m_job;
    KIO::filesize_t m_size;
    bool m_stop;
};

#endif // KIS_IMAGE_MAGICK_CONVERTER_H_

