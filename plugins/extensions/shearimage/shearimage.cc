/*
 * shearimage.cc -- Part of Krita
 *
 * SPDX-FileCopyrightText: 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "shearimage.h"

#include <klocalizedstring.h>
#include <kpluginfactory.h>

#include <kis_image.h>
#include <KisViewManager.h>
#include <kis_node_manager.h>
#include <kis_image_manager.h>
#include <kis_action.h>
#include "kis_selection.h"

#include "dlg_shearimage.h"

K_PLUGIN_FACTORY_WITH_JSON(ShearImageFactory, "kritashearimage.json", registerPlugin<ShearImage>();)

ShearImage::ShearImage(QObject *parent, const QVariantList &)
    : KisActionPlugin(parent)
{
    KisAction *action = createAction("shearimage");
    connect(action,  SIGNAL(triggered()), this, SLOT(slotShearImage()));

    action = createAction("shearlayer");
    connect(action,  SIGNAL(triggered()), this, SLOT(slotShearLayer()));

    action = createAction("shearAllLayers");
    connect(action,  SIGNAL(triggered()), this, SLOT(slotShearAllLayers()));
}

ShearImage::~ShearImage()
{
}

void ShearImage::slotShearImage()
{
    KisImageWSP image = viewManager()->image();
    if (!image) return;

    if (!viewManager()->blockUntilOperationsFinished(image)) return;

    DlgShearImage * dlgShearImage = new DlgShearImage(viewManager()->mainWindow(), "ShearImage");
    Q_CHECK_PTR(dlgShearImage);

    dlgShearImage->setCaption(i18n("Shear Image"));

    if (dlgShearImage->exec() == QDialog::Accepted) {
        qint32 angleX = dlgShearImage->angleX();
        qint32 angleY = dlgShearImage->angleY();
        viewManager()->imageManager()->shearCurrentImage(angleX, angleY);
    }
    delete dlgShearImage;
}

void ShearImage::shearLayerImpl(KisNodeSP rootNode)
{
    KisImageWSP image = viewManager()->image();
    if (!image) return;

    if (!viewManager()->blockUntilOperationsFinished(image)) return;

    DlgShearImage * dlgShearImage = new DlgShearImage(viewManager()->mainWindow(), "ShearLayer");
    Q_CHECK_PTR(dlgShearImage);

    dlgShearImage->setCaption(i18n("Shear Layer"));

    if (dlgShearImage->exec() == QDialog::Accepted) {
        qint32 angleX = dlgShearImage->angleX();
        qint32 angleY = dlgShearImage->angleY();

        image->shearNode(rootNode,
                         angleX, angleY,
                         viewManager()->selection());
    }
    delete dlgShearImage;
}

void ShearImage::slotShearLayer()
{
    shearLayerImpl(viewManager()->activeNode());
}

void ShearImage::slotShearAllLayers()
{
    KisImageWSP image = viewManager()->image();
    if (!image) return;

    shearLayerImpl(image->root());
}

#include "shearimage.moc"
