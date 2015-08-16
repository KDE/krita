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
#include <QToolButton>
#include <kicon.h>
#include <klocale.h>

#include <QtGlobal> // for qrand()

/*    ************ ToolHelper **********
 * This class wrangles the tool factory, toolbox button and switch-tool action
 * for a single tool. It assumes the  will continue to live once it is created.
 * (Hiding the toolbox is OK.)
 */

ToolHelper::ToolHelper(KoToolFactoryBase *tool)
    : m_toolFactory(tool),
      m_uniqueId((int)qrand()),
      button(0),
      action(0)
{
}

QToolButton* ToolHelper::createButton()
{
    button = new QToolButton();
    button->setObjectName(m_toolFactory->id());
    button->setIcon(KIcon(m_toolFactory->iconName()));
    button->setToolTip(buttonToolTip());

    connect(button, SIGNAL(clicked()), this, SLOT(buttonPressed()));
    return button;
}

void ToolHelper::buttonPressed()
{
    emit toolActivated(this);
}

QString ToolHelper::id() const
{
    return m_toolFactory->id();
}

QString ToolHelper::activationShapeId() const
{
    return m_toolFactory->activationShapeId();
}

QString ToolHelper::toolTip() const
{
    return m_toolFactory->toolTip();
}

QString ToolHelper::buttonToolTip() const
{
  return shortcut().isEmpty() ?
    i18nc("@info:tooltip", "%1", toolTip()) :
    i18nc("@info:tooltip %2 is shortcut", "%1 (%2)", toolTip(),
          shortcut().toString());
}

void ToolHelper::actionUpdated()
{
    if (button)
        button->setToolTip(buttonToolTip());
}

KoToolBase *ToolHelper::createTool(KoCanvasBase *canvas) const
{
    KoToolBase *tool = m_toolFactory->createTool(canvas);
    if (tool) {
        tool->setToolId(id());
    }
    return tool;
}

QString ToolHelper::toolType() const
{
    return m_toolFactory->toolType();
}

int ToolHelper::priority() const
{
    return m_toolFactory->priority();
}

KShortcut ToolHelper::shortcut() const
{
    if (action) {
        return action->shortcut();
    }

    return m_toolFactory->shortcut();
}

void ToolHelper::setAction(KAction *a)
{
    action = a;
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

//   ************ ToolAction **********
ToolAction::ToolAction(KoToolManager* toolManager, const QString &id, const QString &name, QObject *parent)
    : KAction(name, parent),
    m_toolManager(toolManager),
    m_toolID(id)
{
    connect(this, SIGNAL(triggered(bool)), this, SLOT(actionTriggered()));
}

ToolAction::~ToolAction()
{
}

void ToolAction::actionTriggered()
{
    m_toolManager->switchToolRequested(m_toolID);
}



#include <KoToolManager_p.moc>
