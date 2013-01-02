/*
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
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

#include <kapplication.h>
#include <kdialog.h>
#include <kpluginfactory.h>
#include <kmessagebox.h>

#include <KoFilterChain.h>

#include <kis_doc2.h>
#include <kis_image.h>

#include "qml_converter.h"

K_PLUGIN_FACTORY(ExportFactory, registerPlugin<QMLExport>();)
K_EXPORT_PLUGIN(ExportFactory("calligrafilters"))

QMLExport::QMLExport(QObject *parent, const QVariantList &) : KoFilter(parent)
{
}

QMLExport::~QMLExport()
{
}

KoFilter::ConversionStatus QMLExport::convert(const QByteArray& from, const QByteArray& to)
{
    if (from != "application/x-krita")
        return KoFilter::NotImplemented;

    KisDoc2 *input = dynamic_cast<KisDoc2*>(m_chain->inputDocument());
    QString filename = m_chain->outputFile();

    kDebug() << "input " << input;
    if (!input) {
        return KoFilter::NoDocumentCreated;
    }

    kDebug() << "filename " << input;

    if (filename.isEmpty()) {
        return KoFilter::FileNotFound;
    }

    KUrl url;
    url.setPath(filename);

    KisImageWSP image = input->image();
    Q_CHECK_PTR(image);

    QMLConverter converter;
    KisImageBuilder_Result result = converter.buildFile(url, image);
    if (result == KisImageBuilder_RESULT_OK) {
        dbgFile << "success !";
        return KoFilter::OK;
    }
    dbgFile << " Result =" << result;
    return KoFilter::InternalError;
}

#include <qml_export.moc>

