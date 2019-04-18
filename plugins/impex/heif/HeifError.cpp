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


ImportExport::ErrorCode setHeifError(KisDocument* document,
                                                     heif::Error error)
{
  switch (error.get_code()) {
  case heif_error_Ok:
        return ImportExport::ErrorCodeID::OK;

  case heif_error_Input_does_not_exist:
    // this should never happen because we do not read from file names
    KIS_ASSERT_RECOVER_RETURN_VALUE(true, ImportExport::ErrorCodeID::InternalError);
    return ImportExport::ErrorCodeID::InternalError;

  case heif_error_Invalid_input:
  case heif_error_Decoder_plugin_error:
    return ImportExport::ErrorCodeID::FileFormatIncorrect;

  case heif_error_Unsupported_filetype:
  case heif_error_Unsupported_feature:
    return ImportExport::ErrorCodeID::FormatFeaturesUnsupported;

  case heif_error_Usage_error:
  case heif_error_Encoder_plugin_error:
    // this should never happen if we use libheif in the correct way
      KIS_ASSERT_RECOVER_RETURN_VALUE(true, ImportExport::ErrorCodeID::InternalError);
      return ImportExport::ErrorCodeID::InternalError;

  case heif_error_Memory_allocation_error:
    document->setErrorMessage(i18n("Could not allocate memory."));
    return ImportExport::ErrorCodeID::InsufficientMemory;

  case heif_error_Encoding_error:
    document->setErrorMessage(i18n("Could not encode or write image."));
    return ImportExport::ErrorCodeID::NoAccessToWrite;

  default:
    // we only get here when we forgot to handle an error ID
    document->setErrorMessage(i18n("Unknown error."));
    return ImportExport::ErrorCodeID::Failure;
  }
}
