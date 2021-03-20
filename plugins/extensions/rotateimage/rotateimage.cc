/*
 * SPDX-FileCopyrightText: 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "rotateimage.h"

#include <math.h>

#include <klocalizedstring.h>
#include <kis_debug.h>
#include <kpluginfactory.h>
#include <kis_icon.h>
#include <kundo2magicstring.h>
#include <kis_image.h>
#include <kis_types.h>
#include <KisViewManager.h>
#include <kis_image_manager.h>
#include <kis_node_manager.h>
#include <kis_canvas_resource_provider.h>
#include <kis_group_layer.h>
#include <kis_action.h>
#include <kis_selection.h>

#include "dlg_rotateimage.h"

K_PLUGIN_FACTORY_WITH_JSON(RotateImageFactory, "kritarotateimage.json", registerPlugin<RotateImage>();)

RotateImage::RotateImage(QObject *parent, const QVariantList &)
        : KisActionPlugin(parent)
{

    KisAction *action  = createAction("rotateimage");
    connect(action, SIGNAL(triggered()), this, SLOT(slotRotateImage()));

    action  = createAction("rotateImageCW90");
    connect(action, SIGNAL(triggered()), this, SLOT(slotRotateImage90()));

    action  = createAction("rotateImage180");
    connect(action, SIGNAL(triggered()), this, SLOT(slotRotateImage180()));

    action  = createAction("rotateImageCCW90");
    connect(action, SIGNAL(triggered()), this, SLOT(slotRotateImage270()));

    action  = createAction("mirrorImageHorizontal");
    connect(action, SIGNAL(triggered()), this, SLOT(slotMirrorImageHorizontal()));

    action  = createAction("mirrorImageVertical");
    connect(action, SIGNAL(triggered()), this, SLOT(slotMirrorImageVertical()));

    action  = createAction("rotatelayer");
    connect(action, SIGNAL(triggered()), this, SLOT(slotRotateLayer()));

    action  = createAction("rotateLayer180");
    connect(action, SIGNAL(triggered()), this, SLOT(slotRotateLayer180()));

    action  = createAction("rotateLayerCW90");
    connect(action, SIGNAL(triggered()), this, SLOT(slotRotateLayerCW90()));

    action  = createAction("rotateLayerCCW90");
    connect(action, SIGNAL(triggered()), this, SLOT(slotRotateLayerCCW90()));

    action  = createAction("rotateAllLayers");
    connect(action, SIGNAL(triggered()), this, SLOT(slotRotateAllLayers()));

    action  = createAction("rotateAllLayersCW90");
    connect(action, SIGNAL(triggered()), this, SLOT(slotRotateAllLayersCW90()));

    action  = createAction("rotateAllLayersCCW90");
    connect(action, SIGNAL(triggered()), this, SLOT(slotRotateAllLayersCCW90()));

    action  = createAction("rotateAllLayers180");
    connect(action, SIGNAL(triggered()), this, SLOT(slotRotateAllLayers180()));
}

RotateImage::~RotateImage()
{
}

void RotateImage::slotRotateImage()
{
    KisImageWSP image = viewManager()->image();
    if (!image) return;

    if (!viewManager()->blockUntilOperationsFinished(image)) return;

    DlgRotateImage * dlgRotateImage = new DlgRotateImage(viewManager()->mainWindow(), "RotateImage");
    Q_CHECK_PTR(dlgRotateImage);

    dlgRotateImage->setCaption(i18n("Rotate Image"));

    if (dlgRotateImage->exec() == QDialog::Accepted) {
        double angle = dlgRotateImage->angle() * M_PI / 180;
        viewManager()->imageManager()->rotateCurrentImage(angle);
    }
    delete dlgRotateImage;
}

void RotateImage::slotRotateImage90()
{
    viewManager()->imageManager()->rotateCurrentImage(M_PI / 2);
}

void RotateImage::slotRotateImage180()
{
    viewManager()->imageManager()->rotateCurrentImage(M_PI);
}

void RotateImage::slotRotateImage270()
{
    viewManager()->imageManager()->rotateCurrentImage(- M_PI / 2 + M_PI*2);
}

void RotateImage::slotMirrorImageVertical()
{
    KisImageWSP image = viewManager()->image();
    if (!image) return;
    viewManager()->nodeManager()->mirrorNode(image->rootLayer(),
                                             kundo2_i18n("Mirror Image Vertically"),
                                             Qt::Vertical, 0);
}

void RotateImage::slotMirrorImageHorizontal()
{
    KisImageWSP image = viewManager()->image();
    if (!image) return;
    viewManager()->nodeManager()->mirrorNode(image->rootLayer(),
                                             kundo2_i18n("Mirror Image Horizontally"),
                                             Qt::Horizontal, 0);
}


void RotateImage::rotateLayerCustomImpl(KisNodeSP rootNode)
{
    KisImageWSP image = viewManager()->image();
    if (!image) return;

    if (!viewManager()->blockUntilOperationsFinished(image)) return;

    DlgRotateImage * dlgRotateImage = new DlgRotateImage(viewManager()->mainWindow(), "RotateLayer");
    Q_CHECK_PTR(dlgRotateImage);

    dlgRotateImage->setCaption(i18n("Rotate Layer"));

    if (dlgRotateImage->exec() == QDialog::Accepted) {
        double angle = dlgRotateImage->angle() * M_PI / 180;
        image->rotateNode(rootNode, angle, viewManager()->selection());
    }
    delete dlgRotateImage;
}

void RotateImage::rotateLayerImpl(KisNodeSP rootNode, qreal radians)
{
    KisImageWSP image = viewManager()->image();
    if (!image) return;

    if (!viewManager()->blockUntilOperationsFinished(image)) return;

    image->rotateNode(rootNode, radians, viewManager()->selection());
}

void RotateImage::slotRotateLayer()
{
    rotateLayerCustomImpl(viewManager()->activeLayer());
}

void RotateImage::slotRotateAllLayers()
{
    KisImageWSP image = viewManager()->image();
    if (!image) return;

    rotateLayerCustomImpl(image->root());
}

void RotateImage::slotRotateLayerCW90()
{
    rotateLayerImpl(viewManager()->activeLayer(), M_PI / 2);
}

void RotateImage::slotRotateLayerCCW90()
{
    rotateLayerImpl(viewManager()->activeLayer(), -M_PI / 2);
}

void RotateImage::slotRotateLayer180()
{
    rotateLayerImpl(viewManager()->activeLayer(), M_PI);
}

void RotateImage::slotRotateAllLayersCW90()
{
    KisImageWSP image = viewManager()->image();
    if (!image) return;

    rotateLayerImpl(image->root(), M_PI / 2);
}

void RotateImage::slotRotateAllLayersCCW90()
{
    KisImageWSP image = viewManager()->image();
    if (!image) return;

    rotateLayerImpl(image->root(), -M_PI / 2);
}

void RotateImage::slotRotateAllLayers180()
{
    KisImageWSP image = viewManager()->image();
    if (!image) return;

    rotateLayerImpl(image->root(), M_PI);
}

#include "rotateimage.moc"
