/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoToolManager_p.h"

#include <KoShapeManager.h>
#include <KoSelection.h>
#include <KoToolBase.h>
#include <KoToolFactoryBase.h>


static int newUniqueToolHelperId()
{
    static int idCounter = 0;
    return ++idCounter;
}

/*    ************ ToolHelper **********
 * This class wrangles the tool factory, toolbox button and switch-tool action
 * for a single tool. It assumes the  will continue to live once it is created.
 * (Hiding the toolbox is OK.)
 */

ToolHelper::ToolHelper(KoToolFactoryBase *tool)
    : m_toolFactory(tool),
      m_uniqueId(newUniqueToolHelperId()),
      m_hasCustomShortcut(false),
      m_toolAction(0)
{
    // TODO: how to get an existing custom shortcut in the beginning here?
    // Once the first ShortcutToolAction is added to the actionCollection,
    // it will get any custom shortcut set by the actionCollection and
    // by that trigger shortcutToolActionUpdated().
    // But until then shortcut() will report a wrong shortcut and e.g. show
    // that in the tooltips of the KoToolBox.
}

KoToolAction *ToolHelper::toolAction()
{
    // create lazily
    if (!m_toolAction) {
        m_toolAction = new KoToolAction(this);
    }
    return m_toolAction;
}

QString ToolHelper::id() const
{
    return m_toolFactory->id();
}

QString ToolHelper::activationShapeId() const
{
    return m_toolFactory->activationShapeId();
}

QString ToolHelper::iconName() const
{
    return m_toolFactory->iconName();
}

QString ToolHelper::text() const
{
    // TODO: add text property to KoToolFactoryBase
    return m_toolFactory->toolTip();
}

QString ToolHelper::iconText() const
{
    // TODO: add text iconText to KoToolFactoryBase
    return m_toolFactory->toolTip();
}

QString ToolHelper::toolTip() const
{
    return m_toolFactory->toolTip();
}

void ToolHelper::activate()
{
    emit toolActivated(this);
}

void ToolHelper::shortcutToolActionUpdated()
{
    ShortcutToolAction *action = static_cast<ShortcutToolAction*>(sender());
    // check if shortcut changed
    const QKeySequence actionShortcut = action->shortcut();
    const QKeySequence currentShortcut = shortcut();
    if (actionShortcut != currentShortcut) {
        m_hasCustomShortcut = true;
        m_customShortcut = actionShortcut;
        if (m_toolAction) {
            emit m_toolAction->changed();
        }
        // no need to forward the new shortcut to the other ShortcutToolAction objects,
        // they are synchronized behind the scenes
        // Thus they will also trigger this method, but due to them having
        // the same shortcut not result in any further action.
    }
}

KoToolBase *ToolHelper::createTool(KoCanvasBase *canvas) const
{
    KoToolBase *tool = m_toolFactory->createTool(canvas);
    if (tool) {
        tool->setToolId(id());
    }
    return tool;
}

ShortcutToolAction* ToolHelper::createShortcutToolAction(QObject *parent)
{
    ShortcutToolAction* action = new ShortcutToolAction(id(), text(), parent);
    action->setShortcut(shortcut());

    connect(action, SIGNAL(changed()), SLOT(shortcutToolActionUpdated()));

    return action;
}

QString ToolHelper::toolType() const
{
    return m_toolFactory->toolType();
}

int ToolHelper::priority() const
{
    return m_toolFactory->priority();
}

QKeySequence ToolHelper::shortcut() const
{
    if (m_hasCustomShortcut) {
        return m_customShortcut;
    }

    return m_toolFactory->shortcut();
}

//   ************ Connector **********
Connector::Connector(KoShapeManager *parent)
        : QObject(parent),
        m_shapeManager(parent)
{
    connect(m_shapeManager, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
}

void Connector::selectionChanged()
{
    emit selectionChanged(m_shapeManager->selection()->selectedShapes());
}

//   ************ ShortcutToolAction **********
ShortcutToolAction::ShortcutToolAction(const QString &id, const QString &name, QObject *parent)
    : QAction(name, parent)
    , m_toolID(id)
{
    connect(this, SIGNAL(triggered(bool)), this, SLOT(actionTriggered()));
}

ShortcutToolAction::~ShortcutToolAction()
{
}

void ShortcutToolAction::actionTriggered()
{
    // TODO: why not ToolHelper::activate(); and thus a slightly different behaviour?
    KoToolManager::instance()->switchToolRequested(m_toolID);
}

