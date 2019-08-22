/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "jp2_import.h"

#include <kpluginfactory.h>
#include <QFileInfo>

#include <KisFilterChain.h>

#include <KisDocument.h>
#include <kis_image.h>

#include "jp2_converter.h"

K_PLUGIN_FACTORY_WITH_JSON(ImportFactory, "krita_jp2_import.json", registerPlugin<jp2Import>();)

jp2Import::jp2Import(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

jp2Import::~jp2Import()
{
}

KisImportExportFilter::ConversionStatus jp2Import::convert(const QByteArray&, const QByteArray& to)
{
    dbgFile << "Importing using JP2Import!";

    if (to != "application/x-krita")
        return KisImportExportFilter::BadMimeType;

    KisDocument * doc = outputDocument();

    if (!doc)
        return KisImportExportFilter::NoDocumentCreated;

    QString filename = inputFile();

    doc->prepareForImport();

    if (!filename.isEmpty()) {

        jp2Converter ib(doc);

        switch (ib.buildImage(filename)) {
        case KisImageBuilder_RESULT_UNSUPPORTED:
            return KisImportExportFilter::NotImplemented;
            break;
        case KisImageBuilder_RESULT_INVALID_ARG:
            return KisImportExportFilter::BadMimeType;
            break;
        case KisImageBuilder_RESULT_NO_URI:
        case KisImageBuilder_RESULT_NOT_LOCAL:
            return KisImportExportFilter::FileNotFound;
            break;
        case KisImageBuilder_RESULT_BAD_FETCH:
        case KisImageBuilder_RESULT_EMPTY:
            return KisImportExportFilter::ParsingError;
            break;
        case KisImageBuilder_RESULT_FAILURE:
            return KisImportExportFilter::InternalError;
            break;
        case KisImageBuilder_RESULT_OK:
            doc -> setCurrentImage(ib.getImage());
            return KisImportExportFilter::OK;
        default:
            break;
        }

    }
    return KisImportExportFilter::StorageCreationError;
}

#include <jp2_import.moc>

