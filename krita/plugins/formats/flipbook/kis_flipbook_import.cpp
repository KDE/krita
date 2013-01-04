/*
 *  Copyright (c) 2011 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_flipbook_import.h"

#include <QCheckBox>
#include <QSlider>

#include <kio/netaccess.h>
#include <kio/deletejob.h>

#include <kapplication.h>
#include <kdialog.h>
#include <kpluginfactory.h>

#include <KoColorSpace.h>
#include <KoFilterChain.h>
#include <KoColorSpaceRegistry.h>

#include <kis_transaction.h>
#include <kis_paint_device.h>
#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <kis_node.h>
#include <kis_group_layer.h>
#include <kis_flipbook.h>
#include <kis_flipbook_item.h>

K_PLUGIN_FACTORY(KisFlipbookImportFactory, registerPlugin<KisFlipbookImport>();)
K_EXPORT_PLUGIN(KisFlipbookImportFactory("calligrafilters"))

KisFlipbookImport::KisFlipbookImport(QObject *parent, const QVariantList &)
    : KoFilter(parent)
{
}

KisFlipbookImport::~KisFlipbookImport()
{
}

KoFilter::ConversionStatus KisFlipbookImport::convert(const QByteArray& from, const QByteArray& to)
{
    qDebug() << "Flipbook import! From:" << from << ", To:" << to << 0;

    if (!(from == "application/x-krita-flipbook"))
        return KoFilter::NotImplemented;

    if (to != "application/x-krita")
        return KoFilter::BadMimeType;

    KisDoc2 *doc = dynamic_cast<KisDoc2*>(m_chain->outputDocument());

    if (!doc)
        return KoFilter::NoDocumentCreated;

    QString filename = m_chain->inputFile();

    if (filename.isEmpty() || !QFile::exists(filename))
        return KoFilter::FileNotFound;

    KisFlipbook *flipbook = new KisFlipbook();
    flipbook->load(filename);
    if (flipbook->rowCount() > 0) {
        // XXX: load the last loaded current file
    }

    KisPart2 *part = dynamic_cast<KisPart2*>(doc->documentPart());
    if (!part) {
        return KoFilter::NoDocumentCreated;
    }

    part->setFlipbook(flipbook);

    KisFlipbookItem *item = static_cast<KisFlipbookItem*>(flipbook->item(0));
    if (!QFile::exists(item->filename()))
        return KoFilter::FileNotFound;

    KisDoc2 *tmpDoc = item->document();
    doc->setCurrentImage(tmpDoc->image());
    doc->setUrl(tmpDoc->url());

    return KoFilter::OK;

}

