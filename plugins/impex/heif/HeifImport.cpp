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

#include "HeifImport.h"

#include <kpluginfactory.h>
#include <QFileInfo>

#include <KisImportExportManager.h>

#include <KisDocument.h>
#include <kis_image.h>

#include "HeifConverter.h"

K_PLUGIN_FACTORY_WITH_JSON(ImportFactory, "krita_heif_import.json", registerPlugin<HeifImport>();)

HeifImport::HeifImport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

HeifImport::~HeifImport()
{
}

KisImportExportFilter::ConversionStatus HeifImport::convert(KisDocument *document, QIODevice */*io*/,  KisPropertiesConfigurationSP /*configuration*/)
{
    HeifConverter imageBuilder(document, !batchMode());

    switch (imageBuilder.buildImage(filename())) {
    case KisImageBuilder_RESULT_UNSUPPORTED:
    case KisImageBuilder_RESULT_UNSUPPORTED_COLORSPACE:
        document->setErrorMessage(i18n("Krita does support this type of EXR file."));
        return KisImportExportFilter::NotImplemented;

    case KisImageBuilder_RESULT_INVALID_ARG:
        document->setErrorMessage(i18n("This is not a HEIF file."));
        return KisImportExportFilter::BadMimeType;

    case KisImageBuilder_RESULT_NO_URI:
    case KisImageBuilder_RESULT_NOT_LOCAL:
        document->setErrorMessage(i18n("The HEIF file does not exist."));
        return KisImportExportFilter::FileNotFound;

    case KisImageBuilder_RESULT_BAD_FETCH:
    case KisImageBuilder_RESULT_EMPTY:
        document->setErrorMessage(i18n("The HEIF file is corrupted."));
        return KisImportExportFilter::ParsingError;

    case KisImageBuilder_RESULT_FAILURE:
        document->setErrorMessage(i18n("Krita could not create a new image."));
        return KisImportExportFilter::InternalError;

    case KisImageBuilder_RESULT_OK:
        Q_ASSERT(imageBuilder.image());
        document -> setCurrentImage(imageBuilder.image());
        return KisImportExportFilter::OK;

    default:
        break;
    }

    return KisImportExportFilter::StorageCreationError;
}

#include <HeifImport.moc>

