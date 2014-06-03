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

class KisAction::Private {

public:
    Private() : flags(NONE), conditions(NO_CONDITION), actionManager(0) {}

    ActivationFlags flags;
    ActivationConditions conditions;
    QStringList excludedNodeTypes;
    QString operationID;
    KisActionManager* actionManager;
};

KisAction::KisAction(QObject* parent): KAction(parent), d(new Private)
{
    connect(this, SIGNAL(changed()), SLOT(slotChanged()));
}

KisAction::KisAction(const QString& text, QObject* parent): KAction(text, parent), d(new KisAction::Private)
{
    connect(this, SIGNAL(changed()), SLOT(slotChanged()));
}

KisAction::KisAction(const KIcon& icon, const QString& text, QObject* parent): KAction(icon, text, parent), d(new Private)
{
    connect(this, SIGNAL(changed()), SLOT(slotChanged()));
}

KisAction::~KisAction()
{
    delete d;
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
