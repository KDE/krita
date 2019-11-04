/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "jp2_export.h"

#include <QCheckBox>
#include <QSlider>
#include <QApplication>

#include <KoDialog.h>
#include <kpluginfactory.h>
#include <QFileInfo>

#include <KoColorSpaceConstants.h>
#include <KisFilterChain.h>
#include <KisImportExportManager.h>

#include <kis_properties_configuration.h>
#include <kis_config.h>
#include <KisDocument.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_paint_layer.h>
#include <kis_paint_device.h>

#include "jp2_converter.h"

#include "ui_kis_wdg_options_jp2.h"

class KisExternalLayer;

K_PLUGIN_FACTORY_WITH_JSON(ExportFactory, "krita_jp2_export.json", registerPlugin<jp2Export>();)

jp2Export::jp2Export(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

jp2Export::~jp2Export()
{
}

KisImportExportFilter::ConversionStatus jp2Export::convert(const QByteArray& from, const QByteArray& to)
{
    dbgFile << "JP2 export! From:" << from << ", To:" << to << "";

    if (from != "application/x-krita")
        return KisImportExportFilter::NotImplemented;

    KisDocument *input = inputDocument();
    QString filename = outputFile();

    if (!input)
        return KisImportExportFilter::NoDocumentCreated;

    KisImageWSP image = input->image();
    Q_CHECK_PTR(image);

    if (filename.isEmpty()) return KisImportExportFilter::FileNotFound;

    KoDialog* kdb = new KoDialog(0);
    kdb->setWindowTitle(i18n("JPEG 2000 Export Options"));
    kdb->setButtons(KoDialog::Ok | KoDialog::Cancel);

    Ui::WdgOptionsJP2 optionsJP2;

    QWidget* wdg = new QWidget(kdb);
    optionsJP2.setupUi(wdg);

    QString filterConfig = KisConfig().exportConfiguration("JP2");
    KisPropertiesConfiguration cfg;
    cfg.fromXML(filterConfig);
    optionsJP2.numberResolutions->setValue(cfg.getInt("number_resolutions", 6));
    optionsJP2.qualityLevel->setValue(cfg.getInt("quality", 100));

    kdb->setMainWidget(wdg);
    QApplication::restoreOverrideCursor();

    if (!getBatchMode()) {
        if (kdb->exec() == QDialog::Rejected) {
            return KisImportExportFilter::UserCancelled;
        }
    }

    JP2ConvertOptions options;
    options.numberresolution = optionsJP2.numberResolutions->value();
    cfg.setProperty("number_resolutions", options.numberresolution);
    options.rate = optionsJP2.qualityLevel->value();
    cfg.setProperty("quality", options.rate);

    KisConfig().setExportConfiguration("JP2", cfg);


    // the image must be locked at the higher levels
    KIS_SAFE_ASSERT_RECOVER_NOOP(input->image()->locked());
    jp2Converter kpc(input);

    KisPaintDeviceSP pd = new KisPaintDevice(*image->projection());
    KisPaintLayerSP l = new KisPaintLayer(image, "projection", OPACITY_OPAQUE_U8, pd);

    KisImageBuilder_Result res;

    if ((res = kpc.buildFile(filename, l, options)) == KisImageBuilder_RESULT_OK) {
        dbgFile << "success !";
        return KisImportExportFilter::OK;
    }
    dbgFile << " Result =" << res;
    return KisImportExportFilter::InternalError;
}

#include <jp2_export.moc>

