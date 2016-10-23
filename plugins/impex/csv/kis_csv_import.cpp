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

#include <KisFilterChain.h>
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

KisImportExportFilter::ConversionStatus KisCSVImport::convert(const QByteArray&, const QByteArray& to, KisPropertiesConfigurationSP configuration)
{
    Q_UNUSED(configuration);
    dbgFile << "Importing using CSVImport!";

    if (to != "application/x-krita")
        return KisImportExportFilter::BadMimeType;

    KisDocument * doc = outputDocument();

    if (!doc)
        return KisImportExportFilter::NoDocumentCreated;

    QString filename = inputFile();

    doc -> prepareForImport();

    if (!filename.isEmpty() && QFileInfo(filename).exists()) {

        CSVLoader ib(doc, getBatchMode());

        KisImageBuilder_Result result = ib.buildAnimation(filename);

        switch (result) {
        case KisImageBuilder_RESULT_UNSUPPORTED:
            return KisImportExportFilter::NotImplemented;
        case KisImageBuilder_RESULT_INVALID_ARG:
            return KisImportExportFilter::BadMimeType;
        case KisImageBuilder_RESULT_NO_URI:
        case KisImageBuilder_RESULT_NOT_EXIST:
        case KisImageBuilder_RESULT_NOT_LOCAL:
            qDebug() << "ib returned KisImageBuilder_RESULT_NOT_LOCAL";
            return KisImportExportFilter::FileNotFound;
        case KisImageBuilder_RESULT_BAD_FETCH:
        case KisImageBuilder_RESULT_EMPTY:
            return KisImportExportFilter::ParsingError;
        case KisImageBuilder_RESULT_FAILURE:
            return KisImportExportFilter::InternalError;
        case KisImageBuilder_RESULT_CANCEL:
            return KisImportExportFilter::ProgressCancelled;
        case KisImageBuilder_RESULT_OK:
            doc -> setCurrentImage( ib.image());
            return KisImportExportFilter::OK;
        default:
            return KisImportExportFilter::StorageCreationError;
        }
    }
    return KisImportExportFilter::StorageCreationError;
}

#include <kis_csv_import.moc>

