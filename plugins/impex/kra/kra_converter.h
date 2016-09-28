/*
 * Copyright (C) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _KRA_CONVERTER_H_
#define _KRA_CONVERTER_H_

#include <stdio.h>

#include <QObject>
#include <QDomDocument>

#include <KoStore.h>
#include <kis_png_converter.h>
#include <kis_types.h>
#include <kis_kra_saver.h>
class KisDocument;

class KraConverter : public QObject
{
    Q_OBJECT

public:

    KraConverter(KisDocument *doc);
    virtual ~KraConverter();

    KisImageBuilder_Result buildImage(QIODevice *io);
    KisImageBuilder_Result buildFile(QIODevice *io, KisImageWSP image, vKisNodeSP activeNodes);
    /**
     * Retrieve the constructed image
     */
    KisImageWSP image();
    vKisNodeSP activeNodes();

public Q_SLOTS:

    virtual void cancel();

private:

    bool saveRootDocuments(KoStore *store);
    bool saveToStream(QIODevice *dev);
    QDomDocument createDomDocument();
    bool savePreview(KoStore *store);

    KisDocument *m_doc {0};
    KisImageWSP m_image;

    vKisNodeSP m_activeNodes;
    bool m_stop {false};

    KoStore *m_store {0};
    KisKraSaver *m_kraSaver {0};
};

#endif
