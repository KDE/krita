/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_select_layer_action.h"

#include <kis_debug.h>
#include <QMouseEvent>
#include <QApplication>
#include <QMenu>

#include <klocalizedstring.h>

#include <kis_canvas2.h>
#include <kis_image.h>
#include <KisViewManager.h>
#include <kis_node_manager.h>
#include <kis_cursor.h>

#include "kis_input_manager.h"
#include "kis_tool_utils.h"
#include <kis_group_layer.h>

#include <kis_assert.h>

class KisSelectLayerAction::Private
{
public:
    KisSelectLayerAction *q {nullptr};
    int shortcut {makeShortcut(LayerSelectionMode_TopLayer, SelectionOverrideMode_Replace)};

    Private(KisSelectLayerAction *q)
        : q(q)
    {}

    static int makeShortcut(LayerSelectionMode layerSelectionMode, SelectionOverrideMode selectionOverrideMode)
    {
        // Store the layer selection mode on the second byte and the selection override mode on the first one
        return (layerSelectionMode << 8) | selectionOverrideMode;
    }

    static int layerSelectionMode(int shortcut)
    {
        // Get the layer selection mode from the second byte
        return (shortcut >> 8) & 0xFF;
    }

    static int selectionOverrideMode(int shortcut)
    {
        // Get the selection override mode from the first byte
        return shortcut & 0xFF;
    }

    void selectNodes(const KisNodeList &nodesToSelect, int selectionOverrideMode, bool includeGroups) const
    {
        KisNodeManager *nodeManager = q->inputManager()->canvas()->viewManager()->nodeManager();
        KisNodeSP activeNode = nodeManager->activeNode();
        KisNodeList finalSelectedNodes;

        // Make the final list of nodes to select, excluding the group layers,
        // if needed
        if (includeGroups) {
            finalSelectedNodes = nodesToSelect;
        } else {
            Q_FOREACH(KisNodeSP node, nodesToSelect) {
                if (!dynamic_cast<KisGroupLayer*>(node.data())) {
                    finalSelectedNodes.append(node);
                }
            }
        }

        // Expand the group layers that contain newly selected nodes
        Q_FOREACH(KisNodeSP node, finalSelectedNodes) {
            KisNodeSP tmpNode = node->parent();
            while (tmpNode) {
                if (dynamic_cast<KisGroupLayer*>(tmpNode.data())) {
                    tmpNode->setCollapsed(false);
                }
                tmpNode = tmpNode->parent();
            }
        }

        // Combine the list of nodes with the current selection
        if (selectionOverrideMode == SelectionOverrideMode_Add) {
            KisNodeList currentlySelectedNodes = nodeManager->selectedNodes();
            Q_FOREACH(KisNodeSP node, currentlySelectedNodes) {
                if (!finalSelectedNodes.contains(node)) {
                    finalSelectedNodes.append(node);
                }
            }
        }

        // Try to retain the previously selected node or select the top one otherwise
        if (!finalSelectedNodes.contains(activeNode)) {
            activeNode = finalSelectedNodes.last();
        }

        // Select
        nodeManager->slotImageRequestNodeReselection(activeNode, finalSelectedNodes);
    }

    void selectNode(KisNodeSP node, int selectionOverrideMode) const
    {
        KisNodeList nodesToSelect;
        nodesToSelect.append(node);
        selectNodes(nodesToSelect, selectionOverrideMode, true);
    }
};

KisSelectLayerAction::KisSelectLayerAction()
    : KisAbstractInputAction("Select Layer")
    , d(new Private(this))
{
    setName(i18n("Select Layer"));
    setDescription(i18n("Select layers under the cursor position"));

    QHash<QString, int> shortcuts;
    shortcuts.insert(i18n("Select Top Layer (Replace Selection)"),
                     d->makeShortcut(LayerSelectionMode_TopLayer, SelectionOverrideMode_Replace));
    shortcuts.insert(i18n("Select All Layers (Replace Selection)"),
                     d->makeShortcut(LayerSelectionMode_AllLayers, SelectionOverrideMode_Replace));
    shortcuts.insert(i18n("Select from Menu (Replace Selection)"),
                     d->makeShortcut(LayerSelectionMode_Ask, SelectionOverrideMode_Replace));
    shortcuts.insert(i18n("Select Top Layer (Add to Selection)"),
                     d->makeShortcut(LayerSelectionMode_TopLayer, SelectionOverrideMode_Add));
    shortcuts.insert(i18n("Select All Layers (Add to Selection)"),
                     d->makeShortcut(LayerSelectionMode_AllLayers, SelectionOverrideMode_Add));
    shortcuts.insert(i18n("Select from Menu (Add to Selection)"),
                     d->makeShortcut(LayerSelectionMode_Ask, SelectionOverrideMode_Add));
    setShortcutIndexes(shortcuts);
}

