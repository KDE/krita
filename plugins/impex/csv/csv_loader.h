/*
 *  Copyright (c) 2016 Laszlo Fazekas <mneko@freemail.hu>
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

#ifndef CSV_LOADER_H_
#define CSV_LOADER_H_

#include <QObject>
#include <QUrl>

#include "kis_image.h"
#include "kritaui_export.h"
class KisDocument;

#include "csv_layer_record.h"

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

class CSVLoader : public QObject {

    Q_OBJECT

public:
    CSVLoader(KisDocument* doc);
    virtual ~CSVLoader();

    KisImageBuilder_Result buildAnimation(const QUrl &, const QString &);

    KisImageWSP image();

private:
    KisImageBuilder_Result decode(const QUrl &, const QString &);
    KisImageBuilder_Result setLayer(CSVLayerRecord* , KisDocument* ,const QString &);
    KisImageBuilder_Result createNewImage(int, int, float, const QString &);
    QString convertBlending(const QString &);

private:
    KisImageWSP m_image; 
    KisDocument* m_doc;
    bool m_stop;
};

#endif
