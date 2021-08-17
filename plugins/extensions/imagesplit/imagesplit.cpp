/*
 * imagesplit.cc -- Part of Krita
 *
 * SPDX-FileCopyrightText: 2004 Boudewijn Rempt (boud@valdyas.org)
 * SPDX-FileCopyrightText: 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
#include <kis_coordinates_converter.h>
#include <kis_guides_config.h>

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
    if (!document->exportDocumentSync(url, mimeType.toLatin1())) {
        if (document->errorMessage().isEmpty()) {
            QMessageBox::critical(qApp->activeWindow(), i18nc("@title:window", "Krita"), i18n("Could not save\n%1", document->localFilePath()));
        } else {
            QMessageBox::critical(qApp->activeWindow(), i18nc("@title:window", "Krita"), i18n("Could not save %1\nReason: %2", document->localFilePath(), document->errorMessage()));
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
    if (defaultMime.isEmpty()) defaultMime = QString::fromLatin1(viewManager()->document()->nativeFormatMimeType());
    int defaultMimeIndex = 0;

    listMimeFilter.sort();
    QStringList filteredMimeTypes;
    QStringList listFileType;
    int i = 0;
    Q_FOREACH (const QString &mimeType, listMimeFilter) {
        listFileType.append(KisMimeDatabase::descriptionForMimeType(mimeType));
        filteredMimeTypes.append(mimeType);
        if (mimeType == defaultMime) {
            defaultMimeIndex = i;
        }
        i++;
    }

    listMimeFilter = filteredMimeTypes;

    Q_ASSERT(listMimeFilter.size() == listFileType.size());

    KisCoordinatesConverter converter;
    QList <qreal> xGuides = viewManager()->document()->guidesConfig().verticalGuideLines();
    QList <qreal> yGuides = viewManager()->document()->guidesConfig().horizontalGuideLines();

    std::sort(xGuides.begin(), xGuides.end());
    std::sort(yGuides.begin(), yGuides.end());

    KisImageWSP image = viewManager()->image();

    converter.setImage(image);
    QTransform transform = converter.imageToDocumentTransform().inverted();

    for (int i = 0; i< xGuides.size(); i++) {
        qreal line = xGuides[i];
        xGuides[i] = transform.map(QPointF(line, line)).x();
    }
    for (int i = 0; i< yGuides.size(); i++) {
        qreal line = yGuides[i];
        yGuides[i] = transform.map(QPointF(line, line)).y();
    }

    qreal thumbnailRatio = qreal(200)/qMax(image->width(), image->height());

    DlgImagesplit *dlgImagesplit = new DlgImagesplit(viewManager()
                                                     , suffix
                                                     , listFileType
                                                     , defaultMimeIndex
                                                     , viewManager()->document()->generatePreview(QSize(200, 200)).toImage()
                                                     , yGuides, xGuides, thumbnailRatio);
    dlgImagesplit->setObjectName("Imagesplit");
    Q_CHECK_PTR(dlgImagesplit);

    if (dlgImagesplit->exec() == QDialog::Accepted) {

        int numHorizontalLines = dlgImagesplit->horizontalLines();
        int numVerticalLines = dlgImagesplit->verticalLines();

        int img_width = image->width() / (numVerticalLines + 1);
        int img_height = image->height() / (numHorizontalLines + 1);

        if (dlgImagesplit->useHorizontalGuides()) {
            numHorizontalLines = yGuides.size();
        }
        if (dlgImagesplit->useVerticalGuides()) {
            numVerticalLines = xGuides.size();
        }


        bool stop = false;

        QString mimeType;
        QString filepath;
        QString homepath;
        QString suffix;

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

            mimeType = listMimeFilter.at(dlgImagesplit->cmbIndex);
            homepath = directory.toLocalFile();
            suffix = KisMimeDatabase::suffixesForMimeType(mimeType).first();
            if (suffix.startsWith("*.")) {
                suffix = suffix.remove(0, 1);
            }
            if (!suffix.startsWith(".")) {
                suffix = suffix.prepend('.');
            }
	    }

        int outerLoop;
        int innerLoop;

        if (dlgImagesplit->sortHorizontal()) {
            outerLoop = numHorizontalLines + 1;
            innerLoop = numVerticalLines + 1;
        }
        else {
            outerLoop = numVerticalLines + 1;
            innerLoop = numHorizontalLines + 1;
        }

        xGuides.prepend(qreal(0));
        xGuides.append(image->width());
        yGuides.prepend(qreal(0));
        yGuides.append(image->height());

        int currentX = 0;
        int currentY = 0;

        for (int i = 0, k = 1; i < outerLoop; i++) {
            for (int j = 0; j < innerLoop; j++, k++) {
                int row;
                int column;
                if (dlgImagesplit->sortHorizontal()) {
                    row = i;
                    column = j;
                }
                else {
                    row = j;
                    column = i;
                }

                if (dlgImagesplit->useVerticalGuides()) {
                    currentX = xGuides[column];
                    img_width = xGuides[column+1]-currentX;
                } else {
                    currentX = (column * img_width);
                }
                if (dlgImagesplit->useHorizontalGuides()) {
                    currentY = yGuides[row];
                    img_height = yGuides[row+1]-currentY;
                } else {
                    currentY = (row * img_height);
                }

                if (dlgImagesplit->autoSave()) {
                    QString fileName = dlgImagesplit->suffix() + '_' + QString::number(k) + suffix;
                    filepath = homepath  + '/' + fileName;
		            mimeType = listMimeFilter.at(dlgImagesplit->cmbIndex);
	            }
                else {
                    KoFileDialog dialog(viewManager()->mainWindow(), KoFileDialog::SaveFile, "OpenDocument");
                    dialog.setCaption(i18n("Save Image on Split"));
                    dialog.setDefaultDir(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
                    dialog.setMimeTypeFilters(listMimeFilter, defaultMime);

                    QUrl url = QUrl::fromUserInput(dialog.filename());
                    filepath = url.toLocalFile();

                    mimeType = KisMimeDatabase::mimeTypeForFile(url.toLocalFile(), false);

                    if (url.isEmpty())
                        return;

                }
                if (!saveAsImage(QRect(currentX, currentY, img_width, img_height), mimeType, filepath)) {
                    stop = true;
                    break;
                }

            }
            if (stop) {
                break;
            }
        }
    }
    delete dlgImagesplit;
}

#include "imagesplit.moc"
