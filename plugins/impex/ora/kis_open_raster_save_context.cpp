/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_open_raster_save_context.h"

#include <QDomDocument>

#include <KoStore.h>
#include <KoStoreDevice.h>
#include <KoColorSpaceRegistry.h>
#include <kundo2command.h>
#include <kis_paint_layer.h>
#include <kis_paint_device.h>
#include <kis_image.h>

#include <kis_meta_data_store.h>

#include "kis_png_converter.h"

KisOpenRasterSaveContext::KisOpenRasterSaveContext(KoStore* store)
    : m_id(0)
    , m_store(store)
{
}

QString KisOpenRasterSaveContext::saveDeviceData(KisPaintDeviceSP dev, KisMetaData::Store* metaData, const QRect &imageRect, const qreal xRes, const qreal yRes)
{
    QString filename = QString("data/layer%1.png").arg(m_id++);
    if (KisPNGConverter::saveDeviceToStore(filename, imageRect, xRes, yRes, dev, m_store, metaData)) {
        return filename;
    }
    return "";
}


void KisOpenRasterSaveContext::saveStack(const QDomDocument& doc)
{
    if (m_store->open("stack.xml")) {
        KoStoreDevice io(m_store);
        io.write(doc.toByteArray());
        io.close();
        m_store->close();
    } else {
        dbgFile << "Opening of the stack.xml file failed :";
    }
}
