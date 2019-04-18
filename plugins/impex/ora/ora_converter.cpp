/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

ImportExport::ErrorCode OraConverter::buildImage(QIODevice *io)
{
    KoStore* store = KoStore::createStore(io, KoStore::Read, "image/openraster", KoStore::Zip);
    if (!store) {
        delete store;
        return ImportExport::ErrorCodeID::Failure;
    }

    KisOpenRasterLoadContext olc(store);
    KisOpenRasterStackLoadVisitor orslv(m_doc->createUndoStore(), &olc);
    orslv.loadImage();
    m_image = orslv.image();
    m_activeNodes = orslv.activeNodes();
    delete store;

    return ImportExport::ErrorCodeID::OK;
}

KisImageSP OraConverter::image()
{
    return m_image;
}

vKisNodeSP OraConverter::activeNodes()
{
    return m_activeNodes;
}

ImportExport::ErrorCode OraConverter::buildFile(QIODevice *io, KisImageSP image, vKisNodeSP activeNodes)
{

    // Open file for writing
    KoStore* store = KoStore::createStore(io, KoStore::Write, "image/openraster", KoStore::Zip);
    if (!store) {
        delete store;
        return ImportExport::ErrorCodeID::Failure;
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
    return ImportExport::ErrorCodeID::OK;
}


void OraConverter::cancel()
{
    m_stop = true;
}


