/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
