/*
 *  Copyright (c) 2018 Dirk Farin <farin@struktur.de>
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

#include "HeifError.h"


KisImportExportFilter::ConversionStatus setHeifError(KisDocument* document,
                                                     heif::Error error)
{
  switch (error.get_code()) {
  case heif_error_Ok:
        return KisImportExportFilter::OK;

  case heif_error_Input_does_not_exist:
    // this should never happen because we do not read from file names
    document->setErrorMessage(i18n("Internal error."));
    return KisImportExportFilter::InternalError;

  case heif_error_Invalid_input:
  case heif_error_Decoder_plugin_error:
    document->setErrorMessage(i18n("The HEIF file is corrupted."));
    return KisImportExportFilter::ParsingError;

  case heif_error_Unsupported_filetype:
  case heif_error_Unsupported_feature:
    document->setErrorMessage(i18n("Krita does not support this type of HEIF file."));
    return KisImportExportFilter::NotImplemented;

  case heif_error_Usage_error:
  case heif_error_Encoder_plugin_error:
    // this should never happen if we use libheif in the correct way
    document->setErrorMessage(i18n("Internal libheif API error."));
    return KisImportExportFilter::InternalError;

  case heif_error_Memory_allocation_error:
    document->setErrorMessage(i18n("Could not allocate memory."));
    return KisImportExportFilter::StorageCreationError;

  case heif_error_Encoding_error:
    document->setErrorMessage(i18n("Could not encode or write image."));
    return KisImportExportFilter::CreationError;

  default:
    // we only get here when we forgot to handle an error ID
    document->setErrorMessage(i18n("Unknown error."));
    return KisImportExportFilter::InternalError;
  }
}
