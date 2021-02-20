/*
 * SPDX-FileCopyrightText: 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "offsetimage.h"

#include <cmath>

#include <klocalizedstring.h>
#include <kis_debug.h>
#include <kpluginfactory.h>
#include <kis_icon.h>
#include <kis_image.h>
#include <kis_types.h>
#include <KisViewManager.h>
#include <kis_image_manager.h>
#include <kis_node_manager.h>
#include <kis_canvas_resource_provider.h>
#include <kis_group_layer.h>
#include <kis_image_signal_router.h>
#include <kis_processing_applicator.h>
#include <kis_action.h>
#include <kis_selection.h>

#include "dlg_offsetimage.h"
#include "kis_offset_processing_visitor.h"

K_PLUGIN_FACTORY_WITH_JSON(OffsetImageFactory, "kritaoffsetimage.json", registerPlugin<OffsetImage>();)

OffsetImage::OffsetImage(QObject *parent, const QVariantList &)
        : KisActionPlugin(parent)
{
    KisAction *action  = createAction("offsetimage");
    connect(action, SIGNAL(triggered()), this, SLOT(slotOffsetImage()));

    action  = createAction("offsetlayer");
    connect(action, SIGNAL(triggered()), this, SLOT(slotOffsetLayer()));
}


OffsetImage::~OffsetImage()
{
}


void OffsetImage::slotOffsetImage()
{
    KisImageSP image = viewManager()->image().toStrongRef();
    if (image) {

        DlgOffsetImage * dlgOffsetImage = new DlgOffsetImage(viewManager()->mainWindow(), "OffsetImage", offsetWrapRect().size());
        Q_CHECK_PTR(dlgOffsetImage);

        KUndo2MagicString actionName = kundo2_i18n("Offset Image");
        dlgOffsetImage->setCaption(i18nc("@title:window", "Offset Image"));

        if (dlgOffsetImage->exec() == QDialog::Accepted) {
            QPoint offsetPoint = QPoint(dlgOffsetImage->offsetX(), dlgOffsetImage->offsetY());
            offsetImpl(actionName, image->root(), offsetPoint);
        }
        delete dlgOffsetImage;
    }
    else
    {
        dbgKrita << "KisImage not available";
    }
}


void OffsetImage::slotOffsetLayer()
{
    KisImageSP image = viewManager()->image().toStrongRef();
    if (image) {

        DlgOffsetImage * dlgOffsetImage = new DlgOffsetImage(viewManager()->mainWindow(), "OffsetLayer", offsetWrapRect().size());
        Q_CHECK_PTR(dlgOffsetImage);

        KUndo2MagicString actionName = kundo2_i18n("Offset Layer");
        dlgOffsetImage->setCaption(i18nc("@title:window", "Offset Layer"));

        if (dlgOffsetImage->exec() == QDialog::Accepted) {
            QPoint offsetPoint = QPoint(dlgOffsetImage->offsetX(), dlgOffsetImage->offsetY());
            KisNodeSP activeNode = viewManager()->activeNode();
            offsetImpl(actionName, activeNode, offsetPoint);
        }
        delete dlgOffsetImage;
    }
    else
    {
        dbgKrita << "KisImage not available";
    }
}

void OffsetImage::offsetImpl(const KUndo2MagicString& actionName, KisNodeSP node, const QPoint& offsetPoint)
{
    KisImageSignalVector emitSignals;

    KisProcessingApplicator applicator(viewManager()->image(), node,
                                       KisProcessingApplicator::RECURSIVE,
                                       emitSignals, actionName);

    QRect rc = offsetWrapRect();
    KisProcessingVisitorSP visitor = new KisOffsetProcessingVisitor(offsetPoint, rc);
    applicator.applyVisitor(visitor, KisStrokeJobData::CONCURRENT);
    applicator.end();
}


QRect OffsetImage::offsetWrapRect()
{
    QRect offsetWrapRect;
    if (viewManager()->selection())
    {
        offsetWrapRect = viewManager()->selection()->selectedExactRect();
    }
    else
    {
        KisImageSP image = viewManager()->image().toStrongRef();
        if (image) {
            offsetWrapRect = image->bounds();
        }
    }
    return offsetWrapRect;
}



#include "offsetimage.moc"
