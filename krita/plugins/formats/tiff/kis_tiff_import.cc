/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
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

#include "kis_tiff_import.h"

#include <kpluginfactory.h>

#include <KoFilterChain.h>

#include <kis_doc2.h>
#include <kis_image.h>

#include <kis_view2.h>

#include "kis_tiff_converter.h"

K_PLUGIN_FACTORY(TIFFImportFactory, registerPlugin<KisTIFFImport>();)
K_EXPORT_PLUGIN(TIFFImportFactory("kofficefilters"))

KisTIFFImport::KisTIFFImport(QObject *parent, const QVariantList &) : KoFilter(parent)
{
}

KisTIFFImport::~KisTIFFImport()
{
}

KoFilter::ConversionStatus KisTIFFImport::convert(const QByteArray&, const QByteArray& to)
{
    dbgFile << "Importing using TIFFImport!";

    if (to != "application/x-krita")
        return KoFilter::BadMimeType;

    KisDoc2 * doc = dynamic_cast<KisDoc2*>(m_chain -> outputDocument());

    if (!doc)
        return KoFilter::CreationError;

    KisView2 * view = 0;

    if (!doc->views().isEmpty()) {
        view = static_cast<KisView2*>(doc->views().first());
    }

    QString filename = m_chain -> inputFile();

    doc -> prepareForImport();

    if (!filename.isEmpty()) {

        KUrl url;
        url.setPath(filename);

        if (url.isEmpty())
            return KoFilter::FileNotFound;

        KisTIFFConverter ib(doc, doc -> undoAdapter());

//        if (view != 0)
//            view -> canvasSubject() ->  progressDisplay() -> setSubject(&ib, false, true);

        switch (ib.buildImage(url)) {
        case KisImageBuilder_RESULT_UNSUPPORTED:
            return KoFilter::NotImplemented;
            break;
        case KisImageBuilder_RESULT_INVALID_ARG:
            return KoFilter::BadMimeType;
            break;
        case KisImageBuilder_RESULT_NO_URI:
        case KisImageBuilder_RESULT_NOT_LOCAL:
            return KoFilter::FileNotFound;
            break;
        case KisImageBuilder_RESULT_BAD_FETCH:
        case KisImageBuilder_RESULT_EMPTY:
            return KoFilter::ParsingError;
            break;
        case KisImageBuilder_RESULT_FAILURE:
            return KoFilter::InternalError;
            break;
        case KisImageBuilder_RESULT_OK:
            doc -> setCurrentImage(ib.image());
            return KoFilter::OK;
        default:
            break;
        }

    }
    return KoFilter::StorageCreationError;
}

#include <kis_tiff_import.moc>

