/*
 *  SPDX-FileCopyrightText: 2018 Dirk Farin <farin@struktur.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef HEIF_ERROR_H_
#define HEIF_ERROR_H_

#include <KisDocument.h>

#include "libheif/heif_cxx.h"
#include <KisImportExportErrorCode.h>


KisImportExportErrorCode setHeifError(KisDocument* document,
                                                     heif::Error error);

#endif
