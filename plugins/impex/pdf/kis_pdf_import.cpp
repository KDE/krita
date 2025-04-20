
/*
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_pdf_import.h"

// poppler's headers
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#include <poppler-qt5.h>
#else
#include <poppler-qt6.h>
#endif

// Qt's headers
#include <QFile>
#include <QImage>
#include <QRadioButton>
#include <QApplication>

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
#include <kis_cursor_override_hijacker.h>

// plugins's headers
#include "kis_pdf_import_widget.h"
#include <KisImportExportErrorCode.h>

K_PLUGIN_FACTORY_WITH_JSON(PDFImportFactory, "krita_pdf_import.json",
                           registerPlugin<KisPDFImport>();)

KisPDFImport::KisPDFImport(QObject *parent, const QVariantList &)
    : KisImportExportFilter(parent)
{
}

KisPDFImport::~KisPDFImport()
{
}

KisImportExportErrorCode KisPDFImport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP /*configuration*/)
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    Poppler::Document* pdoc = Poppler::Document::loadFromData(io->readAll());
#else
    std::unique_ptr<Poppler::Document> pdoc = Poppler::Document::loadFromData(io->readAll());
#endif
    if (!pdoc) {
        dbgFile << "Error when reading the PDF";
        return ImportExportCodes::ErrorWhileReading;
    }

    pdoc->setRenderHint(Poppler::Document::Antialiasing, true);
    pdoc->setRenderHint(Poppler::Document::TextAntialiasing, true);

    while (pdoc->isLocked()) {
        KPasswordDialog dlg(0);
        dlg.setPrompt(i18n("A password is required to read that pdf"));
        dlg.setWindowTitle(i18n("A password is required to read that pdf"));
        if (dlg.exec() != QDialog::Accepted) {
            dbgFile << "Password canceled";
            return ImportExportCodes::Cancelled;
        } else
            pdoc->unlock(dlg.password().toLocal8Bit(), dlg.password().toLocal8Bit());
    }

    KoDialog* kdb = new KoDialog(qApp->activeWindow());
    kdb->setCaption(i18n("PDF Import Options"));
    kdb->setModal(false);

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    KisPDFImportWidget* wdg = new KisPDFImportWidget(pdoc, kdb);
#else
    KisPDFImportWidget* wdg = new KisPDFImportWidget(pdoc.get(), kdb);
#endif

    kdb->setMainWidget(wdg);

    {
        KisCursorOverrideHijacker cursorHijacker;

        if (kdb->exec() == QDialog::Rejected) {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            delete pdoc;
#endif
            delete kdb;
            return ImportExportCodes::Cancelled;
        }
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

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        Poppler::Page* page = pdoc->page(*it);
#else
        std::unique_ptr<Poppler::Page> page = pdoc->page(*it);
#endif

        QImage img = page->renderToImage(wdg->intResolution->value(), wdg->intResolution->value(), 0, 0, width, height);
        layer->paintDevice()->convertFromQImage(img, 0, 0, 0);

        image->addNode(layer, image->rootLayer(), 0);
        setProgress(qreal(*it + 1) * 100 / pages.count());
    }

    document->setCurrentImage(image);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    delete pdoc;
#endif
    delete kdb;
    return ImportExportCodes::OK;
}

#include "kis_pdf_import.moc"
