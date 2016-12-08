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

#include "KisActionsSnapshot.h"

#include "kis_action_registry.h"
#include "kactioncollection.h"

#include "kis_debug.h"


struct KisActionsSnapshot::Private
{
    QMap<QString, KActionCollection*> actionCollections;

    ~Private() {
        qDeleteAll(actionCollections);
    }
};

KisActionsSnapshot::KisActionsSnapshot()
    : m_d(new Private)
{

}

KisActionsSnapshot::~KisActionsSnapshot()
{
}

void KisActionsSnapshot::addAction(const QString &name, QAction *action)
{
    KisActionRegistry::ActionCategory cat = KisActionRegistry::instance()->fetchActionCategory(name);

    if (!cat.isValid()) {
        warnKrita << "WARNING: Uncategorized action" << name << "Dropping...";
        return;
    }

    KActionCollection *collection =  m_d->actionCollections[cat.componentName];

    if (!collection) {
        collection = new KActionCollection(0, cat.componentName);
        m_d->actionCollections.insert(cat.componentName, collection);
    }

    collection->addCategorizedAction(name, action, cat.categoryName);
}

QMap<QString, KActionCollection *> KisActionsSnapshot::actionCollections() const
{
    return m_d->actionCollections;
}

