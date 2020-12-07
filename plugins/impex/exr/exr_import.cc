/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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

#include "exr_import.h"

#include <kpluginfactory.h>
#include <QFileInfo>

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

KisImportExportErrorCode exrImport::convert(KisDocument *document, QIODevice */*io*/,  KisPropertiesConfigurationSP /*configuration*/)
{
    EXRConverter ib(document, !batchMode());
    KisImportExportErrorCode result = ib.buildImage(filename());
    if (result.isOk()) {
        document->setCurrentImage(ib.image());
    }
    return result;
}

#include <exr_import.moc>

