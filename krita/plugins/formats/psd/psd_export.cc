/*
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "psd_export.h"

#include <QCheckBox>
#include <QSlider>

#include <kapplication.h>
#include <kdialog.h>
#include <kpluginfactory.h>
#include <kmessagebox.h>

#include <KoFilterManager.h>
#include <KoFilterChain.h>
#include <KoColorSpaceConstants.h>

#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_paint_layer.h>
#include <kis_paint_device.h>

#include "psd_saver.h"

class KisExternalLayer;

K_PLUGIN_FACTORY(ExportFactory, registerPlugin<psdExport>();)
K_EXPORT_PLUGIN(ExportFactory("calligrafilters"))

psdExport::psdExport(QObject *parent, const QVariantList &) : KoFilter(parent)
{
}

psdExport::~psdExport()
{
}

KoFilter::ConversionStatus psdExport::convert(const QByteArray& from, const QByteArray& to)
{
    dbgFile <<"PSD export! From:" << from <<", To:" << to <<"";

    if (from != "application/x-krita")
        return KoFilter::NotImplemented;

    KisDoc2 *input = dynamic_cast<KisDoc2*>(m_chain->inputDocument());
    QString filename = m_chain->outputFile();

    if (!input)
        return KoFilter::NoDocumentCreated;


    if (input->image()->width() > 30000 || input->image()->height() > 30000) {
        if (!m_chain->manager()->getBatchMode()) {
            KMessageBox::error(0, i18n("Unable to save to the Photoshop format.\n"
                                       "The Photoshop format only supports images that are smaller than 30000x3000 pixels."),
                               "Photoshop Export Error");
        }
        return KoFilter::InvalidFormat;
    }

    if (filename.isEmpty()) return KoFilter::FileNotFound;

    KUrl url;
    url.setPath(filename);

    PSDSaver kpc(input);
    KisImageBuilder_Result res;

    if ((res = kpc.buildFile(url)) == KisImageBuilder_RESULT_OK) {
        dbgFile <<"success !";
        return KoFilter::OK;
    }
    dbgFile <<" Result =" << res;
    return KoFilter::InternalError;
}

#include <psd_export.moc>

