/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_tool_lazy_brush.h"

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
#include "kis_layer_properties_icons.h"

#include "kis_canvas_resource_provider.h"
#include "kis_tool_lazy_brush_options_widget.h"

#include "lazybrush/kis_colorize_mask.h"
#include "kis_signal_auto_connection.h"


struct KisToolLazyBrush::Private
{
    bool activateMaskMode = false;
    bool oldShowKeyStrokesValue = false;
    bool oldShowColoringValue = false;

    KisNodeSP manuallyActivatedNode;
    KisSignalAutoConnectionsStore toolConnections;
};


KisToolLazyBrush::KisToolLazyBrush(KoCanvasBase * canvas)
    : KisToolFreehand(canvas,
                      KisCursor::load("tool_freehand_cursor.png", 5, 5),
                      kundo2_i18n("Colorize Mask Key Stroke")),
      m_d(new Private)
{
    setObjectName("tool_lazybrush");
}

KisToolLazyBrush::~KisToolLazyBrush()
{
}

void KisToolLazyBrush::tryDisableKeyStrokesOnMask()
{
    if (m_d->manuallyActivatedNode) {
        KisLayerPropertiesIcons::setNodeProperty(m_d->manuallyActivatedNode, KisLayerPropertiesIcons::colorizeEditKeyStrokes, false, image());
        m_d->manuallyActivatedNode = 0;
    }
}


void KisToolLazyBrush::activate(ToolActivation activation, const QSet<KoShape*> &shapes)
{
    KisCanvas2 * kiscanvas = dynamic_cast<KisCanvas2*>(canvas());
    m_d->toolConnections.addUniqueConnection(
        kiscanvas->viewManager()->canvasResourceProvider(), SIGNAL(sigNodeChanged(KisNodeSP)),
        this, SLOT(slotCurrentNodeChanged(KisNodeSP)));


    KisColorizeMask *mask = qobject_cast<KisColorizeMask*>(currentNode().data());
    if (mask) {
        mask->regeneratePrefilteredDeviceIfNeeded();
    }

    KisToolFreehand::activate(activation, shapes);
}

void KisToolLazyBrush::deactivate()
{
    KisToolFreehand::deactivate();
    tryDisableKeyStrokesOnMask();
    m_d->toolConnections.clear();
}

void KisToolLazyBrush::slotCurrentNodeChanged(KisNodeSP node)
{
    if (node != m_d->manuallyActivatedNode) {
        tryDisableKeyStrokesOnMask();

        KisColorizeMask *mask = qobject_cast<KisColorizeMask*>(node.data());
        if (mask) {
            mask->regeneratePrefilteredDeviceIfNeeded();
        }
    }
}

void KisToolLazyBrush::resetCursorStyle()
{
    KisToolFreehand::resetCursorStyle();
}

bool KisToolLazyBrush::colorizeMaskActive() const
{
    KisNodeSP node = currentNode();
    return node && node->inherits("KisColorizeMask");
}

bool KisToolLazyBrush::canCreateColorizeMask() const
{
    KisNodeSP node = currentNode();
    return node && node->inherits("KisLayer");
}

bool KisToolLazyBrush::shouldActivateKeyStrokes() const
{
    KisNodeSP node = currentNode();

    return node && node->inherits("KisColorizeMask") &&
        !KisLayerPropertiesIcons::nodeProperty(node,
                                               KisLayerPropertiesIcons::colorizeEditKeyStrokes,
                                               true).toBool();
}

void KisToolLazyBrush::tryCreateColorizeMask()
{
    KisNodeSP node = currentNode();
    if (!node) return;

    KoProperties properties;
    properties.setProperty("visible", true);
    properties.setProperty("locked", false);

    QList<KisNodeSP> masks = node->childNodes(QStringList("KisColorizeMask"), properties);

    if (!masks.isEmpty()) {
        KisCanvas2 * kiscanvas = static_cast<KisCanvas2*>(canvas());
        KisViewManager* viewManager = kiscanvas->viewManager();
        viewManager->nodeManager()->slotNonUiActivatedNode(masks.first());
    } else {
        KisCanvas2 * kiscanvas = static_cast<KisCanvas2*>(canvas());
        KisViewManager* viewManager = kiscanvas->viewManager();
        viewManager->nodeManager()->createNode("KisColorizeMask");
    }
}

void KisToolLazyBrush::activatePrimaryAction()
{
    KisToolFreehand::activatePrimaryAction();

    if (shouldActivateKeyStrokes() ||
        (!colorizeMaskActive() && canCreateColorizeMask())) {

        useCursor(KisCursor::handCursor());
        m_d->activateMaskMode = true;
        setOutlineEnabled(false);
    }
}

void KisToolLazyBrush::deactivatePrimaryAction()
{
    if (m_d->activateMaskMode) {
        m_d->activateMaskMode = false;
        setOutlineEnabled(true);
        resetCursorStyle();
    }

    KisToolFreehand::deactivatePrimaryAction();
}

