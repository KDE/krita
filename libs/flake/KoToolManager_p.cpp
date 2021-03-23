/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoToolManager_p.h"

#include <KoShapeManager.h>
#include <KoSelection.h>
#include <KoToolBase.h>
#include <KoToolFactoryBase.h>
#include "kis_action_registry.h"

//   ************ KoToolAction::Private **********

class Q_DECL_HIDDEN KoToolAction::Private
{
public:
    KoToolFactoryBase *toolFactory;
};

KoToolAction::KoToolAction(KoToolFactoryBase* toolFactory)
    : d(new Private)
{
    d->toolFactory = toolFactory;
}

KoToolAction::~KoToolAction()
{
    delete d;
}

void KoToolAction::trigger()
{
    KoToolManager::instance()->switchToolRequested(id());
}


QString KoToolAction::iconText() const
{
    // There is no specific iconText in KoToolFactoryBase
    return d->toolFactory->toolTip();
}

QString KoToolAction::toolTip() const
{
    return d->toolFactory->toolTip();
}

QString KoToolAction::id() const
{
    return d->toolFactory->id();
}

QString KoToolAction::iconName() const
{
    return d->toolFactory->iconName();
}

QKeySequence KoToolAction::shortcut() const
{
    return d->toolFactory->shortcut();
}


QString KoToolAction::section() const
{
    return d->toolFactory->section();
}

int KoToolAction::priority() const
{
    return d->toolFactory->priority();
}

QString KoToolAction::visibilityCode() const
{
    return d->toolFactory->activationShapeId();
}

KoToolFactoryBase *KoToolAction::toolFactory() const
{
    return d->toolFactory;
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