KisSelectLayerAction::~KisSelectLayerAction()
{
    delete d;
}

int KisSelectLayerAction::priority() const
{
    return 5;
}

void KisSelectLayerAction::activate(int shortcut)
{
    Q_UNUSED(shortcut);
    QApplication::setOverrideCursor(KisCursor::pickLayerCursor());
}

void KisSelectLayerAction::deactivate(int shortcut)
{
    Q_UNUSED(shortcut);
    QApplication::restoreOverrideCursor();
}

void KisSelectLayerAction::begin(int shortcut, QEvent *event)
{
    KisAbstractInputAction::begin(shortcut, event);

    d->shortcut = shortcut;
    inputEvent(event);
}

void KisSelectLayerAction::inputEvent(QEvent *event)
{
    // Event not recognized
    if (!event || (event->type() != QEvent::MouseMove && event->type() != QEvent::TabletMove &&
                   event->type() != QTouchEvent::TouchUpdate && event->type() != QEvent::MouseButtonPress &&
                   event->type() != QEvent::TabletPress && event->type() != QTouchEvent::TouchBegin)) {
        return;
    }

    const int layerSelectionMode = d->layerSelectionMode(d->shortcut);
    const int selectionOverrideMode = d->selectionOverrideMode(d->shortcut);

    // Shortcut not recognized
    KIS_SAFE_ASSERT_RECOVER_RETURN(
        (layerSelectionMode == LayerSelectionMode_TopLayer ||
         layerSelectionMode == LayerSelectionMode_AllLayers ||
         layerSelectionMode == LayerSelectionMode_Ask) &&
        (selectionOverrideMode == SelectionOverrideMode_Replace ||
         selectionOverrideMode == SelectionOverrideMode_Add)
    );

    QPoint pos =
        inputManager()->canvas()->
        coordinatesConverter()->widgetToImage(eventPosF(event)).toPoint();

    // First make a list with the nodes to be selected
    KisNodeList nodesToSelect;

    if (layerSelectionMode == LayerSelectionMode_TopLayer) {
        KisNodeSP foundNode = KisToolUtils::findNode(inputManager()->canvas()->image()->root(), pos, false);
        if (!foundNode) {
            return;
        }
        nodesToSelect.append(foundNode);
    } else {
        // Retrieve group nodes only if the mode is LayerSelectionMode_Ask
        const KisNodeList foundNodes = KisToolUtils::findNodes(
                                            inputManager()->canvas()->image()->root()->firstChild(),
                                            pos, false, layerSelectionMode == LayerSelectionMode_Ask);

        if (foundNodes.isEmpty()) {
            return;
        }

        if (layerSelectionMode == LayerSelectionMode_AllLayers) {
            nodesToSelect = foundNodes;
        } else { //LayerSelectionMode_Ask
            QWidget *canvasWidget = inputManager()->canvas()->canvasWidget();
            QMenu *menu = new QMenu(canvasWidget);
            menu->setAttribute(Qt::WA_DeleteOnClose);
            int numberOfLayers = 0;

            // Traverse the list in reverse order so that the menu entries order
            // resembles that of the layer stack
            for (int i = foundNodes.size() - 1; i >= 0; --i) {
                KisNodeSP node = foundNodes[i];
                int indentation = -1;
                {
                    KisNodeSP tempNode = node;
                    while (tempNode->parent()) {
                        ++indentation;
                        tempNode = tempNode->parent();
                    }
                }
                QAction *action = menu->addAction(QString(4 * indentation, ' ') + node->name());
                QObject::connect(action, &QAction::triggered,
                    [this, node, selectionOverrideMode]()
                    {
                        d->selectNode(node, selectionOverrideMode);
                    }
                );
                if (!dynamic_cast<KisGroupLayer*>(node.data())) {
                    ++numberOfLayers;
                }
            }

            // Add separator
            menu->addSeparator();

            // Add "select all layers" menu item
            {
                QAction *action = menu->addAction(i18nc("Menu entry for the select layer under cursor canvas input action",
                                                        "Select all layers"));
                action->setVisible(numberOfLayers > 1);
                QObject::connect(action, &QAction::triggered,
                    [this, foundNodes, selectionOverrideMode]()
                    {
                        d->selectNodes(foundNodes, selectionOverrideMode, false);
                    }
                );
            }

            menu->popup(canvasWidget->mapToGlobal(eventPos(event)));
            return;
        }
    }

    // Now select the nodes
    d->selectNodes(nodesToSelect, selectionOverrideMode, true);
}
