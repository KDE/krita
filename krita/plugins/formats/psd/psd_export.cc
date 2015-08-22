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
#include <QMessageBox>

#include <kpluginfactory.h>
#include <kurl.h>

#include <KisImportExportManager.h>
#include <KisFilterChain.h>
#include <KoColorSpaceConstants.h>

#include <KisDocument.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_paint_layer.h>
#include <kis_paint_device.h>

#include "psd_saver.h"

class KisExternalLayer;

K_PLUGIN_FACTORY_WITH_JSON(ExportFactory, "krita_psd_export.json", registerPlugin<psdExport>();)

bool checkHomogenity(KisNodeSP root, const KoColorSpace* cs)
{
    bool res = true;
    KisNodeSP child = root->firstChild();
    while (child) {
        if (child->childCount() > 0) {
            res = checkHomogenity(child, cs);
            if (res == false) {
                break;
            }
        }
        KisLayer *layer = dynamic_cast<KisLayer*>(child.data());
        if (layer) {
            if (layer->colorSpace() != cs) {
                res = false;
                break;
            }
        }
        child = child->nextSibling();
    }
    return res;
}

psdExport::psdExport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

psdExport::~psdExport()
{
}

KisImportExportFilter::ConversionStatus psdExport::convert(const QByteArray& from, const QByteArray& to)
{
    dbgFile <<"PSD export! From:" << from <<", To:" << to <<"";

    if (from != "application/x-krita")
        return KisImportExportFilter::NotImplemented;

    KisDocument *input = m_chain->inputDocument();
    QString filename = m_chain->outputFile();

    if (!input)
        return KisImportExportFilter::NoDocumentCreated;


    if (input->image()->width() > 30000 || input->image()->height() > 30000) {
        if (!m_chain->manager()->getBatchMode()) {
            QMessageBox::critical(0,
                                  i18nc("@title:window", "Photoshop Export Error"),
                                  i18n("Unable to save to the Photoshop format.\n"
                                       "The Photoshop format only supports images that are smaller than 30000x3000 pixels."));
        }
        return KisImportExportFilter::InvalidFormat;
    }


    if (!checkHomogenity(input->image()->rootLayer(), input->image()->colorSpace())) {
        if (!m_chain->manager()->getBatchMode()) {
            QMessageBox::critical(0,
                                  i18nc("@title:window", "Photoshop Export Error"),
                                  i18n("Unable to save to the Photoshop format.\n"
                                       "The Photoshop format only supports images where all layers have the same colorspace as the image."));
        }
        return KisImportExportFilter::InvalidFormat;
    }

    qApp->processEvents(); // For vector layers to be updated
    input->image()->waitForDone();

    if (filename.isEmpty()) return KisImportExportFilter::FileNotFound;

    KUrl url;
    url.setPath(filename);

    PSDSaver kpc(input);
    KisImageBuilder_Result res;

    if ((res = kpc.buildFile(url)) == KisImageBuilder_RESULT_OK) {
        dbgFile <<"success !";
        return KisImportExportFilter::OK;
    }
    dbgFile <<" Result =" << res;
    return KisImportExportFilter::InternalError;
}

#include <psd_export.moc>

