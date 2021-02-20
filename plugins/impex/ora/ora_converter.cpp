/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "ora_converter.h"

#include <QApplication>

#include <QFileInfo>

#include <KoStore.h>
#include <KoStoreDevice.h>
#include <KoColorSpaceRegistry.h>
#include <KisDocument.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_open_raster_stack_load_visitor.h>
#include <kis_open_raster_stack_save_visitor.h>
#include <kis_paint_layer.h>
#include "kis_png_converter.h"
#include "kis_open_raster_load_context.h"
#include "kis_open_raster_save_context.h"

OraConverter::OraConverter(KisDocument *doc)
    : m_doc(doc)
    , m_stop(false)
{
}

OraConverter::~OraConverter()
{
}

KisImportExportErrorCode OraConverter::buildImage(QIODevice *io)
{
    KoStore* store = KoStore::createStore(io, KoStore::Read, "image/openraster", KoStore::Zip);
    if (!store) {
        delete store;
        return ImportExportCodes::FileFormatIncorrect;
    }

    KisOpenRasterLoadContext olc(store);
    KisOpenRasterStackLoadVisitor orslv(m_doc->createUndoStore(), &olc);
    orslv.loadImage();
    m_image = orslv.image();

    qDebug() << "m_image" << m_image;

    if (!m_image) {
        delete store;
        return ImportExportCodes::ErrorWhileReading;
    }

    m_activeNodes = orslv.activeNodes();
    delete store;

    return ImportExportCodes::OK;
}

KisImageSP OraConverter::image()
{
    return m_image;
}

vKisNodeSP OraConverter::activeNodes()
{
    return m_activeNodes;
}

KisImportExportErrorCode OraConverter::buildFile(QIODevice *io, KisImageSP image, vKisNodeSP activeNodes)
{

    // Open file for writing
    KoStore* store = KoStore::createStore(io, KoStore::Write, "image/openraster", KoStore::Zip);
    if (!store) {
        delete store;
        return ImportExportCodes::Failure;
    }

    KisOpenRasterSaveContext osc(store);
    KisOpenRasterStackSaveVisitor orssv(&osc, activeNodes);

    image->rootLayer()->accept(orssv);

    if (store->open("Thumbnails/thumbnail.png")) {
        QSize previewSize = image->bounds().size();
        previewSize.scale(QSize(256,256), Qt::KeepAspectRatio);

        QImage preview = image->convertToQImage(previewSize, 0);

        KoStoreDevice io(store);
        if (io.open(QIODevice::WriteOnly)) {
            preview.save(&io, "PNG");
        }
        io.close();
        store->close();
    }

    KisPaintDeviceSP dev = image->projection();
    KisPNGConverter::saveDeviceToStore("mergedimage.png", image->bounds(), image->xRes(), image->yRes(), dev, store);

    delete store;
    return ImportExportCodes::OK;
}


void OraConverter::cancel()
{
    m_stop = true;
}


