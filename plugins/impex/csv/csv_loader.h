/*
 *  SPDX-FileCopyrightText: 2016 Laszlo Fazekas <mneko@freemail.hu>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef CSV_LOADER_H_
#define CSV_LOADER_H_

#include <QObject>
#include <QFileInfo>

#include "kis_image.h"
#include "kritaui_export.h"
#include <KisImportExportErrorCode.h>
class KisDocument;

#include "csv_layer_record.h"

class CSVLoader : public QObject {

    Q_OBJECT

public:
    CSVLoader(KisDocument* doc, bool batchMode);
    ~CSVLoader() override;

    KisImportExportErrorCode buildAnimation(QIODevice *io, const QString &filename);

    KisImageSP image();

private:
    KisImportExportErrorCode decode(QIODevice *io, const QString &filename);
    KisImportExportErrorCode setLayer(CSVLayerRecord* , KisDocument* ,const QString &);
    KisImportExportErrorCode createNewImage(int, int, float, const QString &);
    QString convertBlending(const QString &);
    QString validPath(const QString &, const QString &);

private Q_SLOTS:
    void cancel();

private:
    KisImageSP m_image;
    KisDocument* m_doc;
    bool m_batchMode;
    bool m_stop;
};

#endif
