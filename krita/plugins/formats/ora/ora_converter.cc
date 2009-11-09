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

#include "ora_converter.h"

#include <kapplication.h>

#include <kio/netaccess.h>
#include <kio/deletejob.h>

#include <KoStore.h>

#include <kis_doc2.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_open_raster_stack_load_visitor.h>
#include <kis_open_raster_stack_save_visitor.h>
#include <kis_paint_layer.h>
#include <kis_undo_adapter.h>

#include "ora_load_context.h"
#include "ora_save_context.h"

OraConverter::OraConverter(KisDoc2 *doc, KisUndoAdapter *adapter)
{
    m_doc = doc;
    m_adapter = adapter;
    m_job = 0;
    m_stop = false;
}

OraConverter::~OraConverter()
{
}


KisImageBuilder_Result OraConverter::buildImage(const KUrl& uri)
{
    if (uri.isEmpty())
        return KisImageBuilder_RESULT_NO_URI;

    if (!KIO::NetAccess::exists(uri, KIO::NetAccess::SourceSide, QApplication::activeWindow())) {
        return KisImageBuilder_RESULT_NOT_EXIST;
    }

    // We're not set up to handle asynchronous loading at the moment.
    QString tmpFile;

    KoStore* store = KoStore::createStore(QApplication::activeWindow(), uri, KoStore::Read, "odr", KoStore::Zip);
    if (!store) {
        return KisImageBuilder_RESULT_FAILURE;
    }

    OraLoadContext olc(store);
    KisOpenRasterStackLoadVisitor orslv(m_doc, &olc);
    orslv.loadImage();
    m_img = orslv.image();
    return KisImageBuilder_RESULT_OK;

}


KisImageWSP OraConverter::image()
{
    return m_img;
}


KisImageBuilder_Result OraConverter::buildFile(const KUrl& uri, KisImageWSP image)
{

    if (uri.isEmpty())
        return KisImageBuilder_RESULT_NO_URI;

    if (!uri.isLocalFile())
        return KisImageBuilder_RESULT_NOT_LOCAL;
    // Open file for writing
    KoStore* store = KoStore::createStore(QApplication::activeWindow(), uri, KoStore::Write, "odr", KoStore::Zip);
    if (!store) {
        return KisImageBuilder_RESULT_FAILURE;
    }

    OraSaveContext osc(store);
    KisOpenRasterStackSaveVisitor orssv(&osc);

    image->rootLayer()->accept(orssv);

    delete store;
    return KisImageBuilder_RESULT_OK;
}


void OraConverter::cancel()
{
    m_stop = true;
}

#include "ora_converter.moc"

