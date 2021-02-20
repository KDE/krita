/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
