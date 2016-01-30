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

#include "qml_export.h"

#include <QCheckBox>
#include <QSlider>

#include <kpluginfactory.h>
#include <QUrl>
#include <QApplication>

#include <KisFilterChain.h>

#include <KisDocument.h>
#include <kis_image.h>

#include "qml_converter.h"

K_PLUGIN_FACTORY_WITH_JSON(ExportFactory, "krita_qml_export.json", registerPlugin<QMLExport>();)

QMLExport::QMLExport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

QMLExport::~QMLExport()
{
}

KisImportExportFilter::ConversionStatus QMLExport::convert(const QByteArray& from, const QByteArray& to)
{
    Q_UNUSED(to);

    if (from != "application/x-krita")
        return KisImportExportFilter::NotImplemented;

    KisDocument *input = m_chain->inputDocument();
    QString filename = m_chain->outputFile();

    dbgKrita << "input " << input;
    if (!input) {
        return KisImportExportFilter::NoDocumentCreated;
    }

    dbgKrita << "filename " << input;

    if (filename.isEmpty()) {
        return KisImportExportFilter::FileNotFound;
    }

    QUrl url = QUrl::fromLocalFile(filename);

    KisImageWSP image = input->image();

    qApp->processEvents(); // For vector layers to be updated
    image->waitForDone();

    Q_CHECK_PTR(image);

    QMLConverter converter;
    KisImageBuilder_Result result = converter.buildFile(url, image);
    if (result == KisImageBuilder_RESULT_OK) {
        dbgFile << "success !";
        return KisImportExportFilter::OK;
    }
    dbgFile << " Result =" << result;
    return KisImportExportFilter::InternalError;
}

#include <qml_export.moc>

