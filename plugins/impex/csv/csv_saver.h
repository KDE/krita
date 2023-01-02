/*
 *  SPDX-FileCopyrightText: 2016 Laszlo Fazekas <mneko@freemail.hu>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef CSV_SAVER_H_
#define CSV_SAVER_H_

#include <QObject>

#include <KisImportExportErrorCode.h>
#include <kis_types.h>

class CSVLayerRecord;
class KisDocument;
class QIODevice;

class CSVSaver : public QObject {

    Q_OBJECT

public:
    CSVSaver(KisDocument* doc, bool batchMode);
    ~CSVSaver() override;

    KisImportExportErrorCode buildAnimation(QIODevice *io);
    KisImageSP image();

private:
    KisImportExportErrorCode encode(QIODevice *io);
    KisImportExportErrorCode getLayer(CSVLayerRecord* , KisDocument* , KisKeyframeSP, const QString &, int, int);
    void createTempImage(KisDocument* );
    QString convertToBlending(const QString &);

private Q_SLOTS:
    void cancel();

private:
    KisImageSP m_image;
    KisDocument* m_doc;
    bool m_batchMode;
    bool m_stop;
};

#endif
