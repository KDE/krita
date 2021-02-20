/*
 *  SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef _PSD_CONVERTER_H_
#define _PSD_CONVERTER_H_

#include <stdio.h>

#include <QObject>

#include <QFileInfo>

#include "kis_types.h"
#include <KisImportExportErrorCode.h>


// max number of pixels in one dimension of psd file
extern const int MAX_PSD_SIZE;


class KisDocument;

class PSDSaver : public QObject {

    Q_OBJECT

public:

    PSDSaver(KisDocument *doc);
    ~PSDSaver() override;

public:

    KisImportExportErrorCode buildFile(QIODevice *io);

    KisImageSP image();

public Q_SLOTS:

    virtual void cancel();

private:
    KisImageSP m_image;
    KisDocument *m_doc;
    bool m_stop;
};

#endif
