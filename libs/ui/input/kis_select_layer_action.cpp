/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_select_layer_action.h"

#include <kis_debug.h>
#include <QMouseEvent>
#include <QApplication>

#include <klocalizedstring.h>

#include <kis_canvas2.h>
#include <kis_image.h>
#include <KisViewManager.h>
#include <kis_node_manager.h>
#include <kis_cursor.h>

#include "kis_input_manager.h"
#include "kis_tool_utils.h"


class KisSelectLayerAction::Private
{
public:
    Private() : multipleMode(false) {}
    bool multipleMode;
};

KisSelectLayerAction::KisSelectLayerAction()
    : KisAbstractInputAction("Select Layer")
    , d(new Private)
{
    setName(i18n("Select Layer"));
    setDescription(i18n("Selects a layer under cursor position"));

    QHash<QString, int> shortcuts;
    shortcuts.insert(i18n("Select Layer Mode"), SelectLayerModeShortcut);
    shortcuts.insert(i18n("Select Multiple Layer Mode"), SelectMultipleLayerModeShortcut);
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

    switch (shortcut) {
    case SelectMultipleLayerModeShortcut:
    case SelectLayerModeShortcut:
        d->multipleMode = shortcut == SelectMultipleLayerModeShortcut;
        inputEvent(event);
        break;
    }
}

void KisSelectLayerAction::inputEvent(QEvent *event)
{
    if (event &&
        (event->type() == QEvent::MouseMove || event->type() == QEvent::TabletMove ||
         event->type() == QEvent::MouseButtonPress || event->type() == QEvent::TabletPress)) {

        QPoint pos =
            inputManager()->canvas()->
            coordinatesConverter()->widgetToImage(eventPosF(event)).toPoint();

        KisNodeSP node = KisToolUtils::findNode(inputManager()->canvas()->image()->root(), pos, false);
        if (!node) return;

        KisNodeManager *nodeManager = inputManager()->canvas()->viewManager()->nodeManager();

        if (!d->multipleMode) {
            nodeManager->slotNonUiActivatedNode(node);
        } else {
            KisNodeList nodes = nodeManager->selectedNodes();
            if (!nodes.contains(node)) {
                nodes.append(node);
            }
            nodeManager->slotImageRequestNodeReselection(node, nodes);
        }
    }
}
