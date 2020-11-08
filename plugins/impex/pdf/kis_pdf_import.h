/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#ifndef KIS_PDF_IMPORT_H
#define KIS_PDF_IMPORT_H

#include <QVariant>

#include <KisImportExportFilter.h>

class KisPDFImport : public KisImportExportFilter
{
    Q_OBJECT
public:
    KisPDFImport(QObject *parent, const QVariantList &);
    virtual ~KisPDFImport();
public:
    virtual KisImportExportErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = 0);
};

#endif
