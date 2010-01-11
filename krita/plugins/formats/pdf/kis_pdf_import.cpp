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

#define UNSTABLE_POPPLER_QT4
#include "kis_pdf_import.h"

// poppler's headers
#include <poppler-qt4.h>

// Qt's headers
#include <qfile.h>
#include <qimage.h>
#include <qradiobutton.h>

// KDE's headers
#include <kapplication.h>
#include <kis_debug.h>
#include <kis_paint_device.h>
#include <kdialog.h>
#include <kgenericfactory.h>
#include <knuminput.h>
#include <kpassworddialog.h>

#include <kio/netaccess.h>

// koffice's headers
#include <KoFilterChain.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

// krita's headers
#include <kis_doc2.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <kis_transaction.h>

// plugins's headers
#include "kis_pdf_import_widget.h"

typedef KGenericFactory<KisPDFImport, KoFilter> PDFImportFactory;
K_EXPORT_COMPONENT_FACTORY(libkritapdfimport, PDFImportFactory("kofficefilters"))

KisPDFImport::KisPDFImport(QObject* parent, const QStringList&) : KoFilter(parent)
{
}

KisPDFImport::~KisPDFImport()
{
}

KisPDFImport::ConversionStatus KisPDFImport::convert(const QByteArray& , const QByteArray&)
{
    QString filename = m_chain -> inputFile();
    dbgFile << "Importing using PDFImport!" << filename;

    if (filename.isEmpty())
        return KoFilter::FileNotFound;


    KUrl url(filename);

    if (!KIO::NetAccess::exists(url, KIO::NetAccess::SourceSide, QApplication::activeWindow())) {
        return KoFilter::FileNotFound;
    }

    // We're not set up to handle asynchronous loading at the moment.
    QString tmpFile;
    if (KIO::NetAccess::download(url, tmpFile, QApplication::activeWindow())) {
        url.setPath(tmpFile);
    }

    Poppler::Document* pdoc = Poppler::Document::load(QFile::encodeName(url.toLocalFile()));


    if (!pdoc) {
        dbgFile << "Error when reading the PDF";
        return KoFilter::StorageCreationError;
    }


    while (pdoc->isLocked()) {
        KPasswordDialog dlg(0);
        dlg.setPrompt(i18n("A password is required to read that pdf"));
        dlg.setWindowTitle(i18n("A password is required to read that pdf"));
        if (dlg.exec() != QDialog::Accepted) {
            dbgFile << "Password canceled";
            return KoFilter::StorageCreationError;
        } else
            pdoc->unlock(dlg.password().toLocal8Bit(), dlg.password().toLocal8Bit());
    }

    KDialog* kdb = new KDialog(0);
    kdb->setCaption(i18n("PDF Import Options"));
    kdb->setModal(false);

    KisPDFImportWidget* wdg = new KisPDFImportWidget(pdoc, kdb);
    kdb->setMainWidget(wdg);
    kapp->restoreOverrideCursor();
    if (kdb->exec() == QDialog::Rejected) {
        delete pdoc;
        delete kdb;
        return KoFilter::StorageCreationError; // FIXME Cancel doesn't exist :(
    }

    // Init kis's doc
    KisDoc2 * doc = dynamic_cast<KisDoc2*>(m_chain -> outputDocument());
    if (!doc) {
        delete pdoc;
        delete kdb;
        return KoFilter::CreationError;
    }

    doc -> prepareForImport();
    // Create the krita image
    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->colorSpace(KoID("RGBA"), "");
    int width = wdg->intWidth->value();
    int height = wdg->intHeight->value();
    KisImageWSP image = new KisImage(doc->undoAdapter(), width, height, cs, "built image");
    image->lock();
    // create a layer
    QList<int> pages = wdg->pages();
    for (QList<int>::const_iterator it = pages.constBegin(); it != pages.constEnd(); ++it) {
        KisPaintLayer* layer = new KisPaintLayer(image.data(),
                i18n("Page %1", *it + 1),
                quint8_MAX);

        KisTransaction("", layer->paintDevice());

        Poppler::Page* page = pdoc->page(*it);
        for (int x = 0; x < width; x += 1000) {
            int currentWidth = (x + 1000 > width) ? (width - x) : 1000;
            for (int y = 0; y < height; y += 1000) {
                int currentHeight = (y + 1000 > height) ? (height - x) : 1000;
                layer->paintDevice()->convertFromQImage(page->renderToImage(wdg->intHorizontal->value(), wdg->intVertical->value(), x, y, currentWidth, currentHeight), "", x, y);
            }
        }
        delete page;
        image->addNode(layer, image->rootLayer(), 0);
        layer->setDirty();
    }

    doc->setCurrentImage(image);
    image->unlock();
    KIO::NetAccess::removeTempFile(tmpFile);

    delete pdoc;
    delete kdb;
    return KoFilter::OK;
}

#include "kis_pdf_import.moc"
