/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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

#include "exr_export.h"

#include <QCheckBox>
#include <QSlider>
#include <QApplication>

#include <KoDialog.h>
#include <kpluginfactory.h>
#include <QFileInfo>

#include <KisFilterChain.h>
#include <KoColorSpaceConstants.h>
#include <KisImportExportManager.h>

#include <kis_properties_configuration.h>
#include <kis_config.h>
#include <KisDocument.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_paint_device.h>
#include <kis_paint_layer.h>

#include "exr_converter.h"

#include "ui_exr_export_widget.h"

class KisExternalLayer;

K_PLUGIN_FACTORY_WITH_JSON(ExportFactory, "krita_exr_export.json", registerPlugin<exrExport>();)

exrExport::exrExport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

exrExport::~exrExport()
{
}

KisPropertiesConfigurationSP exrExport::defaultConfiguration(const QByteArray &/*from*/, const QByteArray &/*to*/) const
{
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    cfg->setProperty("flatten", false);
    return cfg;
}

KisPropertiesConfigurationSP exrExport::lastSavedConfiguration(const QByteArray &/*from*/, const QByteArray &/*to*/) const
{
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    QString filterConfig = KisConfig().exportConfiguration("EXR");
    cfg->fromXML(filterConfig);
    return cfg;
}

KisConfigWidget *exrExport::createConfigurationWidget(QWidget *parent, const QByteArray &/*from*/, const QByteArray &/*to*/) const
{
    return 0;
}

KisImportExportFilter::ConversionStatus exrExport::convert(const QByteArray& from, const QByteArray& to, KisPropertiesConfigurationSP configuration)
{
    dbgFile << "EXR export! From:" << from << ", To:" << to << "";

    if (from != "application/x-krita")
        return KisImportExportFilter::NotImplemented;

    KisDocument *input = inputDocument();
    if (!input)
        return KisImportExportFilter::NoDocumentCreated;
    KisImageWSP image = input->image();
    Q_CHECK_PTR(image);

    KoDialog dialog;
    dialog.setWindowTitle(i18n("OpenEXR Export Options"));
    dialog.setButtons(KoDialog::Ok | KoDialog::Cancel);
    Ui::ExrExportWidget widget;
    QWidget *page = new QWidget(&dialog);
    widget.setupUi(page);
    dialog.setMainWidget(page);
    dialog.resize(dialog.minimumSize());

    KisPropertiesConfigurationSP cfg = lastSavedConfiguration(from, to);

    widget.flatten->setChecked(cfg->getBool("flatten", false));

    if (!getBatchMode() ) {
        QApplication::restoreOverrideCursor();
        if (dialog.exec() == QDialog::Rejected) {
            return KisImportExportFilter::UserCancelled;
        }
    }

    cfg->setProperty("flatten", widget.flatten->isChecked());
    KisConfig().setExportConfiguration("EXR", *cfg.data());

    QString filename = outputFile();
    if (filename.isEmpty()) return KisImportExportFilter::FileNotFound;

    exrConverter kpc(input, !getBatchMode());

    KisImageBuilder_Result res;

    if (widget.flatten->isChecked()) {
        // the image must be locked at the higher levels
        KIS_SAFE_ASSERT_RECOVER_NOOP(input->image()->locked());

        KisPaintDeviceSP pd = new KisPaintDevice(*image->projection());
        KisPaintLayerSP l = new KisPaintLayer(image, "projection", OPACITY_OPAQUE_U8, pd);

        res = kpc.buildFile(filename, l);
    }
    else {
        // the image must be locked at the higher levels
        KIS_SAFE_ASSERT_RECOVER_NOOP(input->image()->locked());

        res = kpc.buildFile(filename, image->rootLayer());
    }

    dbgFile << " Result =" << res;
    switch (res) {
    case KisImageBuilder_RESULT_INVALID_ARG:
        input->setErrorMessage(i18n("This layer cannot be saved to EXR."));
        return KisImportExportFilter::WrongFormat;

    case KisImageBuilder_RESULT_EMPTY:
        input->setErrorMessage(i18n("The layer does not have an image associated with it."));
        return KisImportExportFilter::WrongFormat;

    case KisImageBuilder_RESULT_NO_URI:
        input->setErrorMessage(i18n("The filename is empty."));
        return KisImportExportFilter::CreationError;

    case KisImageBuilder_RESULT_NOT_LOCAL:
        input->setErrorMessage(i18n("EXR images cannot be saved remotely."));
        return KisImportExportFilter::InternalError;

    case KisImageBuilder_RESULT_UNSUPPORTED_COLORSPACE:
        input->setErrorMessage(i18n("Colorspace not supported: EXR images must be 16 or 32 bits floating point RGB."));
        return KisImportExportFilter::WrongFormat;

    case KisImageBuilder_RESULT_OK:
        return KisImportExportFilter::OK;
    default:
        break;
    }

    input->setErrorMessage(i18n("Internal Error"));
    return KisImportExportFilter::InternalError;

}

#include <exr_export.moc>

