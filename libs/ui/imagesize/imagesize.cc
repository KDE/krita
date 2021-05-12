/*
 * imagesize.cc -- Part of Krita
 *
 * SPDX-FileCopyrightText: 2004 Boudewijn Rempt (boud@valdyas.org)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

ImageSize::ImageSize(QObject *parent)
    : KisActionPlugin(parent)
{

    KisAction *action  = createAction("imagesize");
    connect(action, SIGNAL(triggered()), this, SLOT(slotImageSize()));

    action = createAction("canvassize");
    connect(action, SIGNAL(triggered()), this, SLOT(slotCanvasSize()));

    action = createAction("layersize");
    connect(action, SIGNAL(triggered()), this, SLOT(slotLayerSize()));

    action = createAction("scaleAllLayers");
    connect(action, SIGNAL(triggered()), this, SLOT(slotScaleAllLayers()));

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

    if (!viewManager()->blockUntilOperationsFinished(image)) return;

    DlgImageSize * dlgImageSize = new DlgImageSize(viewManager()->mainWindow(), image->width(), image->height(), image->yRes());
    Q_CHECK_PTR(dlgImageSize);
    dlgImageSize->setObjectName("ImageSize");

    if (dlgImageSize->exec() == QDialog::Accepted) {
        const QSize desiredSize(dlgImageSize->desiredWidth(), dlgImageSize->desiredHeight());
        double res = dlgImageSize->desiredResolution();
        viewManager()->imageManager()->scaleCurrentImage(desiredSize, res, res, dlgImageSize->filterType());
    }

    delete dlgImageSize;
}

void ImageSize::slotCanvasSize()
{
    KisImageWSP image = viewManager()->image();
    if (!image) return;

    if(!viewManager()->blockUntilOperationsFinished(image)) return;

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

void ImageSize::scaleLayerImpl(KisNodeSP rootNode)
{
    KisImageWSP image = viewManager()->image();
    if (!image) return;

    if(!viewManager()->blockUntilOperationsFinished(image)) return;

    QRect bounds;
    KisSelectionSP selection = viewManager()->selection();

    if (selection) {
        bounds = selection->selectedExactRect();
    } else {
        KisPaintDeviceSP dev = rootNode->projection();
        KIS_SAFE_ASSERT_RECOVER_RETURN(dev);
        bounds = dev->exactBounds();
    }

    DlgLayerSize * dlgLayerSize = new DlgLayerSize(viewManager()->mainWindow(), "LayerSize", bounds.width(), bounds.height(), image->yRes());
    Q_CHECK_PTR(dlgLayerSize);
    dlgLayerSize->setCaption(i18n("Resize Layer"));

    if (dlgLayerSize->exec() == QDialog::Accepted) {
        const QSize desiredSize(dlgLayerSize->desiredWidth(), dlgLayerSize->desiredHeight());

        viewManager()->image()->scaleNode(rootNode,
                                          QRectF(bounds).center(),
                                          qreal(desiredSize.width()) / bounds.width(),
                                          qreal(desiredSize.height()) / bounds.height(),
                                          dlgLayerSize->filterType(),
                                          selection);
    }
    delete dlgLayerSize;
}

void ImageSize::slotLayerSize()
{
    scaleLayerImpl(viewManager()->activeNode());
}

void ImageSize::slotScaleAllLayers()
{
    KisImageWSP image = viewManager()->image();
    if (!image) return;

    scaleLayerImpl(image->root());
}

void ImageSize::slotSelectionScale()
{
    KisImageSP image = viewManager()->image();
    if (!image) return;

    if(!viewManager()->blockUntilOperationsFinished(image)) return;

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
        qint32 w = dlgSize->desiredWidth();
        qint32 h = dlgSize->desiredHeight();

        image->scaleNode(selectionMask,
                         QRectF(rc).center(),
                         qreal(w) / rc.width(),
                         qreal(h) / rc.height(),
                         dlgSize->filterType(), 0);
    }
    delete dlgSize;
}
