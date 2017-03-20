/*
 *  Copyright (c) 2017 Eugene Ingerman
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

#include "kis_tool_smart_patch.h"

#include <klocalizedstring.h>
#include <QAction>
#include <QLabel>
#include <kactioncollection.h>

#include <KoCanvasBase.h>
#include <KoCanvasController.h>

#include <KisViewManager.h>
#include "kis_canvas2.h"
#include "kis_cursor.h"
#include "kis_config.h"
#include "kundo2magicstring.h"

#include "KoProperties.h"
#include "kis_node_manager.h"

#include "kis_tool_smart_patch_options_widget.h"
#include "libs/image/kis_paint_device_debug_utils.h"

#include "kis_datamanager.h"

struct KisToolSmartPatch::Private
{
    KisNodeSP maskNode = nullptr;
    KisNodeSP paintNode = nullptr;
    KisPaintDeviceSP maskDev = nullptr;
};


KisToolSmartPatch::KisToolSmartPatch(KoCanvasBase * canvas)
    : KisToolFreehand(canvas,
                      KisCursor::load("tool_freehand_cursor.png", 5, 5),
                      kundo2_i18n("Smart Patch Stroke")),
      m_d(new Private)
{
    setObjectName("tool_SmartPatch");
}

KisToolSmartPatch::~KisToolSmartPatch()
{
}

void KisToolSmartPatch::activate(ToolActivation activation, const QSet<KoShape*> &shapes)
{
    KisToolFreehand::activate(activation, shapes);
}

void KisToolSmartPatch::deactivate()
{

    KisToolFreehand::deactivate();
}

void KisToolSmartPatch::resetCursorStyle()
{
    KisToolFreehand::resetCursorStyle();
}


bool KisToolSmartPatch::canCreateInpaintMask() const
{
    KisNodeSP node = currentNode();
    return node && node->inherits("KisLayer");
}

void KisToolSmartPatch::activatePrimaryAction()
{
    KisToolFreehand::activatePrimaryAction();

    if (!canCreateInpaintMask()) {
        qDebug() << "Inpaint can only be applied to paint Layer";
    }
}

void KisToolSmartPatch::deactivatePrimaryAction()
{
    KisToolFreehand::deactivatePrimaryAction();
}

void KisToolSmartPatch::beginPrimaryAction(KoPointerEvent *event)
{
    qDebug() << __FUNCTION__ << " 0";

    KisNodeSP node = currentNode();
    if (!node) return;
    KisCanvas2 * kiscanvas = static_cast<KisCanvas2*>(canvas());
    KisViewManager* viewManager = kiscanvas->viewManager();

    if (!m_d->maskNode.isNull()) {
        viewManager->nodeManager()->slotNonUiActivatedNode(m_d->maskNode);
        qDebug() << __FUNCTION__ << " 1";
    } else {

        m_d->paintNode = viewManager->nodeManager()->activeNode();
        viewManager->nodeManager()->createNode("KisInpaintMask", true);
        m_d->maskNode = currentNode();

        if ( ! m_d->maskNode.isNull() ){
            m_d->maskNode->setProperty("visible", false);
            m_d->maskNode->setProperty("temporary", true);
            m_d->maskNode->setProperty("inpaintmask", true);
        }
        viewManager->nodeManager()->slotNonUiActivatedNode(m_d->maskNode);
        qDebug() << __FUNCTION__ << " 2";
        KisToolFreehand::beginPrimaryAction(event);
        qDebug() << __FUNCTION__ << " 3";
    }
}

void KisToolSmartPatch::continuePrimaryAction(KoPointerEvent *event)
{
    KisToolFreehand::continuePrimaryAction(event);
}


void KisToolSmartPatch::endPrimaryAction(KoPointerEvent *event)
{
    qDebug() << __FUNCTION__ << " 0";
    if( mode() != KisTool::PAINT_MODE )
        return;

    KisToolFreehand::endPrimaryAction(event);


    qDebug() << __FUNCTION__ << " 1";

    //Wait for the paint operation to finish
    image()->waitForDone();

    m_d->maskDev = new KisPaintDevice(currentNode()->paintDevice()->colorSpace());
    m_d->maskDev->makeCloneFrom(currentNode()->paintDevice(), currentNode()->paintDevice()->extent());

    KIS_DUMP_DEVICE_2(currentNode()->paintDevice(), currentNode()->paintDevice()->extent(), "output", "/home/eugening/Projects/Out");

    KisCanvas2 * kiscanvas = static_cast<KisCanvas2*>(canvas());
    KisViewManager* viewManager = kiscanvas->viewManager();
    if( ! m_d->maskNode.isNull() )
        viewManager->nodeManager()->removeSingleNode(m_d->maskNode);
    if( ! m_d->paintNode.isNull() )
        viewManager->nodeManager()->slotNonUiActivatedNode( m_d->paintNode );
    m_d->paintNode = nullptr;
    m_d->maskNode = nullptr;
    qDebug() << __FUNCTION__ << " 3";
}

QWidget * KisToolSmartPatch::createOptionWidget()
{
    KisCanvas2 * kiscanvas = dynamic_cast<KisCanvas2*>(canvas());

    QWidget *optionsWidget = new KisToolSmartPatchOptionsWidget(kiscanvas->viewManager()->resourceProvider(), 0);
    optionsWidget->setObjectName(toolId() + "option widget");

    // // See https://bugs.kde.org/show_bug.cgi?id=316896
    // QWidget *specialSpacer = new QWidget(optionsWidget);
    // specialSpacer->setObjectName("SpecialSpacer");
    // specialSpacer->setFixedSize(0, 0);
    // optionsWidget->layout()->addWidget(specialSpacer);

    return optionsWidget;
}


