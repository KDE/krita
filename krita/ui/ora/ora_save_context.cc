/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "ora_save_context.h"

#include <QDomDocument>

#include <KoStore.h>
#include <KoStoreDevice.h>

#include <kis_paint_layer.h>
#include <kis_paint_device.h>
#include <kis_image.h>

#include <kis_meta_data_store.h>

#include "kis_png_converter.h"

OraSaveContext::OraSaveContext(KoStore* _store) : m_id(0), m_store(_store)
{

}

QString OraSaveContext::saveDeviceData(KisPaintDeviceSP dev, KisMetaData::Store* metaData, KisImageWSP image)
{
    QString filename = QString("data/layer%1.png").arg(m_id++);

    if (m_store->open(filename)) {
        KoStoreDevice io(m_store);
        if (!io.open(QIODevice::WriteOnly)) {
            dbgFile << "Could not open for writing:" << filename;
            return "";
        }
        KisPNGConverter pngconv(0);
        vKisAnnotationSP_it annotIt = 0;

        KisMetaData::Store* store = new KisMetaData::Store(*metaData);
        bool success = pngconv.buildFile(&io, image, dev, annotIt, annotIt, KisPNGOptions(), store);
        if (success != KisImageBuilder_RESULT_OK) {
            dbgFile << "Saving PNG failed:" << filename;
            delete store;
            return "";
        }
        delete store;
        io.close();
        if (!m_store->close()) {
            return "";
        }
    } else {
        dbgFile << "Opening of data file failed :" << filename;
        return "";
    }

    return filename;
}


void OraSaveContext::saveStack(const QDomDocument& doc)
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
