/*
 *  SPDX-FileCopyrightText: 2019 Dmitrii Utkin <loentar@gmail.com>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisClipboardUtil.h"

#include <QApplication>
#include <QClipboard>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QTemporaryFile>

#include "KisDocument.h"
#include "KisImportExportManager.h"
#include "KisMainWindow.h"
#include "KisMimeDatabase.h"
#include "KisRemoteFileFetcher.h"
#include "KisViewManager.h"
#include "kis_canvas2.h"
#include "kis_clipboard.h"
#include "kis_file_layer.h"
#include "kis_image_manager.h"
#include "kis_import_catcher.h"
#include "kis_node_commands_adapter.h"
#include "kis_paint_layer.h"

namespace KisClipboardUtil {



void clipboardHasUrlsAction(KisView *kisview, const QMimeData *data, const QPoint eventPos)
{
    if (data->hasUrls()) {
        QList<QUrl> urls = data->urls();
        if (urls.length() > 0) {

            QMenu popup;
            popup.setObjectName("drop_popup");

            QAction *insertAsNewLayer = new QAction(i18n("Insert as New Layer"), &popup);
            QAction *insertManyLayers = new QAction(i18n("Insert Many Layers"), &popup);

            QAction *insertAsNewFileLayer = new QAction(i18n("Insert as New File Layer"), &popup);
            QAction *insertManyFileLayers = new QAction(i18n("Insert Many File Layers"), &popup);

            QAction *openInNewDocument = new QAction(i18n("Open in New Document"), &popup);
            QAction *openManyDocuments = new QAction(i18n("Open Many Documents"), &popup);

            QAction *insertAsReferenceImage = new QAction(i18n("Insert as Reference Image"), &popup);
            QAction *insertAsReferenceImages = new QAction(i18n("Insert as Reference Images"), &popup);

            QAction *cancel = new QAction(i18n("Cancel"), &popup);

            popup.addAction(insertAsNewLayer);
            popup.addAction(insertAsNewFileLayer);
            popup.addAction(openInNewDocument);
            popup.addAction(insertAsReferenceImage);

            popup.addAction(insertManyLayers);
            popup.addAction(insertManyFileLayers);
            popup.addAction(openManyDocuments);
            popup.addAction(insertAsReferenceImages);

            insertAsNewLayer->setEnabled(kisview->image() && (data->hasImage() || urls.count() == 1));
            insertAsNewFileLayer->setEnabled(kisview->image() && urls.count() == 1);
            openInNewDocument->setEnabled(urls.count() == 1);
            insertAsReferenceImage->setEnabled(kisview->image() && (data->hasImage() || urls.count() == 1));

            insertManyLayers->setEnabled(kisview->image() && urls.count() > 1);
            insertManyFileLayers->setEnabled(kisview->image() && urls.count() > 1);
            openManyDocuments->setEnabled(urls.count() > 1);
            insertAsReferenceImages->setEnabled(kisview->image() && urls.count() > 1);

            popup.addSeparator();
            popup.addAction(cancel);

            const QAction *action = popup.exec(QCursor::pos());

            if (data->hasImage() && (action == insertAsNewLayer || action == insertAsReferenceImage)) {
                KisPaintDeviceSP clip = KisClipboard::instance()->clip(QRect(), true);
                if (clip) {
                    KisImportCatcher::adaptClipToImageColorSpace(clip, kisview->image());

                    if (action == insertAsNewLayer) {
                        KisPaintLayerSP layer = new KisPaintLayer(kisview->image(), "", OPACITY_OPAQUE_U8, clip);
                        KisNodeCommandsAdapter adapter(kisview->mainWindow()->viewManager());
                        adapter.addNode(layer,
                                        kisview->mainWindow()->viewManager()->activeNode()->parent(),
                                        kisview->mainWindow()->viewManager()->activeNode());
                        kisview->activateWindow();
                        return;
                    } else if (action == insertAsReferenceImage) {
                        KisReferenceImage *reference =
                            KisReferenceImage::fromClipboard(*kisview->canvasBase()->coordinatesConverter());
                        reference->setPosition((*kisview->viewConverter()).imageToDocument(QCursor::pos()));
                        kisview->canvasBase()->referenceImagesDecoration()->addReferenceImage(reference);

                        KoToolManager::instance()->switchToolRequested("ToolReferenceImages");
                        return;
                    }
                }
            } else if (action != nullptr && action != cancel) {
                for (QUrl url : urls) { // do copy it
                    QScopedPointer<QTemporaryFile> tmp(new QTemporaryFile());
                    tmp->setAutoRemove(true);

                    if (!url.isLocalFile()) {
                        // download the file and substitute the url
                        KisRemoteFileFetcher fetcher;

                        if (!fetcher.fetchFile(url, tmp.data())) {
                            qWarning() << "Fetching" << url << "failed";
                            continue;
                        }
                        url = QUrl::fromLocalFile(tmp->fileName());
                    }

                    if (url.isLocalFile()) {
                        if (action == insertAsNewLayer || action == insertManyLayers) {
                            kisview->mainWindow()->viewManager()->imageManager()->importImage(url);
                            kisview->activateWindow();
                        }
                        else if (action == insertAsNewFileLayer || action == insertManyFileLayers) {
                            KisNodeCommandsAdapter adapter(kisview->mainWindow()->viewManager());
                            QFileInfo fileInfo(url.toLocalFile());

                            QString type = KisMimeDatabase::mimeTypeForFile(url.toLocalFile());
                            QStringList mimes =
                                KisImportExportManager::supportedMimeTypes(KisImportExportManager::Import);

                            if (!mimes.contains(type)) {
                                QString msg =
                                    KisImportExportErrorCode(ImportExportCodes::FileFormatNotSupported).errorMessage();
                                QMessageBox::warning(
                                    kisview, i18nc("@title:window", "Krita"),
                                    i18n("Could not open %2.\nReason: %1.", msg, url.toDisplayString()));
                                continue;
                            }

                            KisFileLayer *fileLayer = new KisFileLayer(kisview->image(), "", url.toLocalFile(),
                                                                       KisFileLayer::None, fileInfo.fileName(), OPACITY_OPAQUE_U8);

                            KisLayerSP above = kisview->mainWindow()->viewManager()->activeLayer();
                            KisNodeSP parent = above ? above->parent() : kisview->mainWindow()->viewManager()->image()->root();

                            adapter.addNode(fileLayer, parent, above);
                        }
                        else if (action == openInNewDocument || action == openManyDocuments) {
                            if (kisview->mainWindow()) {
                                kisview->mainWindow()->openDocument(url.toLocalFile(), KisMainWindow::None);
                            }
                        }
                        else if (action == insertAsReferenceImage || action == insertAsReferenceImages) {
                            auto *reference = KisReferenceImage::fromFile(url.toLocalFile(), *kisview->viewConverter(), kisview);

                            if (reference) {
                                const auto pos = kisview->canvasBase()->coordinatesConverter()->widgetToImage(eventPos);
                                reference->setPosition((*kisview->viewConverter()).imageToDocument(pos));
                                kisview->canvasBase()->referenceImagesDecoration()->addReferenceImage(reference);

                                KoToolManager::instance()->switchToolRequested("ToolReferenceImages");
                            }
                        }

                    }
                }
            }
        }
    }
}
}
