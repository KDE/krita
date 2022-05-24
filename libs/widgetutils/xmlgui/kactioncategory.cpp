/* SPDX-FileCopyrightText: 2008 Michael Jansen <kde@michael-jansen.biz>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kactioncategory.h"
#include "config-xmlgui.h"
#include <QAction>
#include "kstandardaction.h"

struct KisKActionCategoryPrivate {

    KisKActionCategoryPrivate(KisKActionCategory *host);

    //! Our host
    KisKActionCategory *q;

    //! The text for this category
    QString text;

    //! List of actions
    QList<QAction *> actions;

}; // class KisKActionCategoryPrivate

KisKActionCategory::KisKActionCategory(const QString &text, KisKActionCollection *parent)
    :   QObject(parent)
    , d(new KisKActionCategoryPrivate(this))
{
    d->text = text;
}

KisKActionCategory::~KisKActionCategory()
{
    delete d;
}

const QList<QAction *> KisKActionCategory::actions() const
{
    return d->actions;
}

QAction *KisKActionCategory::addAction(const QString &name, QAction *action)
{
    collection()->addAction(name, action);
    addAction(action);
    return action;
}

QAction *KisKActionCategory::addAction(
    KStandardAction::StandardAction actionType,
    const QObject *receiver,
    const char *member)
{
    QAction *action = collection()->addAction(actionType, receiver, member);
    addAction(action);
    return action;
}

QAction *KisKActionCategory::addAction(
    KStandardAction::StandardAction actionType,
    const QString &name,
    const QObject *receiver,
    const char *member)
{
    QAction *action = collection()->addAction(actionType, name, receiver, member);
    addAction(action);
    return action;
}

QAction *KisKActionCategory::addAction(
    const QString &name,
    const QObject *receiver,
    const char *member)
{
    QAction *action = collection()->addAction(name, receiver, member);
    addAction(action);
    return action;
}

void KisKActionCategory::addAction(QAction *action)
{
    // Only add the action if wasn't added earlier.
    if (!d->actions.contains(action)) {
        d->actions.append(action);
    }
}

KisKActionCollection *KisKActionCategory::collection() const
{
    return qobject_cast<KisKActionCollection *>(parent());
}

QString KisKActionCategory::text() const
{
    return d->text;
}

void KisKActionCategory::setText(const QString &text)
{
    d->text = text;
}

void KisKActionCategory::unlistAction(QAction *action)
{
    // ATTENTION:
    //   This method is called from KisKActionCollection with an QObject formerly
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

KisKActionCategoryPrivate::KisKActionCategoryPrivate(KisKActionCategory *host)
    : q(host)
{}

