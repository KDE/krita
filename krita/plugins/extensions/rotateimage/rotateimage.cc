/*
 * rotateimage.cc -- Part of Krita
 *
 * Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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

#include "rotateimage.h"

#include <math.h>

#include <klocale.h>
#include <kis_debug.h>
#include <kpluginfactory.h>
#include <KoIcon.h>
#include <kis_image.h>
#include <kis_types.h>
#include <kis_view2.h>
#include <kis_image_manager.h>
#include <kis_node_manager.h>
#include <kis_canvas_resource_provider.h>
#include <kis_group_layer.h>
#include <kis_action.h>

#include "dlg_rotateimage.h"

K_PLUGIN_FACTORY(RotateImageFactory, registerPlugin<RotateImage>();)
K_EXPORT_PLUGIN(RotateImageFactory("krita"))

RotateImage::RotateImage(QObject *parent, const QVariantList &)
        : KisViewPlugin(parent, "kritaplugins/rotateimage.rc")
{
    KisAction *action  = new KisAction(i18n("&Rotate Image..."), this);
    addAction("rotateimage", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotRotateImage()));

    action  = new KisAction(koIcon("object-rotate-right"), i18nc("rotate image 90 degrees to the right", "Rotate Image 90° to the Right"), this);
    addAction("rotateImageCW90", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotRotateImage90()));

    action  = new KisAction(i18nc("rotate image 180 degrees to the right", "Rotate Image 180°"), this);
    addAction("rotateImage180", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotRotateImage180()));

    action  = new KisAction(koIcon("object-rotate-left"), i18nc("rotate image 90 degrees to the left", "Rotate Image 90° to the Left"), this);
    addAction("rotateImageCCW90", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotRotateImage270()));

    action  = new KisAction(koIcon("object-flip-horizontal"), i18n("Mirror Horizontally"), this);
    addAction("mirrorHorizontal", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotMirrorImageHorizontal()));

    action  = new KisAction(koIcon("object-flip-vertical"), i18n("Mirror Vertically"), this);
    addAction("mirrorVertical", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotMirrorImageVertical()));

    action  = new KisAction(i18n("&Rotate Layer..."), this);
    action->setActivationFlags(KisAction::ACTIVE_LAYER);
    action->setActivationConditions(KisAction::ACTIVE_NODE_EDITABLE);
    addAction("rotatelayer", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotRotateLayer()));

    action  = new KisAction(i18nc("rotate the layer 180 degrees", "Rotate Layer 180°"), this);
    action->setActivationFlags(KisAction::ACTIVE_LAYER);
    action->setActivationConditions(KisAction::ACTIVE_NODE_EDITABLE);
    addAction("rotateLayer180", action);
    connect(action, SIGNAL(triggered()), m_view->nodeManager(), SLOT(rotate180()));

    action  = new KisAction(koIcon("object-rotate-right"), i18nc("rotate the layer 90 degrees to the right", "Rotate Layer 90° to the Right"), this);
    action->setActivationFlags(KisAction::ACTIVE_LAYER);
    action->setActivationConditions(KisAction::ACTIVE_NODE_EDITABLE);
    addAction("rotateLayerCW90", action);
    connect(action, SIGNAL(triggered()), m_view->nodeManager(), SLOT(rotateRight90()));

    action  = new KisAction(koIcon("object-rotate-left"), i18nc("rotate the layer 90 degrees to the left", "Rotate Layer 90° to the Left"), this);
    action->setActivationFlags(KisAction::ACTIVE_LAYER);
    action->setActivationConditions(KisAction::ACTIVE_NODE_EDITABLE);
    addAction("rotateLayerCCW90", action);
    connect(action, SIGNAL(triggered()), m_view->nodeManager(), SLOT(rotateLeft90()));
}

RotateImage::~RotateImage()
{
}

void RotateImage::slotRotateImage()
{
    KisImageWSP image = m_view->image();

    if (!image) return;

    DlgRotateImage * dlgRotateImage = new DlgRotateImage(m_view, "RotateImage");
    Q_CHECK_PTR(dlgRotateImage);

    dlgRotateImage->setCaption(i18n("Rotate Image"));

    if (dlgRotateImage->exec() == QDialog::Accepted) {
        double angle = dlgRotateImage->angle() * M_PI / 180;
        m_view->imageManager()->rotateCurrentImage(angle);
    }
    delete dlgRotateImage;
}

void RotateImage::slotRotateImage90()
{
    m_view->imageManager()->rotateCurrentImage(M_PI / 2);
}

void RotateImage::slotRotateImage180()
{
    m_view->imageManager()->rotateCurrentImage(M_PI);
}

void RotateImage::slotRotateImage270()
{
    m_view->imageManager()->rotateCurrentImage(- M_PI / 2 + M_PI*2);
}

void RotateImage::slotMirrorImageVertical()
{
    KisImageWSP image = m_view->image();
    if (!image) return;
    m_view->nodeManager()->mirrorNode(image->rootLayer(), i18n("Mirror Vertically"), Qt::Vertical);
}

void RotateImage::slotMirrorImageHorizontal()
{
    KisImageWSP image = m_view->image();
    if (!image) return;
    m_view->nodeManager()->mirrorNode(image->rootLayer(), i18n("Mirror Horizontally"), Qt::Horizontal);
}

void RotateImage::slotRotateLayer()
{
    KisImageWSP image = m_view->image();

    if (!image) return;

    DlgRotateImage * dlgRotateImage = new DlgRotateImage(m_view, "RotateLayer");
    Q_CHECK_PTR(dlgRotateImage);

    dlgRotateImage->setCaption(i18n("Rotate Layer"));

    if (dlgRotateImage->exec() == QDialog::Accepted) {
        double angle = dlgRotateImage->angle() * M_PI / 180;
        m_view->nodeManager()->rotate(angle);

    }
    delete dlgRotateImage;
}

#include "rotateimage.moc"
