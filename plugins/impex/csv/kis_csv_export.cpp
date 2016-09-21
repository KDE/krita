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

#include "kis_csv_export.h"

#include <QCheckBox>
#include <QSlider>
#include <QMessageBox>

#include <kpluginfactory.h>
#include <QFileInfo>
#include <QApplication>

#include <KisImportExportManager.h>
#include <KoColorSpaceConstants.h>

#include <KisDocument.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_paint_layer.h>
#include <kis_paint_device.h>

#include "csv_saver.h"

K_PLUGIN_FACTORY_WITH_JSON(KisCSVExportFactory, "krita_csv_export.json", registerPlugin<KisCSVExport>();)

bool checkHomogenity(KisNodeSP root)
{
    bool res = true;
    KisNodeSP child = root->firstChild();

    while (child) {
            if (child->childCount() > 0) {
                res = false;
                break;
            }
            child = child->nextSibling();
    }
    return res;
}

KisCSVExport::KisCSVExport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KisCSVExport::~KisCSVExport()
{
}

KisImportExportFilter::ConversionStatus KisCSVExport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP /*configuration*/)
{
    if (!checkHomogenity(document->image()->rootLayer())) {
        if (!batchMode()) {
            QMessageBox::critical(0,
                                  i18nc("@title:window", "CSV Export Error"),
                                  i18n("Unable to save to the CSV format.\n"
                                       "The CSV format not supports layer groups or masked layers."));
        }
        return KisImportExportFilter::InvalidFormat;
    }


    CSVSaver kpc(document, batchMode());
    KisImageBuilder_Result res;

    if ((res = kpc.buildAnimation(io)) == KisImageBuilder_RESULT_OK) {
        dbgFile <<"success!";
        return KisImportExportFilter::OK;
    }
    dbgFile <<" Result =" << res;

    if (res == KisImageBuilder_RESULT_CANCEL)
        return KisImportExportFilter::ProgressCancelled;

    return KisImportExportFilter::InternalError;
}

#include "kis_csv_export.moc"
