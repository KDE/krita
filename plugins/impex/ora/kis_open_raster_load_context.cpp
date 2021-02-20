/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_open_raster_load_context.h"

#include <QDomDocument>

#include <KoStore.h>
#include <KoStoreDevice.h>

#include <kis_image.h>
#include <kis_paint_device.h>
#include "kis_png_converter.h"

KisOpenRasterLoadContext::KisOpenRasterLoadContext(KoStore* _store)
    : m_store(_store)
{
}

KisImageSP KisOpenRasterLoadContext::loadDeviceData(const QString & filename)
{
    if (m_store->open(filename)) {
        KoStoreDevice io(m_store);
        if (!io.open(QIODevice::ReadOnly)) {
            dbgFile << "Could not open for reading:" << filename;
            return 0;
        }
        KisPNGConverter pngConv(0);
        pngConv.buildImage(&io);
        io.close();
        m_store->close();

        return pngConv.image();

    }
    return 0;
}

QDomDocument KisOpenRasterLoadContext::loadStack()
{
    m_store->open("stack.xml");
    KoStoreDevice io(m_store);
    QDomDocument doc;
    doc.setContent(&io, false);
    io.close();
    m_store->close();
    return doc;
}
