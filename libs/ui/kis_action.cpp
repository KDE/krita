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


#include "kis_action.h"
#include "kis_action_manager.h"
#include <QEvent>

class Q_DECL_HIDDEN KisAction::Private {

public:
    Private() : flags(NONE), conditions(NO_CONDITION), actionManager(0) {}

    ActivationFlags flags;
    ActivationConditions conditions;
    QStringList excludedNodeTypes;
    QString operationID;
    KisActionManager* actionManager;
};

KisAction::KisAction(QObject* parent)
    : QWidgetAction(parent)
    , d(new Private)
{
    connect(this, SIGNAL(changed()), SLOT(slotChanged()));
}

KisAction::KisAction(const QString& text, QObject* parent)
    : QWidgetAction(parent)
    , d(new KisAction::Private)
{
    QAction::setText(text);
    connect(this, SIGNAL(changed()), SLOT(slotChanged()));
}

KisAction::KisAction(const QIcon &icon, const QString& text, QObject* parent)
    : QWidgetAction(parent)
    , d(new Private)
{
    QAction::setIcon(icon);
    QAction::setText(text);
    connect(this, SIGNAL(changed()), SLOT(slotChanged()));
}

KisAction::~KisAction()
{
    delete d;
}

// Using a dynamic QObject property is done for compatibility with KAction and
// XmlGui. We may merge KisAction into the XmlGui code to make this unnecessary,
// but that is probably a lot of work for little benefit. We currently store a
// single default shortcut, but the old system used a list (to store default
// primary/alternate shortcuts for local and global settings) so we marshal it
// for compatibility.
void KisAction::setDefaultShortcut(const QKeySequence &shortcut)
{
    QList<QKeySequence> listifiedShortcut;
    // Use the empty list to represent no shortcut
    if (shortcut != QKeySequence("")) {
        listifiedShortcut.append(shortcut);
    }
    setProperty("defaultShortcuts", qVariantFromValue(listifiedShortcut));
}

QKeySequence KisAction::defaultShortcut() const
{
    auto listifiedShortcut = property("defaultShortcuts").value<QList<QKeySequence> >();
    if (listifiedShortcut.isEmpty()) {
        return QKeySequence();
    } else {
        return listifiedShortcut.first();
    }
}

void KisAction::setActivationFlags(KisAction::ActivationFlags flags)
{
    d->flags = flags;
}

KisAction::ActivationFlags KisAction::activationFlags()
{
    return d->flags;
}

void KisAction::setActivationConditions(KisAction::ActivationConditions conditions)
{
    d->conditions = conditions;
}

KisAction::ActivationConditions KisAction::activationConditions()
{
    return d->conditions;
}

void KisAction::setExcludedNodeTypes(const QStringList &nodeTypes)
{
    d->excludedNodeTypes = nodeTypes;
}

const QStringList& KisAction::excludedNodeTypes() const
{
    return d->excludedNodeTypes;
}

void KisAction::setActionEnabled(bool enabled)
{
    setEnabled(enabled);
}

void KisAction::setActionManager(KisActionManager* actionManager)
{
    d->actionManager = actionManager;
}

void KisAction::setOperationID(const QString& id)
{
    d->operationID = id;
    connect(this, SIGNAL(triggered()), this, SLOT(slotTriggered()));
}

void KisAction::slotTriggered()
{
    if (d->actionManager && !d->operationID.isEmpty()) {
        d->actionManager->runOperation(d->operationID);
    }
}

void KisAction::slotChanged()
{
    emit sigEnableSlaves(isEnabled());
}
