/*
 *  Copyright (c) 2013 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_action_manager.h"

#include <QList>
#include <kstandarddirs.h>
#include <kactioncollection.h>

#include "KisPart.h"
#include "kis_action.h"
#include "KisViewManager.h"
#include "kis_selection_manager.h"
#include "operations/kis_operation_ui_factory.h"
#include "operations/kis_operation_registry.h"
#include "operations/kis_operation.h"
#include "kis_layer.h"
#include "KisDocument.h"




class Q_DECL_HIDDEN KisActionManager::Private {

public:
    Private()
        : viewManager(0)
    {}

    KisViewManager* viewManager;
    QList<KisAction*> actions;
    KoGenericRegistry<KisOperationUIFactory*> uiRegistry;
    KisOperationRegistry operationRegistry;

};

KisActionManager::KisActionManager(KisViewManager* viewManager)
    : d(new Private)
{
    d->viewManager = viewManager;


}

KisActionManager::~KisActionManager()
{
    delete d;
}

void KisActionManager::setView(QPointer<KisView> imageView)
{
    Q_UNUSED(imageView);
}

void KisActionManager::addAction(const QString& name, KisAction* action)
{
    Q_ASSERT(!name.isEmpty());
    Q_ASSERT(action);
    Q_ASSERT(d->viewManager);
    Q_ASSERT(d->viewManager->actionCollection());

    d->viewManager->actionCollection()->addAction(name, action);
    action->setObjectName(name);
    action->setParent(d->viewManager->actionCollection());
    d->actions.append(action);
    action->setActionManager(this);
}

void KisActionManager::takeAction(KisAction* action)
{
    d->actions.removeOne(action);

    if (!action->objectName().isEmpty()) {
        KIS_ASSERT_RECOVER_RETURN(d->viewManager->actionCollection());
        d->viewManager->actionCollection()->takeAction(action);
    }
}

KisAction *KisActionManager::actionByName(const QString &name) const
{
    foreach(KisAction *action, d->actions) {
        if (action->objectName() == name) {
            return action;
        }
    }
    return 0;
}

void KisActionManager::updateGUI()
{
    //TODO other flags
    KisAction::ActivationFlags flags;

    KisImageWSP image;
    KisNodeSP node;
    KisLayerSP layer;
    KisPaintDeviceSP device;
    KisDocument* document = 0;
    KisSelectionManager* selectionManager = 0;
    KisAction::ActivationConditions conditions = KisAction::NO_CONDITION;

    if (d->viewManager) {

        // if there are no views, that means no document is open.
        // we cannot have nodes (selections), devices, or documents without a view
        if ( d->viewManager->viewCount() > 0 )
        {
            image = d->viewManager->image();
            flags |= KisAction::ACTIVE_IMAGE;

            node = d->viewManager->activeNode();
            device = d->viewManager->activeDevice();
            document = d->viewManager->document();
            selectionManager = d->viewManager->selectionManager();

            if (d->viewManager->viewCount() > 1) {
                flags |= KisAction::MULTIPLE_IMAGES;
            }

            if (document && document->isModified()) {
                flags |= KisAction::CURRENT_IMAGE_MODIFIED;
            }

            if (device) {
                flags |= KisAction::ACTIVE_DEVICE;
            }
        }

    }

    // is there a selection/mask?
    // you have to have at least one view(document) open for this to be true
    if (node) {

        // if a node exists, we know there is an active layer as well
        flags |= KisAction::ACTIVE_NODE;

        layer = dynamic_cast<KisLayer*>(node.data());
        if (layer) {
            flags |= KisAction::ACTIVE_LAYER;
        }

        if (node->inherits("KisTransparencyMask")) {
            flags |= KisAction::ACTIVE_TRANSPARENCY_MASK;
        }


        if (layer && layer->inherits("KisShapeLayer")) {
            flags |= KisAction::ACTIVE_SHAPE_LAYER;
        }

        if (selectionManager)
        {
            if (selectionManager->havePixelsSelected()) {
                flags |= KisAction::PIXELS_SELECTED;
            }

            if (selectionManager->haveShapesSelected()) {
                flags |= KisAction::SHAPES_SELECTED;
            }

            if (selectionManager->havePixelSelectionWithPixels()) {
                flags |= KisAction::PIXEL_SELECTION_WITH_PIXELS;
            }

            if (selectionManager->havePixelsInClipboard()) {
                flags |= KisAction::PIXELS_IN_CLIPBOARD;
            }

            if (selectionManager->haveShapesInClipboard()) {
                flags |= KisAction::SHAPES_IN_CLIPBOARD;
            }
        }

        if (node->isEditable(false)) {
            conditions |= KisAction::ACTIVE_NODE_EDITABLE;
        }

        if (node->hasEditablePaintDevice()) {
            conditions |= KisAction::ACTIVE_NODE_EDITABLE_PAINT_DEVICE;
        }

        if (d->viewManager->selectionEditable()) {
            conditions |= KisAction::SELECTION_EDITABLE;
        }
    }



    // loop through all actions in action manager and determine what should be enabled
    foreach(KisAction* action, d->actions) {
        bool enable;

        if (action->activationFlags() == KisAction::NONE) {
            enable = true;
        }
        else {
            enable = action->activationFlags() & flags; // combine action flags with updateGUI flags
        }

        enable = enable && (int)(action->activationConditions() & conditions) == (int)action->activationConditions();
        
        if (node && enable) {
            foreach (const QString &type, action->excludedNodeTypes()) {
                if (node->inherits(type.toLatin1())) {
                    enable = false;
                    break;
                }
            }
        }

        action->setActionEnabled(enable);
    }
}

KisAction *KisActionManager::createStandardAction(KStandardAction::StandardAction actionType, const QObject *receiver, const char *member)
{
    QAction *standardAction = KStandardAction::create(actionType, receiver, member, 0);
    KisAction *action = new KisAction(KIcon(standardAction->icon()), standardAction->text());
<<<<<<< HEAD
    const QList<QKeySequence> defaultShortcuts = standardAction->property("defaultShortcuts").value<QList<QKeySequence> >();
    const QKeySequence defaultShortcut = defaultShortcuts.isEmpty() ? QKeySequence() : defaultShortcuts.at(0);
    action->setShortcut(defaultShortcut, KAction::DefaultShortcut);
    action->setShortcut(standardAction->shortcut(), KAction::ActiveShortcut);
=======
    action->setShortcut(standardAction->shortcut(KAction::DefaultShortcut), KAction::DefaultShortcut);
    action->setShortcut(standardAction->shortcut(KAction::ActiveShortcut), KAction::ActiveShortcut);
#ifdef Q_OS_WIN
    if (actionType == KStandardAction::SaveAs && standardAction->shortcut(KAction::DefaultShortcut).primary().isEmpty()) {
        action->setShortcut(QKeySequence("CTRL+SHIFT+S"), KAction::DefaultShortcut);
    }
#endif
>>>>>>> calligra/2.9
    action->setCheckable(standardAction->isCheckable());
    if (action->isCheckable()) {
        action->setChecked(standardAction->isChecked());
    }
    action->setMenuRole(standardAction->menuRole());
    action->setText(standardAction->text());
    action->setToolTip(standardAction->toolTip());

    if (receiver && member) {
        if (actionType == KStandardAction::OpenRecent) {
            QObject::connect(action, SIGNAL(urlSelected(KUrl)), receiver, member);
        }
        else if (actionType == KStandardAction::ConfigureToolbars) {
            QObject::connect(action, SIGNAL(triggered(bool)), receiver, member, Qt::QueuedConnection);
        }
        else {
            QObject::connect(action, SIGNAL(triggered(bool)), receiver, member);
        }
    }

    addAction(standardAction->objectName(), action);
    delete standardAction;
    return action;
}

void KisActionManager::registerOperationUIFactory(KisOperationUIFactory* factory)
{
    d->uiRegistry.add(factory);
}

void KisActionManager::registerOperation(KisOperation* operation)
{
    d->operationRegistry.add(operation);
}

void KisActionManager::runOperation(const QString& id)
{
    KisOperationConfiguration* config = new KisOperationConfiguration(id);

    KisOperationUIFactory* uiFactory = d->uiRegistry.get(id);
    if (uiFactory) {
        bool gotConfig = uiFactory->fetchConfiguration(d->viewManager, config);
        if (!gotConfig) {
            return;
        }
    }
    runOperationFromConfiguration(config);
}

void KisActionManager::runOperationFromConfiguration(KisOperationConfiguration* config)
{
    KisOperation* operation = d->operationRegistry.get(config->id());
    Q_ASSERT(operation);
    if (operation) {
        operation->runFromXML(d->viewManager, *config);
    }
    delete config;
}

void KisActionManager::dumpActionFlags()
{
    QFile data("actions.txt");
    if (data.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream out(&data);

        foreach(KisAction* action, d->actions) {
            KisAction::ActivationFlags flags = action->activationFlags();
            out << "-------- " << action->text() << " --------\n";
            out << "Action will activate on: \n";

            if (flags & KisAction::ACTIVE_IMAGE) {
                out << "    Active image\n";
            }
            if (flags & KisAction::MULTIPLE_IMAGES) {
                out << "    More than one image open\n";
            }
            if (flags & KisAction::CURRENT_IMAGE_MODIFIED) {
                out << "    Active image modified\n";
            }
            if (flags & KisAction::ACTIVE_DEVICE) {
                out << "    Active device\n";
            }
            if (flags & KisAction::ACTIVE_LAYER) {
                out << "    Active layer\n";
            }
            if (flags & KisAction::ACTIVE_TRANSPARENCY_MASK) {
                out << "    Active transparency mask\n";
            }
            if (flags & KisAction::ACTIVE_NODE) {
                out << "    Active node\n";
            }
            if (flags & KisAction::ACTIVE_SHAPE_LAYER) {
                out << "    Active shape layer\n";
            }
            if (flags & KisAction::PIXELS_SELECTED) {
                out << "    Pixels selected\n";
            }
            if (flags & KisAction::SHAPES_SELECTED) {
                out << "    Shapes selected\n";
            }
            if (flags & KisAction::PIXEL_SELECTION_WITH_PIXELS) {
                out << "    Pixel selection with pixels\n";
            }
            if (flags & KisAction::PIXELS_IN_CLIPBOARD) {
                out << "    Pixels in clipboard\n";
            }
            if (flags & KisAction::SHAPES_IN_CLIPBOARD) {
                out << "    Shape in clipboard\n";
            }
            out << "\n\n";
            out << "Action will only activate if the following conditions are met: \n";
            KisAction::ActivationConditions conditions = action->activationConditions();
            if ((int)conditions == 0) {
                out << "    -\n";
            }
            if (conditions & KisAction::ACTIVE_NODE_EDITABLE) {
                out << "    Active Node editable\n";
            }
            if (conditions & KisAction::ACTIVE_NODE_EDITABLE_PAINT_DEVICE) {
                out << "    Active Node has editable paint device\n";
            }
            if (conditions & KisAction::SELECTION_EDITABLE) {
                out << "    Selection is editable\n";
            }
            out << "\n\n";
        }
    }
}
