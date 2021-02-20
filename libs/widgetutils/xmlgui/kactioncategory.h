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

struct KActionCategoryPrivate;

class QAction;

/**
 * Categorize actions for KShortcutsEditor.
 *
 * KActionCategory provides a second level to organize the actions in
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
 * Using KActionCategory it's possible to group the actions of one collection.
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
 * The synchronization between KActionCollection and KActionCategory is done
 * internally. There is for example no need to remove actions from a category.
 * It is done implicitly if the action is removed from the associated
 * collection.
 *
 * \code
 *
 * KActionCategory *file = new KActionCategory(i18n("File"), actionCollection());
 * file->addAction(
 *      KStandardAction::New,   //< see KStandardAction
 *      this,                   //< Receiver
 *      SLOT(fileNew()));       //< SLOT
 *
 * ... more actions added to file ...
 *
 * KActionCategory *edit = new KActionCategory(i18n("Edit"), actionCollection());
 * edit->addAction(
 *      KStandardAction::Copy,  //< see KStandardAction
 *      this,                   //< Receiver
 *      SLOT(fileNew()));       //< SLOT
 *
 * ...
 *
 * \endcode
 */
class KRITAWIDGETUTILS_EXPORT KActionCategory : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString text READ text WRITE setText)

public:

    /**
     * Default constructor
     */
    explicit KActionCategory(const QString &text, KActionCollection *parent = 0);

    /**
     * Destructor
     */
    ~KActionCategory() override;

    /**
     * \name Adding Actions
     *
     * Add a action to the category.
     *
     * This methods are provided for your convenience. They call the
     * corresponding method of KActionCollection.
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
    KActionCollection *collection() const;

    /**
     * The action categorys descriptive text
     */
    QString text() const;

    /**
     * Set the action categorys descriptive text.
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

    //! KActionCollection needs access to some of our helper methods
    friend class KActionCollectionPrivate;

    //! Implementation details
    KActionCategoryPrivate *const d;
};

#endif /* #ifndef KACTIONCATEGORY_H */
