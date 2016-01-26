/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _JP2_CONVERTER_H_
#define _JP2_CONVERTER_H_

#include <stdio.h>

#include <QObject>

#include "kis_types.h"
class KisDocument;

class QUrl;

struct JP2ConvertOptions {
  int rate;
  int numberresolution;
};

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

class jp2Converter : public QObject
{
    Q_OBJECT
private:
    enum {
        J2K_CFMT = 0,
        JP2_CFMT = 1,
        JPT_CFMT = 2
    };
public:
    jp2Converter(KisDocument *doc);
    virtual ~jp2Converter();
public:
    KisImageBuilder_Result buildImage(const QUrl &uri);
    KisImageBuilder_Result buildFile(const QUrl &uri, KisPaintLayerSP layer, const JP2ConvertOptions& options);
    /**
     * Retrieve the constructed image
     */
    KisImageWSP getImage();
private:
    KisImageBuilder_Result decode(const QUrl &uri);
public Q_SLOTS:
    virtual void cancel();
private:
    int getFileFormat(const QUrl &uri) const;
private:
    KisImageWSP m_image;
    KisDocument *m_doc;
    bool m_stop;
};

#endif
