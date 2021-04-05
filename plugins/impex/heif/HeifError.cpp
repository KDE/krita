/*
 *  SPDX-FileCopyrightText: 2018 Dirk Farin <farin@struktur.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "HeifError.h"


KisImportExportErrorCode setHeifError(KisDocument* document,
                                                     heif::Error error)
{
  switch (error.get_code()) {
  case heif_error_Ok:
        return ImportExportCodes::OK;

  case heif_error_Input_does_not_exist:
    // this should never happen because we do not read from file names
    KIS_ASSERT_RECOVER_RETURN_VALUE(true, ImportExportCodes::InternalError);
    return ImportExportCodes::InternalError;

  case heif_error_Invalid_input:
  case heif_error_Decoder_plugin_error:
    return ImportExportCodes::FileFormatIncorrect;

  case heif_error_Unsupported_filetype:
  case heif_error_Unsupported_feature:
    return ImportExportCodes::FormatFeaturesUnsupported;

  case heif_error_Usage_error:
  case heif_error_Encoder_plugin_error:
    // this should never happen if we use libheif in the correct way
      KIS_ASSERT_RECOVER_RETURN_VALUE(true, ImportExportCodes::InternalError);
      return ImportExportCodes::InternalError;

  case heif_error_Memory_allocation_error:
    document->setErrorMessage(i18n("Could not allocate memory."));
    return ImportExportCodes::InsufficientMemory;

  case heif_error_Encoding_error:
    document->setErrorMessage(i18n("Could not encode or write image."));
    return ImportExportCodes::NoAccessToWrite;

  case heif_error_Color_profile_does_not_exist:
      document->setErrorMessage(i18n("Color profile was not found in the document."));
      return ImportExportCodes::OK;
  default:
    // we only get here when we forgot to handle an error ID
    document->setErrorMessage(QString(i18n("Unknown error: %1")).arg(QString::fromStdString(error.get_message())));
    return ImportExportCodes::Failure;
  }
}
