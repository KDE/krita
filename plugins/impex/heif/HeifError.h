/*
 *  Copyright (c) 2018 Dirk Farin <farin@struktur.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

#ifndef HEIF_ERROR_H_
#define HEIF_ERROR_H_

#include <KisDocument.h>

#include "libheif/heif_cxx.h"
#include <KisImportExportErrorCode.h>


KisImportExportErrorCode setHeifError(KisDocument* document,
                                                     heif::Error error);

#endif
