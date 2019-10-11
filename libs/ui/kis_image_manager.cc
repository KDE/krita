/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2006
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

#include "kis_image_manager.h"

#include <QString>
#include <QStandardPaths>

#include <QAction>
#include <QUrl>
#include <QColorDialog>

#include <klocalizedstring.h>

#include <KoColor.h>
#include <KoFileDialog.h>

#include <kis_types.h>
#include <kis_image.h>
#include <kis_icon.h>
#include <KisImportExportManager.h>
#include "kis_import_catcher.h"
#include "KisViewManager.h"
#include "KisDocument.h"
#include "dialogs/kis_dlg_image_properties.h"
#include "commands/kis_image_commands.h"
#include "kis_action.h"
#include "kis_action_manager.h"
#include "kis_layer_utils.h"

#include "kis_signal_compressor_with_param.h"


KisImageManager::KisImageManager(KisViewManager * view)
        : m_view(view)
{
}

void KisImageManager::setView(QPointer<KisView>imageView)
{
    Q_UNUSED(imageView);
}

void KisImageManager::setup(KisActionManager *actionManager)
{

    KisAction *action  = actionManager->createAction("import_layer_from_file");
    connect(action, SIGNAL(triggered()), this, SLOT(slotImportLayerFromFile()));

    action  = actionManager->createAction("image_properties");
    connect(action, SIGNAL(triggered()), this, SLOT(slotImageProperties()));

    action  = actionManager->createAction("import_layer_as_paint_layer");
    connect(action, SIGNAL(triggered()), this, SLOT(slotImportLayerFromFile()));

    action  = actionManager->createAction("import_layer_as_transparency_mask");
    connect(action, SIGNAL(triggered()), this, SLOT(slotImportLayerAsTransparencyMask()));

    action  = actionManager->createAction("import_layer_as_filter_mask");
    connect(action, SIGNAL(triggered()), this, SLOT(slotImportLayerAsFilterMask()));

    action  = actionManager->createAction("import_layer_as_selection_mask");
    connect(action, SIGNAL(triggered()), this, SLOT(slotImportLayerAsSelectionMask()));

    action = actionManager->createAction("image_color");
    connect(action, SIGNAL(triggered()), this, SLOT(slotImageColor()));
}

void KisImageManager::slotImportLayerFromFile()
{
    importImage(QUrl(), "KisPaintLayer");
}

void KisImageManager::slotImportLayerAsTransparencyMask()
{
    importImage(QUrl(), "KisTransparencyMask");
}

void KisImageManager::slotImportLayerAsFilterMask()
{
    importImage(QUrl(), "KisFilterMask");
}

void KisImageManager::slotImportLayerAsSelectionMask()
{
    importImage(QUrl(), "KisSelectionMask");
}


qint32 KisImageManager::importImage(const QUrl &urlArg, const QString &layerType)
{
    KisImageWSP currentImage = m_view->image();

    if (!currentImage) {
        return 0;
    }

    QList<QUrl> urls;
    qint32 rc = 0;

    if (urlArg.isEmpty()) {
        KoFileDialog dialog(m_view->mainWindow(), KoFileDialog::OpenFiles, "OpenDocument");
        dialog.setCaption(i18n("Import Image"));
        dialog.setDefaultDir(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
        dialog.setMimeTypeFilters(KisImportExportManager::supportedMimeTypes(KisImportExportManager::Import));
        QStringList fileNames = dialog.filenames();
        Q_FOREACH (const QString &fileName, fileNames) {
            urls << QUrl::fromLocalFile(fileName);
        }

    } else {
        urls.push_back(urlArg);
    }

    if (urls.empty()) {
        return 0;
    }

    Q_FOREACH(const QUrl &url, urls) {
        if (url.toLocalFile().endsWith("svg")) {
            new KisImportCatcher(url, m_view, "KisShapeLayer");
        }
        else {
            new KisImportCatcher(url, m_view, layerType);
        }
    }

    m_view->canvas()->update();

    return rc;
}

void KisImageManager::resizeCurrentImage(qint32 w, qint32 h, qint32 xOffset, qint32 yOffset)
{
    if (!m_view->image()) return;

    m_view->image()->resizeImage(QRect(-xOffset, -yOffset, w, h));
}

void KisImageManager::scaleCurrentImage(const QSize &size, qreal xres, qreal yres, KisFilterStrategy *filterStrategy)
{
    if (!m_view->image()) return;
    m_view->image()->scaleImage(size, xres, yres, filterStrategy);
}

void KisImageManager::rotateCurrentImage(double radians)
{
    if (!m_view->image()) return;
    m_view->image()->rotateImage(radians);
}

void KisImageManager::shearCurrentImage(double angleX, double angleY)
{
    if (!m_view->image()) return;
    m_view->image()->shear(angleX, angleY);
}


void KisImageManager::slotImageProperties()
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    QPointer<KisDlgImageProperties> dlg = new KisDlgImageProperties(image, m_view->mainWindow());
    if (dlg->exec() == QDialog::Accepted) {
        if (dlg->convertLayerPixels()) {
            image->convertImageColorSpace(dlg->colorSpace(),
                                          KoColorConversionTransformation::internalRenderingIntent(),
                                          KoColorConversionTransformation::internalConversionFlags());

        } else {
            image->convertImageProjectionColorSpace(dlg->colorSpace());
        }
    }
    delete dlg;
}

void updateImageBackgroundColor(KisImageSP image, const QColorDialog *dlg)
{
    QColor newColor = dlg->currentColor();
    KoColor bg = image->defaultProjectionColor();
    bg.fromQColor(newColor);

    KisLayerUtils::changeImageDefaultProjectionColor(image, bg);
}

void KisImageManager::slotImageColor()
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    QColorDialog dlg;
    dlg.setOption(QColorDialog::ShowAlphaChannel, true);
    dlg.setWindowTitle(i18n("Select a Color"));
    KoColor bg = image->defaultProjectionColor();
    dlg.setCurrentColor(bg.toQColor());

    KisSignalCompressor compressor(200, KisSignalCompressor::FIRST_INACTIVE);

    std::function<void ()> updateCall(std::bind(updateImageBackgroundColor, image, &dlg));
    SignalToFunctionProxy proxy(updateCall);

    connect(&dlg, SIGNAL(currentColorChanged(QColor)), &compressor, SLOT(start()));
    connect(&compressor, SIGNAL(timeout()), &proxy, SLOT(start()));

    dlg.exec();
}


