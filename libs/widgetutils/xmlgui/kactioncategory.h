/* This file is part of the KDE libraries

   SPDX-FileCopyrightText: 2008 Michael Jansen <kde@michael-jansen.biz>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KACTIONCATEGORY_H
#define KACTIONCATEGORY_H

#include <kritawidgetutils_export.h>
#include "config-xmlgui.h"
#include <QObject>
#include <QString>
#include <QList>

#include <kstandardaction.h>
#include "kactioncollection.h"

struct KisKActionCategoryPrivate;

class QAction;

/**
 * Categorize actions for KShortcutsEditor.
 *
 * KisKActionCategory provides a second level to organize the actions in
 * KShortcutsEditor.
 *
 * The first possibility is using more than one action collection. Each
 * actions collection becomes a top level node.
 *
 * + action collection 1
 *   + first action
 *   + second action
 *   + third action
 * + action collection 2
 *   + first action
 *   + second action
 *   + third action
 *
 * Using KisKActionCategory it's possible to group the actions of one collection.
 * + action collection 1
 *   + first action
 *   + first category
 *     + action 1 in category
 *     + action 2 in category
 *   + second action
 *
 * \section Usage
 *
 * The usage is analog to action collections. Just create a category and use
 * it instead of the collection to create the actions.
 *
 * The synchronization between KisKActionCollection and KisKActionCategory is done
 * internally. There is for example no need to remove actions from a category.
 * It is done implicitly if the action is removed from the associated
 * collection.
 *
 * \code
 *
 * KisKActionCategory *file = new KisKActionCategory(i18n("File"), actionCollection());
 * file->addAction(
 *      KStandardAction::New,   //< see KStandardAction
 *      this,                   //< Receiver
 *      SLOT(fileNew()));       //< SLOT
 *
 * ... more actions added to file ...
 *
 * KisKActionCategory *edit = new KisKActionCategory(i18n("Edit"), actionCollection());
 * edit->addAction(
 *      KStandardAction::Copy,  //< see KStandardAction
 *      this,                   //< Receiver
 *      SLOT(fileNew()));       //< SLOT
 *
 * ...
 *
 * \endcode
 */
class KRITAWIDGETUTILS_EXPORT KisKActionCategory : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString text READ text WRITE setText)

public:

    /**
     * Default constructor
     */
    explicit KisKActionCategory(const QString &text, KisKActionCollection *parent = 0);

    /**
     * Destructor
     */
    ~KisKActionCategory() override;

    /**
     * \name Adding Actions
     *
     * Add a action to the category.
     *
     * This methods are provided for your convenience. They call the
     * corresponding method of KisKActionCollection.
     */
    //@{
    QAction *addAction(const QString &name, QAction *action);
    QAction *addAction(
        KStandardAction::StandardAction actionType,
        const QObject *receiver = 0,
        const char *member = 0);

    QAction *addAction(
        KStandardAction::StandardAction actionType,
        const QString &name,
        const QObject *receiver = 0,
        const char *member = 0);

    QAction *addAction(
        const QString &name,
        const QObject *receiver = 0,
        const char *member = 0);

    template<class ActionType>
    ActionType *add(
        const QString &name,
        const QObject *receiver = 0,
        const char *member = 0)
    {
        ActionType *action = collection()->add<ActionType>(name, receiver, member);
        addAction(action);
        return action;
    }
    //@}

    /**
     * Returns the actions belonging to this category
      */
    const QList<QAction *> actions() const;

    /**
     * The action collection this category is associated with.
     */
    KisKActionCollection *collection() const;

    /**
     * The action categories descriptive text
     */
    QString text() const;

    /**
     * Set the action categories descriptive text.
     */
    void setText(const QString &text);

private:

    /**
     * Remove \action from this category if found.
     */
    void unlistAction(QAction *action);

    /**
     * Add action to category
     */
    void addAction(QAction *action);

    //! KisKActionCollection needs access to some of our helper methods
    friend class KisKActionCollectionPrivate;

    //! Implementation details
    KisKActionCategoryPrivate *const d;
};

#endif /* #ifndef KACTIONCATEGORY_H */
