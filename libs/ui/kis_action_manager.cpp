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
#include <kactioncollection.h>

#include <kis_icon.h>
#include "KisPart.h"
#include "kis_action.h"
#include "KisViewManager.h"
#include "kis_selection_manager.h"
#include "operations/kis_operation_ui_factory.h"
#include "operations/kis_operation_registry.h"
#include "operations/kis_operation.h"
#include "kis_layer.h"
#include "KisDocument.h"
#include "kis_clipboard.h"
#include <kis_image_animation_interface.h>
#include "kis_config.h"

#include <QMenu>
#include "QFile"
#include <QDomDocument>
#include <QDomElement>

class Q_DECL_HIDDEN KisActionManager::Private {

public:
    Private()
        : viewManager(0)
    {}

    ~Private()
    {
        qDeleteAll(uiRegistry.values());
    }

    KisViewManager* viewManager;
    KActionCollection *actionCollection;

    QList<QPointer<KisAction>> actions;
    KoGenericRegistry<KisOperationUIFactory*> uiRegistry;
    KisOperationRegistry operationRegistry;

};

KisActionManager::KisActionManager(KisViewManager* viewManager, KActionCollection *actionCollection)
    : d(new Private)
{
    d->viewManager = viewManager;
    d->actionCollection = actionCollection;

    connect(d->actionCollection,
            SIGNAL(inserted(QAction*)), SLOT(slotActionAddedToCollection(QAction*)));
}

KisActionManager::~KisActionManager()
{

#if 0
  if ((d->actions.size() > 0)) {

       QDomDocument doc;
       QDomElement e = doc.createElement("Actions");
       e.setAttribute("version", "2");
       doc.appendChild(e);

       Q_FOREACH (KisAction *action, d->actions) {
           QDomElement a = doc.createElement("Action");
           a.setAttribute("name", action->objectName());

           // But seriously, XML is the worst format ever designed
           auto addElement = [&](QString title, QString content) {
               QDomElement newNode = doc.createElement(title);
               QDomText    newText = doc.createTextNode(content);
               newNode.appendChild(newText);
               a.appendChild(newNode);
           };

           addElement("icon", action->icon().name());
           addElement("text", action->text());
           addElement("whatsThis" , action->whatsThis());
           addElement("toolTip" , action->toolTip());
           addElement("iconText" , action->iconText());
           addElement("shortcut" , action->shortcut().toString());
           addElement("activationFlags" , QString::number(action->activationFlags(),2));
           addElement("activationConditions" , QString::number(action->activationConditions(),2));
           addElement("defaultShortcut" , action->defaultShortcut().toString());
           addElement("isCheckable" , QString((action->isChecked() ? "true" : "false")));
           addElement("statusTip", action->statusTip());
           e.appendChild(a);
       }
       QFile f("ActionManager.action");
       f.open(QFile::WriteOnly);
       f.write(doc.toString().toUtf8());
       f.close();

   }
#endif
    delete d;
}

void KisActionManager::setView(QPointer<KisView> imageView)
{
    Q_UNUSED(imageView);
}

void KisActionManager::slotActionAddedToCollection(QAction *action)
{
    /**
     * Small hack alert: not all the actions are still created by the manager and
     * immediately added to the action collection. Some plugins add actions
     * directly to the action collection when a document is created. Here we
     * catch these cases
     */
    KisActionRegistry::instance()->updateShortcut(action->objectName(), action);
}

void KisActionManager::addAction(const QString& name, KisAction* action)
{
    Q_ASSERT(!name.isEmpty());
    Q_ASSERT(action);
    Q_ASSERT(d->viewManager);
    Q_ASSERT(d->actionCollection);

    d->actionCollection->addAction(name, action);
    action->setParent(d->actionCollection);

    d->actions.append(action);
    action->setActionManager(this);
}

void KisActionManager::takeAction(KisAction* action)
{
    d->actions.removeOne(action);

    if (!action->objectName().isEmpty()) {
        KIS_ASSERT_RECOVER_RETURN(d->actionCollection);
        d->actionCollection->takeAction(action);
    }
}

KisAction *KisActionManager::actionByName(const QString &name) const
{
    Q_FOREACH (KisAction *action, d->actions) {
        if (action->objectName() == name) {
            return action;
        }
    }
    return 0;
}


