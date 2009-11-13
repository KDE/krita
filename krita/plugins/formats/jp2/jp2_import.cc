/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "jp2_import.h"

#include <kgenericfactory.h>

#include <KoFilterChain.h>

#include <kis_doc2.h>
#include <kis_image.h>

#include "jp2_converter.h"

typedef KGenericFactory<jp2Import> ImportFactory;
K_EXPORT_COMPONENT_FACTORY(libkritajp2import, ImportFactory("kofficefilters"))

jp2Import::jp2Import(QObject *parent, const QStringList&) : KoFilter(parent)
{
}

jp2Import::~jp2Import()
{
}

KoFilter::ConversionStatus jp2Import::convert(const QByteArray&, const QByteArray& to)
{
    dbgFile << "Importing using JP2Import!";

    if (to != "application/x-krita")
        return KoFilter::BadMimeType;

    KisDoc2 * doc = dynamic_cast<KisDoc2*>(m_chain->outputDocument());

    if (!doc)
        return KoFilter::CreationError;

    QString filename = m_chain -> inputFile();

    doc->prepareForImport();

    if (!filename.isEmpty()) {

        KUrl url;
        url.setPath(filename);

        if (url.isEmpty())
            return KoFilter::FileNotFound;

        jp2Converter ib(doc, doc -> undoAdapter());


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

#include <jp2_import.moc>

