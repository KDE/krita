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

#ifndef HEIF_IMPORT_H_
#define HEIF_IMPORT_H_

#include <QVariant>

#include <KisImportExportFilter.h>

class HeifImport : public KisImportExportFilter
{
    Q_OBJECT
public:
    HeifImport(QObject *parent, const QVariantList &);
    ~HeifImport() override;
    bool supportsIO() const override { return true; }
    KisImportExportErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = 0) override;
};

#endif
