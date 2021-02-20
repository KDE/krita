/*
 *  SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef _PSD_LOADER_H_
#define _PSD_LOADER_H_

#include <stdio.h>

#include <QObject>

#include <QFileInfo>

#include "kis_types.h"
#include <KisImportExportErrorCode.h>
class KisDocument;

class PSDLoader : public QObject {

    Q_OBJECT

public:

    PSDLoader(KisDocument *doc);
    ~PSDLoader() override;

    KisImportExportErrorCode buildImage(QIODevice *io);

    KisImageSP image();

public Q_SLOTS:

    virtual void cancel();

private:

    KisImportExportErrorCode decode(QIODevice *io);

private:

    KisImageSP m_image;
    KisDocument *m_doc;
    bool m_stop;
};

#endif
