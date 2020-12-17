/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _ORA_CONVERTER_H_
#define _ORA_CONVERTER_H_

#include <stdio.h>

#include <QObject>

#include "kis_png_converter.h"
#include "kis_types.h"

class KisDocument;

class OraConverter : public QObject
{
    Q_OBJECT
public:
    OraConverter(KisDocument *doc);
    ~OraConverter() override;
public:
    KisImportExportErrorCode buildImage(QIODevice *io);
    KisImportExportErrorCode buildFile(QIODevice *io, KisImageSP image, vKisNodeSP activeNodes);
    /**
     * Retrieve the constructed image
     */
    KisImageSP image();
    vKisNodeSP activeNodes();
public Q_SLOTS:
    virtual void cancel();
private:
    KisImageSP m_image;
    KisDocument *m_doc;
    vKisNodeSP m_activeNodes;
    bool m_stop;
};

#endif
