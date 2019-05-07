/*
 *  Copyright (c) 2016 Laszlo Fazekas <mneko@freemail.hu>
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

#include "kis_csv_import.h"

#include <kpluginfactory.h>

#include <QDebug>
#include <QFileInfo>

#include <KisImportExportManager.h>

#include <KisDocument.h>
#include <kis_image.h>

#include "csv_loader.h"

K_PLUGIN_FACTORY_WITH_JSON(CSVImportFactory, "krita_csv_import.json", registerPlugin<KisCSVImport>();)

KisCSVImport::KisCSVImport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KisCSVImport::~KisCSVImport()
{
}

KisImportExportErrorCode KisCSVImport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP /*configuration*/)
{
    CSVLoader ib(document, batchMode());
    return ib.buildAnimation(io, filename());
}

#include <kis_csv_import.moc>

