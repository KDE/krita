/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#include "ora_export.h"

#include <QCheckBox>
#include <QSlider>

#include <kapplication.h>
#include <kdialog.h>
#include <kgenericfactory.h>

#include <KoFilterChain.h>

#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_paint_layer.h>

#include "ora_converter.h"

class KisExternalLayer;

typedef KGenericFactory<OraExport> ExportFactory;
K_EXPORT_COMPONENT_FACTORY(libkritaoraexport, ExportFactory("kofficefilters"))

OraExport::OraExport(QObject *parent, const QStringList&) : KoFilter(parent)
{
}

OraExport::~OraExport()
{
}

KoFilter::ConversionStatus OraExport::convert(const QByteArray& from, const QByteArray& to)
{
    dbgFile << "ORA export! From:" << from << ", To:" << to << "";

    if (from != "application/x-krita")
        return KoFilter::NotImplemented;

    KisDoc2 *output = dynamic_cast<KisDoc2*>(m_chain->inputDocument());
    QString filename = m_chain->outputFile();

    if (!output)
        return KoFilter::CreationError;


    if (filename.isEmpty()) return KoFilter::FileNotFound;

    KUrl url;
    url.setPath(filename);

    KisImageWSP image = output->image();
    Q_CHECK_PTR(image);

    OraConverter kpc(output, output->undoAdapter());

    KisImageBuilder_Result res;

    if ((res = kpc.buildFile(url, image)) == KisImageBuilder_RESULT_OK) {
        dbgFile << "success !";
        return KoFilter::OK;
    }
    dbgFile << " Result =" << res;
    return KoFilter::InternalError;
}

#include <ora_export.moc>

