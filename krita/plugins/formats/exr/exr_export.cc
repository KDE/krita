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

#include <kdialog.h>
#include <kpluginfactory.h>

#include <KoFilterChain.h>
#include <KoColorSpaceConstants.h>
#include <KoFilterManager.h>

#include <kis_properties_configuration.h>
#include <kis_config.h>
#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_paint_device.h>
#include <kis_paint_layer.h>

#include "exr_converter.h"

#include "ui_exr_export_widget.h"

class KisExternalLayer;

K_PLUGIN_FACTORY(ExportFactory, registerPlugin<exrExport>();)
K_EXPORT_PLUGIN(ExportFactory("calligrafilters"))

exrExport::exrExport(QObject *parent, const QVariantList &) : KoFilter(parent)
{
}

exrExport::~exrExport()
{
}

KoFilter::ConversionStatus exrExport::convert(const QByteArray& from, const QByteArray& to)
{
    dbgFile << "EXR export! From:" << from << ", To:" << to << "";

    if (from != "application/x-krita")
        return KoFilter::NotImplemented;

    KisDoc2 *input = dynamic_cast<KisDoc2*>(m_chain->inputDocument());
    if (!input)
        return KoFilter::NoDocumentCreated;
    KisImageWSP image = input->image();
    Q_CHECK_PTR(image);

    KDialog dialog;
    dialog.setWindowTitle(i18n("OpenEXR Export Options"));
    dialog.setButtons(KDialog::Ok | KDialog::Cancel);
    Ui::ExrExportWidget widget;
    QWidget *page = new QWidget(&dialog);
    widget.setupUi(page);
    dialog.setMainWidget(page);
    dialog.resize(dialog.minimumSize());

    QString filterConfig = KisConfig().exportConfiguration("EXR");
    KisPropertiesConfiguration cfg;
    cfg.fromXML(filterConfig);

    widget.flatten->setChecked(cfg.getBool("flatten", false));

    if (!m_chain->manager()->getBatchMode() ) {
        qApp->restoreOverrideCursor();
        if (dialog.exec() == QDialog::Rejected) {
            return KoFilter::UserCancelled;
        }
    }
    else {
        qApp->processEvents(); // For vector layers to be updated
    }
    image->waitForDone();

    cfg.setProperty("flatten", widget.flatten->isChecked());
    KisConfig().setExportConfiguration("EXR", cfg);

    QString filename = m_chain->outputFile();
    if (filename.isEmpty()) return KoFilter::FileNotFound;

    KUrl url;
    url.setPath(filename);

    exrConverter kpc(input, !m_chain->manager()->getBatchMode());

    KisImageBuilder_Result res;

    if (widget.flatten->isChecked()) {
        image->refreshGraph();
        image->lock();
        KisPaintDeviceSP pd = new KisPaintDevice(*image->projection());
        KisPaintLayerSP l = new KisPaintLayer(image, "projection", OPACITY_OPAQUE_U8, pd);
        image->unlock();

        res = kpc.buildFile(url, l);
    }
    else {
        image->lock();

        res = kpc.buildFile(url, image->rootLayer());
        image->unlock();

    }

    dbgFile << " Result =" << res;
    switch (res) {
    case KisImageBuilder_RESULT_INVALID_ARG:
        input->setErrorMessage(i18n("This layer cannot be saved to EXR."));
        return KoFilter::WrongFormat;

    case KisImageBuilder_RESULT_EMPTY:
        input->setErrorMessage(i18n("The layer does not have an image associated with it."));
        return KoFilter::WrongFormat;

    case KisImageBuilder_RESULT_NO_URI:
        input->setErrorMessage(i18n("The filename is empty."));
        return KoFilter::CreationError;

    case KisImageBuilder_RESULT_NOT_LOCAL:
        input->setErrorMessage(i18n("EXR images cannot be saved remotely."));
        return KoFilter::InternalError;

    case KisImageBuilder_RESULT_UNSUPPORTED_COLORSPACE:
        input->setErrorMessage(i18n("Colorspace not supported: EXR images must be 16 or 32 bits floating point RGB."));
        return KoFilter::WrongFormat;

    case KisImageBuilder_RESULT_OK:
        return KoFilter::OK;
    default:
        break;
    }

    input->setErrorMessage(i18n("Internal Error"));
    return KoFilter::InternalError;

}

#include <exr_export.moc>

