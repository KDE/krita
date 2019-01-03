/*
 * imagesplit.cc -- Part of Krita
 *
 * Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
 * Copyright (c) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
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

#include "imagesplit.h"

#include <QStringList>
#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>

#include <klocalizedstring.h>
#include <kpluginfactory.h>

#include <KisImportExportManager.h>
#include <KoFileDialog.h>
#include <KisDocument.h>

#include <KisPart.h>
#include <kis_debug.h>
#include <kis_types.h>
#include <KisViewManager.h>
#include <kis_image.h>
#include <kis_action.h>
#include <kis_paint_layer.h>
#include <kis_painter.h>
#include <kis_paint_device.h>
#include <KisMimeDatabase.h>

#include "dlg_imagesplit.h"

K_PLUGIN_FACTORY_WITH_JSON(ImagesplitFactory, "kritaimagesplit.json", registerPlugin<Imagesplit>();)

Imagesplit::Imagesplit(QObject *parent, const QVariantList &)
    : KisActionPlugin(parent)
{
    KisAction *action  = createAction("imagesplit");
    connect(action, SIGNAL(triggered()), this, SLOT(slotImagesplit()));
}

Imagesplit::~Imagesplit()
{
}

bool Imagesplit::saveAsImage(const QRect &imgSize, const QString &mimeType, const QString &url)
{
    KisImageSP image = viewManager()->image();

    KisDocument *document = KisPart::instance()->createDocument();

    { // make sure dst is deleted before calling 'delete exportDocument',
      // since KisDocument checks that its image is properly deref()'d.
      KisImageSP dst = new KisImage(document->createUndoStore(), imgSize.width(), imgSize.height(), image->colorSpace(), image->objectName());
      dst->setResolution(image->xRes(), image->yRes());
      document->setCurrentImage(dst);

      KisPaintLayer* paintLayer = new KisPaintLayer(dst, dst->nextLayerName(), 255);
      KisPainter gc(paintLayer->paintDevice());
      gc.bitBlt(QPoint(0, 0), image->projection(), imgSize);

      dst->addNode(paintLayer, KisNodeSP(0));
      dst->refreshGraph();
    }
    document->setFileBatchMode(true);
    if (!document->exportDocumentSync(QUrl::fromLocalFile(url), mimeType.toLatin1())) {
        if (document->errorMessage().isEmpty()) {
            QMessageBox::critical(0, i18nc("@title:window", "Krita"), i18n("Could not save\n%1", document->localFilePath()));
        } else {
            QMessageBox::critical(0, i18nc("@title:window", "Krita"), i18n("Could not save %1\nReason: %2", document->localFilePath(), document->errorMessage()));
        }
        return false;
    }

    delete document;

    return true;
}

void Imagesplit::slotImagesplit()
{
    // Taking the title - url from caption function and removing file extension
    QStringList strList = ((viewManager()->document())->caption()).split('.');
    QString suffix = strList.at(0);

    // Getting all mime types and converting them into names which are displayed at combo box
    QStringList listMimeFilter = KisImportExportManager::supportedMimeTypes(KisImportExportManager::Export);
    QString defaultMime = QString::fromLatin1(viewManager()->document()->mimeType());
    int defaultMimeIndex = 0;

    listMimeFilter.sort();
    QStringList filteredMimeTypes;
    QStringList listFileType;
    int i = 0;
    Q_FOREACH (const QString & mimeType, listMimeFilter) {
        listFileType.append(KisMimeDatabase::descriptionForMimeType(mimeType));
        filteredMimeTypes.append(mimeType);
        if (mimeType == defaultMime) {
            defaultMimeIndex = i;
        }
        i++;
    }

    listMimeFilter = filteredMimeTypes;

    Q_ASSERT(listMimeFilter.size() == listFileType.size());

    DlgImagesplit *dlgImagesplit = new DlgImagesplit(viewManager(), suffix, listFileType, defaultMimeIndex);
    dlgImagesplit->setObjectName("Imagesplit");
    Q_CHECK_PTR(dlgImagesplit);

    KisImageWSP image = viewManager()->image();

    if (dlgImagesplit->exec() == QDialog::Accepted) {

        int numHorizontalLines = dlgImagesplit->horizontalLines();
        int numVerticalLines = dlgImagesplit->verticalLines();

        int img_width = image->width() / (numVerticalLines + 1);
        int img_height = image->height() / (numHorizontalLines + 1);


        bool stop = false;
        if (dlgImagesplit->autoSave()) {
            KoFileDialog dialog(viewManager()->mainWindow(), KoFileDialog::OpenDirectory, "OpenDocument");
            dialog.setCaption(i18n("Save Image on Split"));
            dialog.setDefaultDir(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
            QStringList mimeFilter = viewManager()->document()->importExportManager()->supportedMimeTypes(KisImportExportManager::Export);
            QString defaultMime = QString::fromLatin1(viewManager()->document()->mimeType());
            dialog.setMimeTypeFilters(mimeFilter, defaultMime);

            QUrl directory = QUrl::fromUserInput(dialog.filename());

            if (directory.isEmpty())
                return;
            for (int i = 0, k = 1; i < (numVerticalLines + 1); i++) {
                for (int j = 0; j < (numHorizontalLines + 1); j++, k++) {
                    QString mimeTypeSelected = listMimeFilter.at(dlgImagesplit->cmbIndex);
                    QString homepath = directory.toLocalFile();
                    QString suffix = KisMimeDatabase::suffixesForMimeType(mimeTypeSelected).first();
                    qDebug() << "suffix" << suffix;
                    if (suffix.startsWith("*.")) {
                        suffix = suffix.remove(0, 1);
                    }
                    qDebug() << "\tsuffix" << suffix;
                    if (!suffix.startsWith(".")) {
                        suffix = suffix.prepend('.');
                    }
                    qDebug() << "\tsuffix" << suffix;
                    QString fileName = dlgImagesplit->suffix() + '_' + QString::number(k) + suffix;
                    QString url = homepath  + '/' + fileName;
                    if (!saveAsImage(QRect((i * img_width), (j * img_height), img_width, img_height), listMimeFilter.at(dlgImagesplit->cmbIndex), url)) {
                        stop = true;
                        break;
                    }
                }
                if (stop) {
                    break;
                }
            }
        }
        else {

            for (int i = 0; i < (numVerticalLines + 1); i++) {
                for (int j = 0; j < (numHorizontalLines + 1); j++) {
                    KoFileDialog dialog(viewManager()->mainWindow(), KoFileDialog::SaveFile, "OpenDocument");
                    dialog.setCaption(i18n("Save Image on Split"));
                    dialog.setDefaultDir(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
                    dialog.setMimeTypeFilters(listMimeFilter, defaultMime);

                    QUrl url = QUrl::fromUserInput(dialog.filename());

                    QString mimefilter = KisMimeDatabase::mimeTypeForFile(url.toLocalFile(), false);

                    if (url.isEmpty())
                        return;
                    if (!saveAsImage(QRect((i * img_width), (j * img_height), img_width, img_height), mimefilter, url.toLocalFile())) {
                        stop = true;
                        break;
                    }
                }
                if (stop) {
                    break;
                }
            }

        }

    }
    delete dlgImagesplit;
}

#include "imagesplit.moc"
