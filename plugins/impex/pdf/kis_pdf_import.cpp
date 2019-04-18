/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#include "kis_pdf_import.h"

// poppler's headers
#include <poppler-qt5.h>

// Qt's headers
#include <QFile>
#include <QImage>
#include <QRadioButton>
#include <QApplication>
#include <QFileInfo>

// KDE's headers
#include <kis_debug.h>
#include <kis_paint_device.h>
#include <KoDialog.h>
#include <kpluginfactory.h>
#include <kpassworddialog.h>

// calligra's headers
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoProgressUpdater.h>
#include <KoUpdater.h>

// krita's headers
#include <KisDocument.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <kis_transaction.h>

// plugins's headers
#include "kis_pdf_import_widget.h"

K_PLUGIN_FACTORY_WITH_JSON(PDFImportFactory, "krita_pdf_import.json",
                           registerPlugin<KisPDFImport>();)

KisPDFImport::KisPDFImport(QObject *parent, const QVariantList &)
    : KisImportExportFilter(parent)
{
}

KisPDFImport::~KisPDFImport()
{
}

ImportExport::ErrorCode KisPDFImport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP /*configuration*/)
{
    Poppler::Document* pdoc = Poppler::Document::loadFromData(io->readAll());

    if (!pdoc) {
        dbgFile << "Error when reading the PDF";
        return ImportExport::ErrorWhileReading;
    }

    pdoc->setRenderHint(Poppler::Document::Antialiasing, true);
    pdoc->setRenderHint(Poppler::Document::TextAntialiasing, true);

    while (pdoc->isLocked()) {
        KPasswordDialog dlg(0);
        dlg.setPrompt(i18n("A password is required to read that pdf"));
        dlg.setWindowTitle(i18n("A password is required to read that pdf"));
        if (dlg.exec() != QDialog::Accepted) {
            dbgFile << "Password canceled";
            return ImportExport::Cancelled;
        } else
            pdoc->unlock(dlg.password().toLocal8Bit(), dlg.password().toLocal8Bit());
    }

    KoDialog* kdb = new KoDialog(0);
    kdb->setCaption(i18n("PDF Import Options"));
    kdb->setModal(false);

    KisPDFImportWidget* wdg = new KisPDFImportWidget(pdoc, kdb);
    kdb->setMainWidget(wdg);
    QApplication::restoreOverrideCursor();
    if (kdb->exec() == QDialog::Rejected) {
        delete pdoc;
        delete kdb;
        return ImportExport::Cancelled;
    }

    // Create the krita image
    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();
    int width = wdg->intWidth->value();
    int height = wdg->intHeight->value();
    KisImageSP image = new KisImage(document->createUndoStore(), width, height, cs, "built image");
    image->setResolution(wdg->intResolution->value() / 72.0, wdg->intResolution->value() / 72.0);

    // create a layer
    QList<int> pages = wdg->pages();
    for (QList<int>::const_iterator it = pages.constBegin(); it != pages.constEnd(); ++it) {
        KisPaintLayer* layer = new KisPaintLayer(image.data(),
                i18n("Page %1", *it + 1),
                quint8_MAX);


        Poppler::Page* page = pdoc->page(*it);

        QImage img = page->renderToImage(wdg->intResolution->value(), wdg->intResolution->value(), 0, 0, width, height);
        layer->paintDevice()->convertFromQImage(img, 0, 0, 0);

        delete page;
        image->addNode(layer, image->rootLayer(), 0);
        setProgress(qreal(*it + 1) * 100 / pages.count());
    }

    document->setCurrentImage(image);

    delete pdoc;
    delete kdb;
    return ImportExport::OK;
}

#include "kis_pdf_import.moc"
