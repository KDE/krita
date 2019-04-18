/*
 *  Copyright (c) 2013 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _QML_EXPORT_H_
#define _QML_EXPORT_H_

#include <QVariant>

#include <KisImportExportFilter.h>

class QMLExport : public KisImportExportFilter
{
    Q_OBJECT
public:
    QMLExport(QObject *parent, const QVariantList &);
    ~QMLExport() override;
public:
    ImportExport::ErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = 0) override;
    void initializeCapabilities() override;
};

#endif
