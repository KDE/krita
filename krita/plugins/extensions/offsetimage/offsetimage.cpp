/*
 * Copyright (c) 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "offsetimage.h"

#include <cmath>

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
#include <kis_image_signal_router.h>
#include <kis_processing_applicator.h>
#include <kis_action.h>

#include "dlg_offsetimage.h"
#include "kis_offset_processing_visitor.h"

K_PLUGIN_FACTORY(OffsetImageFactory, registerPlugin<OffsetImage>();)
K_EXPORT_PLUGIN(OffsetImageFactory("krita"))

OffsetImage::OffsetImage(QObject *parent, const QVariantList &)
        : KisViewPlugin(parent, "kritaplugins/offsetimage.rc")
{
    KisAction *action  = new KisAction(i18n("&Offset Image..."), this);
    addAction("offsetimage", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotOffsetImage()));

    action  = new KisAction(i18n("&Offset Layer..."), this);
    action->setActivationFlags(KisAction::ACTIVE_LAYER);
    action->setActivationConditions(KisAction::ACTIVE_NODE_EDITABLE);
    addAction("offsetlayer", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotOffsetLayer()));
}


OffsetImage::~OffsetImage()
{
}


void OffsetImage::slotOffsetImage()
{
    KisImageWSP image = m_view->image();
    if (image) {

        DlgOffsetImage * dlgOffsetImage = new DlgOffsetImage(m_view, "OffsetImage", image->size());
        Q_CHECK_PTR(dlgOffsetImage);

        QString actionName = i18n("Offset Image");
        dlgOffsetImage->setCaption(actionName);

        if (dlgOffsetImage->exec() == QDialog::Accepted) {
            QPoint offsetPoint = QPoint(dlgOffsetImage->offsetX(), dlgOffsetImage->offsetY());
            offsetImpl(actionName, image->root(), offsetPoint);
        }
        delete dlgOffsetImage;
    }
    else
    {
        kWarning() << "KisImage not available";
    }
}


void OffsetImage::slotOffsetLayer()
{
    KisImageWSP image = m_view->image();
    if (image) {

    DlgOffsetImage * dlgOffsetImage = new DlgOffsetImage(m_view, "OffsetLayer", image->size());
    Q_CHECK_PTR(dlgOffsetImage);

    QString actionName = i18n("Offset Layer");
    dlgOffsetImage->setCaption(actionName);

    if (dlgOffsetImage->exec() == QDialog::Accepted) {
        QPoint offsetPoint = QPoint(dlgOffsetImage->offsetX(), dlgOffsetImage->offsetY());
        KisNodeSP activeNode = m_view->activeNode();
        offsetImpl(actionName, activeNode, offsetPoint);
    }
    delete dlgOffsetImage;

    }
    else
    {
        kWarning() << "KisImage not available";
    }
}

void OffsetImage::offsetImpl(const QString& actionName, KisNodeSP node, const QPoint& offsetPoint)
{
    KisImageSignalVector emitSignals;
    emitSignals << ModifiedSignal;

    KisProcessingApplicator applicator(m_view->image(), node,
                                       KisProcessingApplicator::RECURSIVE,
                                       emitSignals, actionName);

    KisProcessingVisitorSP visitor = new KisOffsetProcessingVisitor(offsetPoint, m_view->image()->bounds());
    applicator.applyVisitor(visitor, KisStrokeJobData::CONCURRENT);
    applicator.end();
}


#include "offsetimage.moc"
