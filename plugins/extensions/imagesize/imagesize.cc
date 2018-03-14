/*
 * imagesize.cc -- Part of Krita
 *
 * Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
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

#include "imagesize.h"

#include <klocalizedstring.h>
#include <kis_debug.h>
#include <kpluginfactory.h>

#include <kis_image.h>
#include <kis_layer.h>
#include <kis_global.h>
#include <kis_types.h>
#include <KisViewManager.h>

#include <kis_image_manager.h>
#include <kis_node_manager.h>
#include <kis_group_layer.h>
#include <kis_selection_mask.h>
#include <kis_selection.h>

#include "dlg_imagesize.h"
#include "dlg_canvassize.h"
#include "dlg_layersize.h"
#include "kis_filter_strategy.h"
#include "kis_action.h"
#include "kis_action_manager.h"

K_PLUGIN_FACTORY_WITH_JSON(ImageSizeFactory, "kritaimagesize.json", registerPlugin<ImageSize>();)

ImageSize::ImageSize(QObject *parent, const QVariantList &)
    : KisActionPlugin(parent)
{
    KisAction *action  = createAction("imagesize");
    connect(action, SIGNAL(triggered()), this, SLOT(slotImageSize()));

    action = createAction("canvassize");
    connect(action, SIGNAL(triggered()), this, SLOT(slotCanvasSize()));

    action = createAction("layersize");
    connect(action, SIGNAL(triggered()), this, SLOT(slotLayerSize()));

    action  = createAction("selectionscale");
    connect(action, SIGNAL(triggered()), this, SLOT(slotSelectionScale()));
}

ImageSize::~ImageSize()
{
}

void ImageSize::slotImageSize()
{
    KisImageSP image = viewManager()->image().toStrongRef();
    if (!image) return;

    DlgImageSize * dlgImageSize = new DlgImageSize(viewManager()->mainWindow(), image->width(), image->height(), image->yRes());
    Q_CHECK_PTR(dlgImageSize);
    dlgImageSize->setObjectName("ImageSize");

    if (dlgImageSize->exec() == QDialog::Accepted) {
        qint32 w = dlgImageSize->width();
        qint32 h = dlgImageSize->height();
        double res = dlgImageSize->resolution();

        viewManager()->imageManager()->scaleCurrentImage(QSize(w, h), res, res, dlgImageSize->filterType());
    }

    delete dlgImageSize;
}

void ImageSize::slotCanvasSize()
{
    KisImageWSP image = viewManager()->image();

    if (!image) return;

    DlgCanvasSize * dlgCanvasSize = new DlgCanvasSize(viewManager()->mainWindow(), image->width(), image->height(), image->yRes());
    Q_CHECK_PTR(dlgCanvasSize);

    if (dlgCanvasSize->exec() == QDialog::Accepted) {
        qint32 width = dlgCanvasSize->width();
        qint32 height = dlgCanvasSize->height();
        qint32 xOffset = dlgCanvasSize->xOffset();
        qint32 yOffset = dlgCanvasSize->yOffset();

        viewManager()->imageManager()->resizeCurrentImage(width, height, xOffset, yOffset);
    }
    delete dlgCanvasSize;
}

void ImageSize::slotLayerSize()
{
    KisImageWSP image = viewManager()->image();

    if (!image) return;

    KisPaintDeviceSP dev = viewManager()->activeLayer()->projection();
    Q_ASSERT(dev);
    QRect rc = dev->exactBounds();

    DlgLayerSize * dlgLayerSize = new DlgLayerSize(viewManager()->mainWindow(), "LayerSize", rc.width(), rc.height(), image->yRes());
    Q_CHECK_PTR(dlgLayerSize);
    dlgLayerSize->setCaption(i18n("Resize Layer"));

    if (dlgLayerSize->exec() == QDialog::Accepted) {
        qint32 w = dlgLayerSize->width();
        qint32 h = dlgLayerSize->height();

        viewManager()->nodeManager()->scale((double)w / ((double)(rc.width())),
                                     (double)h / ((double)(rc.height())),
                                     dlgLayerSize->filterType());
    }
    delete dlgLayerSize;
}

void ImageSize::slotSelectionScale()
{
    KisImageSP image = viewManager()->image();
    if (!image) {
        return;
    }
    KisLayerSP layer = viewManager()->activeLayer();

    KIS_ASSERT_RECOVER_RETURN(image && layer);

    KisSelectionMaskSP selectionMask = layer->selectionMask();
    if (!selectionMask) {
        selectionMask = image->rootLayer()->selectionMask();
    }

    KIS_ASSERT_RECOVER_RETURN(selectionMask);

    QRect rc = selectionMask->selection()->selectedExactRect();
    DlgLayerSize * dlgSize = new DlgLayerSize(viewManager()->mainWindow(), "SelectionScale", rc.width(), rc.height(), image->yRes());
    dlgSize->setCaption(i18n("Scale Selection"));

    if (dlgSize->exec() == QDialog::Accepted) {
        qint32 w = dlgSize->width();
        qint32 h = dlgSize->height();

        image->scaleNode(selectionMask,
                         qreal(w) / rc.width(),
                         qreal(h) / rc.height(),
                         dlgSize->filterType());
    }
    delete dlgSize;
}

#include "imagesize.moc"
