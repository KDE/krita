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

#include <QFileInfo>

#include <kpluginfactory.h>

#include <KisFilterChain.h>

#include <KisDocument.h>
#include <kis_image.h>

#include <KisViewManager.h>

#include "kis_tiff_converter.h"

K_PLUGIN_FACTORY_WITH_JSON(TIFFImportFactory, "krita_tiff_import.json", registerPlugin<KisTIFFImport>();)

KisTIFFImport::KisTIFFImport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KisTIFFImport::~KisTIFFImport()
{
}

KisImportExportFilter::ConversionStatus KisTIFFImport::convert(KisDocument *document, QIODevice */*io*/,  KisPropertiesConfigurationSP /*configuration*/)
{
    KisTIFFConverter tiffConverter(document);

    switch (tiffConverter.buildImage(inputFile())) {
    case KisImageBuilder_RESULT_UNSUPPORTED:
        return KisImportExportFilter::NotImplemented;
    case KisImageBuilder_RESULT_INVALID_ARG:
        return KisImportExportFilter::BadMimeType;
    case KisImageBuilder_RESULT_NO_URI:
    case KisImageBuilder_RESULT_NOT_LOCAL:
        return KisImportExportFilter::FileNotFound;
    case KisImageBuilder_RESULT_BAD_FETCH:
    case KisImageBuilder_RESULT_EMPTY:
        return KisImportExportFilter::ParsingError;
    case KisImageBuilder_RESULT_FAILURE:
        return KisImportExportFilter::InternalError;
    case KisImageBuilder_RESULT_UNSUPPORTED_COLORSPACE:
        return KisImportExportFilter::WrongFormat;
    case KisImageBuilder_RESULT_OK:
        document -> setCurrentImage(tiffConverter.image());
        return KisImportExportFilter::OK;
    default:
        break;
    }
    return KisImportExportFilter::InternalError;
}

#include <kis_tiff_import.moc>

