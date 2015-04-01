/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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

#include "exr_import.h"

#include <kpluginfactory.h>
#include <kurl.h>

#include <KisFilterChain.h>
#include <KisImportExportManager.h>

#include <KisDocument.h>
#include <kis_image.h>

#include "exr_converter.h"

K_PLUGIN_FACTORY_WITH_JSON(ImportFactory, "krita_exr_import.json", registerPlugin<exrImport>();)

exrImport::exrImport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

exrImport::~exrImport()
{
}

KisImportExportFilter::ConversionStatus exrImport::convert(const QByteArray&, const QByteArray& to)
{
    dbgFile << "Importing using EXRImport!";

    if (to != "application/x-krita")
        return KisImportExportFilter::BadMimeType;

    KisDocument * doc = m_chain->outputDocument();

    if (!doc)
        return KisImportExportFilter::NoDocumentCreated;

    QString filename = m_chain->inputFile();

    doc->prepareForImport();

    if (!filename.isEmpty()) {

        KUrl url(filename);

        if (url.isEmpty())
            return KisImportExportFilter::FileNotFound;

        exrConverter ib(doc, !m_chain->manager()->getBatchMode());


        switch (ib.buildImage(url)) {
        case KisImageBuilder_RESULT_UNSUPPORTED:
            doc->setErrorMessage(i18n("Krita does support this type of EXR file."));
            return KisImportExportFilter::NotImplemented;

        case KisImageBuilder_RESULT_INVALID_ARG:
            doc->setErrorMessage(i18n("This is not an EXR file."));
            return KisImportExportFilter::BadMimeType;

        case KisImageBuilder_RESULT_NO_URI:
        case KisImageBuilder_RESULT_NOT_LOCAL:
            doc->setErrorMessage(i18n("The EXR file does not exist."));
            return KisImportExportFilter::FileNotFound;

        case KisImageBuilder_RESULT_BAD_FETCH:
        case KisImageBuilder_RESULT_EMPTY:
            doc->setErrorMessage(i18n("The EXR is corrupted."));
            return KisImportExportFilter::ParsingError;

        case KisImageBuilder_RESULT_FAILURE:
            doc->setErrorMessage(i18n("Krita could not create a new image."));
            return KisImportExportFilter::InternalError;

        case KisImageBuilder_RESULT_OK:
            Q_ASSERT(ib.image());
            doc -> setCurrentImage(ib.image());
            return KisImportExportFilter::OK;
        default:
            break;
        }

    }
    return KisImportExportFilter::StorageCreationError;
}

#include <exr_import.moc>

