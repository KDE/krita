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

#ifndef KISACTIONSSNAPSHOT_H
#define KISACTIONSSNAPSHOT_H

#include <kritawidgetutils_export.h>

#include <QScopedPointer>
#include <QMap>

class QAction;
class KActionCollection;


/**
 * @brief The KisActionsSnapshot class
 */
class KRITAWIDGETUTILS_EXPORT KisActionsSnapshot
{
public:
    KisActionsSnapshot();
    ~KisActionsSnapshot();

    /**
     * @brief registers the action in the snapshot and sorts it into a proper
     *        category. The action is *not* owned by the snapshot.
     *
     * @param name id string of the action
     * @param action the action itself
     */
    void addAction(const QString &name, QAction *action);

    /**
     * Returns all action collections of the current snapshot
     *
     * WARNING: the collections are owned by the snapshot! Don't destroy
     *          the snapshot before you are done with the collections!
     */
    QMap<QString, KActionCollection*> actionCollections();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISACTIONSSNAPSHOT_H
