/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_brush_export.h"

#include <QCheckBox>
#include <QSlider>
#include <QBuffer>

#include <KoProperties.h>
#include <KoDialog.h>
#include <kpluginfactory.h>
#include <QUrl>

#include <KisFilterChain.h>

#include <kis_paint_device.h>
#include <KisDocument.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <kis_spacing_selection_widget.h>
#include <kis_gbr_brush.h>
#include <kis_imagepipe_brush.h>
#include <kis_pipebrush_parasite.h>
#include <KisAnimatedBrushAnnotation.h>
#include <KisImportExportManager.h>

#include <ui_wdg_export_gih.h>

struct KisBrushExportOptions {
    qreal spacing;
    bool mask;
    int brushStyle;
    int selectionMode;
    QString name;
};


K_PLUGIN_FACTORY_WITH_JSON(KisBrushExportFactory, "krita_brush_export.json", registerPlugin<KisBrushExport>();)

KisBrushExport::KisBrushExport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KisBrushExport::~KisBrushExport()
{
}

KisImportExportFilter::ConversionStatus KisBrushExport::convert(const QByteArray& from, const QByteArray& to)
{
    KisDocument *input = m_chain->inputDocument();
    QString filename = m_chain->outputFile();

    if (!input)
        return KisImportExportFilter::NoDocumentCreated;

    if (filename.isEmpty()) return KisImportExportFilter::FileNotFound;

    if (from != "application/x-krita")
        return KisImportExportFilter::NotImplemented;

    KisAnnotationSP annotation = input->image()->annotation("ImagePipe Parasite");
    KisPipeBrushParasite parasite;
    if (annotation) {
        QBuffer buf(const_cast<QByteArray*>(&annotation->annotation()));
        buf.open(QBuffer::ReadOnly);
        //parasite.loadFromDevice(&buf);
        buf.close();
    }

    KisBrushExportOptions exportOptions;
    exportOptions.spacing = 1.0;
    exportOptions.name = input->image()->objectName();
    exportOptions.mask = true;
    exportOptions.selectionMode = 0;
    exportOptions.brushStyle = 0;


    if (input->image()->dynamicPropertyNames().contains("brushspacing")) {
        exportOptions.spacing = input->image()->property("brushspacing").toFloat();
    }
    KisGbrBrush *brush;

    if (!m_chain->manager()->getBatchMode()) {

        KoDialog* dlgBrushExportOptions = new KoDialog(0);
        dlgBrushExportOptions->setWindowTitle(i18n("Brush Tip Export Options"));
        dlgBrushExportOptions->setButtons(KoDialog::Ok | KoDialog::Cancel);

        Ui::WdgExportGih wdgUi;
        QWidget* wdg = new QWidget(dlgBrushExportOptions);
        wdgUi.setupUi(wdg);
        wdgUi.spacingWidget->setSpacing(false, exportOptions.spacing);
        wdgUi.nameLineEdit->setText(exportOptions.name);
        dlgBrushExportOptions->setMainWidget(wdg);


        if (to == "image/x-gimp-brush") {
            brush = new KisGbrBrush(filename);
            wdgUi.groupBox->setVisible(false);
        }
        else if (to == "image/x-gimp-brush-animated") {
            brush = new KisImagePipeBrush(filename);
            wdgUi.groupBox->setVisible(true);
        }
        else {
            delete dlgBrushExportOptions;
            return KisImportExportFilter::BadMimeType;
        }

        if (dlgBrushExportOptions->exec() == QDialog::Rejected) {
            delete dlgBrushExportOptions;
            return KisImportExportFilter::UserCancelled;
        }
        else {
            exportOptions.spacing = wdgUi.spacingWidget->spacing();
            exportOptions.name = wdgUi.brushNameLbl->text();
            exportOptions.mask = wdgUi.colorAsMask->isChecked();
            exportOptions.brushStyle = wdgUi.brushStyle->currentIndex();
            exportOptions.selectionMode = wdgUi.cmbSelectionMode->currentIndex();
            delete dlgBrushExportOptions;
        }
    }
    else {
        qApp->processEvents(); // For vector layers to be updated
    }


    input->image()->waitForDone();
    QRect rc = input->image()->bounds();
    input->image()->refreshGraph();
    input->image()->lock();

    brush->setName(exportOptions.name);
    brush->setSpacing(exportOptions.spacing);
    brush->setUseColorAsMask(exportOptions.mask);

    int w = input->image()->width();
    int h = input->image()->height();

    KisImagePipeBrush *pipeBrush = dynamic_cast<KisImagePipeBrush*>(brush);
    if (pipeBrush) {
        // Create parasite. XXX: share with KisCustomBrushWidget
        QVector< QVector<KisPaintDevice*> > devices;
        devices.push_back(QVector<KisPaintDevice*>());

        KoProperties properties;
        properties.setProperty("visible", true);
        QList<KisNodeSP> layers = input->image()->root()->childNodes(QStringList("KisLayer"), properties);
        KisNodeSP node;
        Q_FOREACH (KisNodeSP node, layers) {
            devices[0].push_back(node->projection().data());
        }


        QVector<KisParasite::SelectionMode > modes;
        switch (exportOptions.selectionMode) {
        case 0: modes.push_back(KisParasite::Constant); break;
        case 1: modes.push_back(KisParasite::Random); break;
        case 2: modes.push_back(KisParasite::Incremental); break;
        case 3: modes.push_back(KisParasite::Pressure); break;
        case 4: modes.push_back(KisParasite::Angular); break;
        default: modes.push_back(KisParasite::Incremental);
        }

        KisPipeBrushParasite parasite;

        // XXX: share code with KisImagePipeBrush, when we figure out how to support more gih features
        parasite.dim = devices.count();
        // XXX Change for multidim! :
        parasite.ncells = devices.at(0).count();
        parasite.rank[0] = parasite.ncells; // ### This can mask some bugs, be careful here in the future
        parasite.selection[0] = modes.at(0);
        // XXX needsmovement!
        parasite.setBrushesCount();
        pipeBrush->setParasite(parasite);
        pipeBrush->setDevices(devices, w, h);
    }
    else {
        QImage image = input->image()->projection()->convertToQImage(0, 0, 0, rc.width(), rc.height(), KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());
        brush->setImage(image);
    }

    brush->setWidth(w);
    brush->setHeight(h);

    input->image()->unlock();

    QFile f(filename);
    f.open(QIODevice::WriteOnly);
    brush->saveToDevice(&f);
    f.close();

    return KisImportExportFilter::OK;
}

#include "kis_brush_export.moc"