KisAction *KisActionManager::createAction(const QString &name)
{
    KisAction *a = actionByName(name); // Check if the action already exists
    if (a) {
        dbgAction << name << "already exists";
        return a;
    }

    // There is some tension here. KisActionManager is supposed to be in control
    // of global actions, but these actions are supposed to be duplicated. We
    // will add them to the KisActionRegistry for the time being so we can get
    // properly categorized shortcuts.
    a = new KisAction();
    KisActionRegistry *actionRegistry = KisActionRegistry::instance();

    // Add extra properties
    actionRegistry->propertizeAction(name, a);
    bool ok; // We will skip this check
    int activationFlags = actionRegistry->getActionProperty(name, "activationFlags").toInt(&ok, 2);
    int activationConditions = actionRegistry->getActionProperty(name, "activationConditions").toInt(&ok, 2);
    a->setActivationFlags((KisAction::ActivationFlags) activationFlags);
    a->setActivationConditions((KisAction::ActivationConditions) activationConditions);

    addAction(name, a);

    return a;
}

void KisActionManager::updateGUI()
{
    //TODO other flags
    KisAction::ActivationFlags flags;

    KisImageWSP image;
    KisNodeSP node;
    KisLayerSP layer;
    KisSelectionManager *selectionManager = 0;

    KisAction::ActivationConditions conditions = KisAction::NO_CONDITION;

    if (d->viewManager) {
        node = d->viewManager->activeNode();
        selectionManager = d->viewManager->selectionManager();

        // if there are no views, that means no document is open.
        // we cannot have nodes (selections), devices, or documents without a view
        if ( d->viewManager->viewCount() > 0 )
        {
            image = d->viewManager->image();
            flags |= KisAction::ACTIVE_IMAGE;

            if (image && image->animationInterface()->hasAnimation()) {
                flags |= KisAction::IMAGE_HAS_ANIMATION;
            }

            if (d->viewManager->viewCount() > 1) {
                flags |= KisAction::MULTIPLE_IMAGES;
            }

            if (d->viewManager->document() && d->viewManager->document()->isModified()) {
                flags |= KisAction::CURRENT_IMAGE_MODIFIED;
            }

            if (d->viewManager->activeDevice()) {
                flags |= KisAction::ACTIVE_DEVICE;
            }
        }

        if (d->viewManager->selectionEditable()) {
            conditions |= KisAction::SELECTION_EDITABLE;
        }

    }

    // is there a selection/mask?
    // you have to have at least one view (document) open for this to be true
    if (node) {

        // if a node exists, we know there is an active layer as well
        flags |= KisAction::ACTIVE_NODE;

        layer = qobject_cast<KisLayer*>(node.data());
        if (layer) {
            flags |= KisAction::ACTIVE_LAYER;
        }

        if (node->inherits("KisTransparencyMask")) {
            flags |= KisAction::ACTIVE_TRANSPARENCY_MASK;
        }


        if (layer && layer->inherits("KisShapeLayer")) {
            flags |= KisAction::ACTIVE_SHAPE_LAYER;
        }

        if (KisClipboard::instance()->hasLayers()) {
            flags |= KisAction::LAYERS_IN_CLIPBOARD;
        }

        if (selectionManager) {

            if (selectionManager->havePixelsSelected()) {
                flags |= KisAction::PIXELS_SELECTED;
            }

            if (selectionManager->haveShapesSelected()) {
                flags |= KisAction::SHAPES_SELECTED;
            }

            if (selectionManager->haveAnySelectionWithPixels()) {
                flags |= KisAction::ANY_SELECTION_WITH_PIXELS;
            }

            if (selectionManager->haveShapeSelectionWithShapes()) {
                flags |= KisAction::SHAPE_SELECTION_WITH_SHAPES;
            }

            if (selectionManager->haveRasterSelectionWithPixels()) {
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
    }

    KisConfig cfg(true);
    if (cfg.useOpenGL()) {
        conditions |= KisAction::OPENGL_ENABLED;
    }

    // loop through all actions in action manager and determine what should be enabled
    Q_FOREACH (QPointer<KisAction> action, d->actions) {
        bool enable;
        if (action) {
            if (action->activationFlags() == KisAction::NONE) {
                enable = true;
            }
            else {
                enable = action->activationFlags() & flags; // combine action flags with updateGUI flags
            }

            enable = enable && (int)(action->activationConditions() & conditions) == (int)action->activationConditions();

            if (node && enable) {
                Q_FOREACH (const QString &type, action->excludedNodeTypes()) {
                    if (node->inherits(type.toLatin1())) {
                        enable = false;
                        break;
                    }
                }
            }

            action->setActionEnabled(enable);
        }
    }
}

KisAction *KisActionManager::createStandardAction(KStandardAction::StandardAction actionType, const QObject *receiver, const char *member)
{
    QAction *standardAction = KStandardAction::create(actionType, receiver, member, 0);
    KisAction *action = new KisAction(standardAction->icon(), standardAction->text());

    const QList<QKeySequence> defaultShortcuts = standardAction->property("defaultShortcuts").value<QList<QKeySequence> >();
    const QKeySequence defaultShortcut = defaultShortcuts.isEmpty() ? QKeySequence() : defaultShortcuts.at(0);
    action->setDefaultShortcut(standardAction->shortcut());
#ifdef Q_OS_WIN
    if (actionType == KStandardAction::SaveAs && defaultShortcuts.isEmpty()) {
        action->setShortcut(QKeySequence("CTRL+SHIFT+S"));
    }
#endif
    action->setCheckable(standardAction->isCheckable());
    if (action->isCheckable()) {
        action->setChecked(standardAction->isChecked());
    }
    action->setMenuRole(standardAction->menuRole());
    action->setText(standardAction->text());
    action->setToolTip(standardAction->toolTip());

    if (receiver && member) {
        if (actionType == KStandardAction::OpenRecent) {
            QObject::connect(action, SIGNAL(urlSelected(QUrl)), receiver, member);
        }
        else if (actionType == KStandardAction::ConfigureToolbars) {
            QObject::connect(action, SIGNAL(triggered(bool)), receiver, member, Qt::QueuedConnection);
        }
        else {
            QObject::connect(action, SIGNAL(triggered(bool)), receiver, member);
        }
    }

    KisActionRegistry *actionRegistry = KisActionRegistry::instance();
    actionRegistry->propertizeAction(standardAction->objectName(), action);

    addAction(standardAction->objectName(), action);
    delete standardAction;
    return action;
}

void KisActionManager::safePopulateMenu(QMenu *menu, const QString &actionId, KisActionManager *actionManager)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(actionManager);

    KisAction *action = actionManager->actionByName(actionId);
    KIS_SAFE_ASSERT_RECOVER_RETURN(action);

    menu->addAction(action);
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
    KisOperationConfigurationSP config = new KisOperationConfiguration(id);

    KisOperationUIFactory* uiFactory = d->uiRegistry.get(id);
    if (uiFactory) {
        bool gotConfig = uiFactory->fetchConfiguration(d->viewManager, config);
        if (!gotConfig) {
            return;
        }
    }
    runOperationFromConfiguration(config);
}

void KisActionManager::runOperationFromConfiguration(KisOperationConfigurationSP config)
{
    KisOperation* operation = d->operationRegistry.get(config->id());
    Q_ASSERT(operation);
    operation->runFromXML(d->viewManager, *config);
}

void KisActionManager::dumpActionFlags()
{
    QFile data("actions.txt");
    if (data.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream out(&data);
        out.setCodec("UTF-8");

        Q_FOREACH (KisAction* action, d->actions) {
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
            if (flags & KisAction::ANY_SELECTION_WITH_PIXELS) {
                out << "    Any selection with pixels\n";
            }
            if (flags & KisAction::PIXELS_IN_CLIPBOARD) {
                out << "    Pixels in clipboard\n";
            }
            if (flags & KisAction::SHAPES_IN_CLIPBOARD) {
                out << "    Shape in clipboard\n";
            }
            if (flags & KisAction::IMAGE_HAS_ANIMATION) {
                out << "    Image has animation\n";
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
            if (conditions & KisAction::OPENGL_ENABLED) {
                out << "    OpenGL is enabled\n";
            }
            out << "\n\n";
        }
    }
}