void KisToolLazyBrush::beginPrimaryAction(KoPointerEvent *event)
{
    if (m_d->activateMaskMode) {
        if (!colorizeMaskActive() && canCreateColorizeMask()) {
            tryCreateColorizeMask();
        } else if (shouldActivateKeyStrokes()) {
            KisNodeSP node = currentNode();

            KIS_SAFE_ASSERT_RECOVER_NOOP(!m_d->manuallyActivatedNode ||
                                         m_d->manuallyActivatedNode == node);

            KisLayerPropertiesIcons::setNodeProperty(node,
                                                     KisLayerPropertiesIcons::colorizeEditKeyStrokes,
                                                     true, image());
            m_d->manuallyActivatedNode = node;
        }
    } else {
        KisToolFreehand::beginPrimaryAction(event);
    }
}

void KisToolLazyBrush::continuePrimaryAction(KoPointerEvent *event)
{
    if (m_d->activateMaskMode) return;
    KisToolFreehand::continuePrimaryAction(event);
}

void KisToolLazyBrush::endPrimaryAction(KoPointerEvent *event)
{
    if (m_d->activateMaskMode) return;
    KisToolFreehand::endPrimaryAction(event);
}

void KisToolLazyBrush::activateAlternateAction(KisTool::AlternateAction action)
{
    if (action == KisTool::Secondary && !m_d->activateMaskMode) {
        KisNodeSP node = currentNode();
        if (!node) return;

        m_d->oldShowKeyStrokesValue =
            KisLayerPropertiesIcons::nodeProperty(node,
                                                  KisLayerPropertiesIcons::colorizeEditKeyStrokes,
                                                  true).toBool();

        KisLayerPropertiesIcons::setNodeProperty(node,
                                                 KisLayerPropertiesIcons::colorizeEditKeyStrokes,
                                                 !m_d->oldShowKeyStrokesValue, image());

        KisToolFreehand::activatePrimaryAction();

    } else if (action == KisTool::Third && !m_d->activateMaskMode) {
        KisNodeSP node = currentNode();
        if (!node) return;

        m_d->oldShowColoringValue =
                KisLayerPropertiesIcons::nodeProperty(node,
                                                      KisLayerPropertiesIcons::colorizeShowColoring,
                                                      true).toBool();

        KisLayerPropertiesIcons::setNodeProperty(node,
                                                 KisLayerPropertiesIcons::colorizeShowColoring,
                                                 !m_d->oldShowColoringValue, image());

        KisToolFreehand::activatePrimaryAction();

    } else {
        KisToolFreehand::activateAlternateAction(action);
    }
}

void KisToolLazyBrush::deactivateAlternateAction(KisTool::AlternateAction action)
{
    if (action == KisTool::Secondary && !m_d->activateMaskMode) {
        KisNodeSP node = currentNode();
        if (!node) return;

        KisLayerPropertiesIcons::setNodeProperty(node,
                                                 KisLayerPropertiesIcons::colorizeEditKeyStrokes,
                                                 m_d->oldShowKeyStrokesValue, image());

        KisToolFreehand::deactivatePrimaryAction();

    } else if (action == KisTool::Third && !m_d->activateMaskMode) {
        KisNodeSP node = currentNode();
        if (!node) return;

        KisLayerPropertiesIcons::setNodeProperty(node,
                                                 KisLayerPropertiesIcons::colorizeShowColoring,
                                                 m_d->oldShowColoringValue, image());

        KisToolFreehand::deactivatePrimaryAction();

    } else {
        KisToolFreehand::deactivateAlternateAction(action);
    }
}

void KisToolLazyBrush::beginAlternateAction(KoPointerEvent *event, KisTool::AlternateAction action)
{
    if (!m_d->activateMaskMode && (action == KisTool::Secondary || action == KisTool::Third)) {
        beginPrimaryAction(event);
    } else {
        KisToolFreehand::beginAlternateAction(event, action);
    }
}

void KisToolLazyBrush::continueAlternateAction(KoPointerEvent *event, KisTool::AlternateAction action)
{
    if (!m_d->activateMaskMode && (action == KisTool::Secondary || action == KisTool::Third)) {
        continuePrimaryAction(event);
    } else {
        KisToolFreehand::continueAlternateAction(event, action);
    }
}

void KisToolLazyBrush::endAlternateAction(KoPointerEvent *event, KisTool::AlternateAction action)
{
    if (!m_d->activateMaskMode && (action == KisTool::Secondary || action == KisTool::Third)) {
        endPrimaryAction(event);
    } else {
        KisToolFreehand::endAlternateAction(event, action);
    }
}

void KisToolLazyBrush::explicitUserStrokeEndRequest()
{
    if (m_d->activateMaskMode) {
        tryCreateColorizeMask();
    } else if (colorizeMaskActive()) {
        KisNodeSP node = currentNode();
        if (!node) return;

        KisLayerPropertiesIcons::setNodeProperty(node, KisLayerPropertiesIcons::colorizeNeedsUpdate, false, image());
    }
}

QWidget * KisToolLazyBrush::createOptionWidget()
{
    KisCanvas2 * kiscanvas = dynamic_cast<KisCanvas2*>(canvas());

    QWidget *optionsWidget = new KisToolLazyBrushOptionsWidget(kiscanvas->viewManager()->canvasResourceProvider(), 0);
    optionsWidget->setObjectName(toolId() + "option widget");

    // // See https://bugs.kde.org/show_bug.cgi?id=316896
    // QWidget *specialSpacer = new QWidget(optionsWidget);
    // specialSpacer->setObjectName("SpecialSpacer");
    // specialSpacer->setFixedSize(0, 0);
    // optionsWidget->layout()->addWidget(specialSpacer);


    return optionsWidget;
}


