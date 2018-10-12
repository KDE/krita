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
#include "./kactioncollection.h"

#include "kis_debug.h"

//#define ACTIONS_CHECKSUM_SANITY_CHECK


struct KisActionsSnapshot::Private
{
    QMap<QString, KActionCollection*> actionCollections;

    ~Private() {
        qDeleteAll(actionCollections);
        qDeleteAll(fakeActions);
    }

    QSet<QString> nonRegisteredShortcuts;
    QVector<QAction*> fakeActions;
};

KisActionsSnapshot::KisActionsSnapshot()
    : m_d(new Private)
{
    m_d->nonRegisteredShortcuts =
        QSet<QString>::fromList(
            KisActionRegistry::instance()->registeredShortcutIds());
}

KisActionsSnapshot::~KisActionsSnapshot()
{
}

void KisActionsSnapshot::addAction(const QString &name, QAction *action)
{
    m_d->nonRegisteredShortcuts.remove(name);
    KisActionRegistry::ActionCategory cat = KisActionRegistry::instance()->fetchActionCategory(name);

    if (!cat.isValid()) {
        warnKrita << "WARNING: Uncategorized action" << name << "Dropping...";
        return;
    }

#ifdef ACTIONS_CHECKSUM_SANITY_CHECK
    if (!KisActionRegistry::instance()->sanityCheckPropertized(action->objectName())) {
        warnKrita << "WARNING: action" << name  << "was not propertized!"  << ppVar(action->property("isShortcutConfigurable").toBool());
    }
#endif /* ACTIONS_CHECKSUM_SANITY_CHECK */

    KActionCollection *collection =  m_d->actionCollections[cat.componentName];

    if (!collection) {
        collection = new KActionCollection(0, cat.componentName);
        m_d->actionCollections.insert(cat.componentName, collection);
    }

    collection->addCategorizedAction(name, action, cat.categoryName);
}

QMap<QString, KActionCollection *> KisActionsSnapshot::actionCollections()
{
    /**
     * A small heuristics to show warnings only when unknown shortcuts appear
     * in the non-registered list
     */
    if (m_d->nonRegisteredShortcuts.size() > 4 &&
        m_d->nonRegisteredShortcuts.size() < 160) {

        warnKrita << "WARNING: The following shortcuts are not registered in the collection, "
                     "they might have wrong shortcuts in the end:";
        Q_FOREACH (const QString &str, m_d->nonRegisteredShortcuts) {
            warnKrita << str;
        }
        warnKrita << "=== end ===";
    }

    // try to workaround non-registered shortcuts by faking them manually
    Q_FOREACH (const QString &str, m_d->nonRegisteredShortcuts) {
        QAction *action = KisActionRegistry::instance()->makeQAction(str, 0);
        m_d->fakeActions << action;
        addAction(action->objectName(), action);
    }

    return m_d->actionCollections;
}

