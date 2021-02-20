/* SPDX-FileCopyrightText: 2008 Michael Jansen <kde@michael-jansen.biz>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kactioncategory.h"
#include "config-xmlgui.h"
#include <QAction>
#include "kstandardaction.h"

struct KActionCategoryPrivate {

    KActionCategoryPrivate(KActionCategory *host);

    //! Our host
    KActionCategory *q;

    //! The text for this category
    QString text;

    //! List of actions
    QList<QAction *> actions;

}; // class KActionCategoryPrivate

KActionCategory::KActionCategory(const QString &text, KActionCollection *parent)
    :   QObject(parent)
    , d(new KActionCategoryPrivate(this))
{
    d->text = text;
}

KActionCategory::~KActionCategory()
{
    delete d;
}

const QList<QAction *> KActionCategory::actions() const
{
    return d->actions;
}

QAction *KActionCategory::addAction(const QString &name, QAction *action)
{
    collection()->addAction(name, action);
    addAction(action);
    return action;
}

QAction *KActionCategory::addAction(
    KStandardAction::StandardAction actionType,
    const QObject *receiver,
    const char *member)
{
    QAction *action = collection()->addAction(actionType, receiver, member);
    addAction(action);
    return action;
}

QAction *KActionCategory::addAction(
    KStandardAction::StandardAction actionType,
    const QString &name,
    const QObject *receiver,
    const char *member)
{
    QAction *action = collection()->addAction(actionType, name, receiver, member);
    addAction(action);
    return action;
}

QAction *KActionCategory::addAction(
    const QString &name,
    const QObject *receiver,
    const char *member)
{
    QAction *action = collection()->addAction(name, receiver, member);
    addAction(action);
    return action;
}

void KActionCategory::addAction(QAction *action)
{
    // Only add the action if wasn't added earlier.
    if (!d->actions.contains(action)) {
        d->actions.append(action);
    }
}

KActionCollection *KActionCategory::collection() const
{
    return qobject_cast<KActionCollection *>(parent());
}

QString KActionCategory::text() const
{
    return d->text;
}

void KActionCategory::setText(const QString &text)
{
    d->text = text;
}

void KActionCategory::unlistAction(QAction *action)
{
    // ATTENTION:
    //   This method is called from KActionCollection with an QObject formerly
    //   known as a QAction during _k_actionDestroyed(). So don't do fancy stuff
    //   here that needs a real QAction!

    // Get the index for the action
    int index = d->actions.indexOf(action);

    // Action not found.
    if (index == -1) {
        return;
    }

    // Remove the action
    d->actions.takeAt(index);
}

KActionCategoryPrivate::KActionCategoryPrivate(KActionCategory *host)
    : q(host)
{}

