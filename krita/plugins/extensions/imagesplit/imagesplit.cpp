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
#include <QDesktopServices>

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
#include <KisDocument.h>
#include <kis_paint_layer.h>
#include <kis_painter.h>
#include <kis_paint_device.h>
#include <QMimeDatabase>
#include <QMimeType>

#include "dlg_imagesplit.h"

K_PLUGIN_FACTORY_WITH_JSON(ImagesplitFactory, "kritaimagesplit.json", registerPlugin<Imagesplit>();)

Imagesplit::Imagesplit(QObject *parent, const QVariantList &)
    : KisViewPlugin(parent)
{
    KisAction *action  = createAction("imagesplit");
    action->setActivationFlags(KisAction::ACTIVE_NODE);
    connect(action, SIGNAL(triggered()), this, SLOT(slotImagesplit()));
}

Imagesplit::~Imagesplit()
{
}

void Imagesplit::saveAsImage(const QRect &imgSize, const QString &mimeType, const QString &url)
{
    KisImageWSP image = m_view->image();

    KisDocument *d = KisPart::instance()->createDocument();
    d->prepareForImport();

    KisImageWSP dst = new KisImage(d->createUndoStore(), imgSize.width(), imgSize.height(), image->colorSpace(), image->objectName());
    dst->setResolution(image->xRes(), image->yRes());
    d->setCurrentImage(dst);

    KisPaintLayer* paintLayer = new KisPaintLayer(dst, dst->nextLayerName(), 255);
    KisPainter gc(paintLayer->paintDevice());
    gc.bitBlt(QPoint(0, 0), image->projection(), imgSize);

    dst->addNode(paintLayer, KisNodeSP(0));
    dst->refreshGraph();
    d->setOutputMimeType(mimeType.toLatin1());
    d->exportDocument(QUrl::fromLocalFile(url));

    delete d;
}

void Imagesplit::slotImagesplit()
{
    // Taking the title - url from caption function and removing file extension
    QStringList strList = ((m_view->document())->caption()).split('.');
    QString suffix = strList.at(0);

    // Getting all mime types and converting them into names which are displayed at combo box
    QStringList listMimeFilter = KisImportExportManager::mimeFilter("application/x-krita", KisImportExportManager::Export);
    QStringList filteredMimeTypes;
    QStringList listFileType;
    foreach(const QString & tempStr, listMimeFilter) {
        QMimeDatabase db;
        QMimeType type = db.mimeTypeForName(tempStr);
        dbgKrita << tempStr << type;
        if (type.isValid()) {
            listFileType.append(type.comment());
            filteredMimeTypes.append(tempStr);
        }
    }

    listMimeFilter = filteredMimeTypes;

    Q_ASSERT(listMimeFilter.size() == listFileType.size());

    DlgImagesplit * dlgImagesplit = new DlgImagesplit(m_view, suffix, listFileType);
    dlgImagesplit->setObjectName("Imagesplit");
    Q_CHECK_PTR(dlgImagesplit);

    KisImageWSP image = m_view->image();

    if (dlgImagesplit->exec() == QDialog::Accepted) {

        int numHorizontalLines = dlgImagesplit->horizontalLines();
        int numVerticalLines = dlgImagesplit->verticalLines();

        int img_width = image->width() / (numVerticalLines + 1);
        int img_height = image->height() / (numHorizontalLines + 1);


        if (dlgImagesplit->autoSave()) {
            for (int i = 0, k = 1; i < (numVerticalLines + 1); i++) {
                for (int j = 0; j < (numHorizontalLines + 1); j++, k++) {
                    QMimeDatabase db;
                    QMimeType mimeTypeSelected = db.mimeTypeForName(listMimeFilter.at(dlgImagesplit->cmbIndex));
                    QString homepath = QDir::homePath();
                    QString fileName = dlgImagesplit->suffix() + '_' + QString::number(k) + mimeTypeSelected.preferredSuffix();
                    QString url = homepath  + '/' + fileName;
                    saveAsImage(QRect((i * img_width), (j * img_height), img_width, img_height), listMimeFilter.at(dlgImagesplit->cmbIndex), url);
                }
            }
        }
        else {

            for (int i = 0; i < (numVerticalLines + 1); i++) {
                for (int j = 0; j < (numHorizontalLines + 1); j++) {
                    KoFileDialog dialog(m_view->mainWindow(), KoFileDialog::SaveFile, "OpenDocument");
                    dialog.setCaption(i18n("Save Image on Split"));
                    dialog.setDefaultDir(QDesktopServices::storageLocation(QDesktopServices::PicturesLocation));
                    dialog.setMimeTypeFilters(listMimeFilter);
                    QUrl url = QUrl::fromUserInput(dialog.filename());

                    QMimeDatabase db;
                    QMimeType mime = db.mimeTypeForUrl(url);
                    QString mimefilter = mime.name();

                    if (url.isEmpty())
                        return;
                    saveAsImage(QRect((i * img_width), (j * img_height), img_width, img_height), mimefilter, url.toLocalFile());
                }
            }

        }

    }
    delete dlgImagesplit;
}

#include "imagesplit.moc"
