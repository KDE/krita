/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
              (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>
              (C) 2005-2006 Hamish Rodda <rodda@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KACTIONCOLLECTION_H
#define KACTIONCOLLECTION_H
#include "config-xmlgui.h"
#include <kritawidgetutils_export.h>
#include <kstandardaction.h>
#include <QObject>

class QAction;
class KXMLGUIClient;
class KConfigGroup;
class QActionGroup;
class QString;
class KActionCategory;

/**
 * \short A container for a set of QAction objects.
 *
 * KActionCollection manages a set of QAction objects.  It
 * allows them to be grouped for organized presentation of configuration to the user,
 * saving + loading of configuration, and optionally for automatic plugging into
 * specified widget(s).
 *
 * Additionally, KActionCollection provides several convenience functions for locating
 * named actions, and actions grouped by QActionGroup.
 *
 * \note If you create your own action collection and need to assign shortcuts
 * to the actions within, you have to call associateWidget() or
 * addAssociatedWidget() to have them working.
 */
class KRITAWIDGETUTILS_EXPORT KActionCollection : public QObject
{
    friend class KXMLGUIClient;

    Q_OBJECT

    Q_PROPERTY(QString configGroup READ configGroup WRITE setConfigGroup)

public:
    /**
     * Constructor.  Allows specification of a component name other than the default
     * application name, where needed (remember to call setComponentDisplayName() too).
     */
    explicit KActionCollection(QObject *parent, const QString &cName = QString());

    /**
     * Destructor.
     */
    ~KActionCollection() override;

    /**
     * Access the list of all action collections in existence for this app
     */
    static const QList<KActionCollection *> &allCollections();

    /**
     * Clears the entire action collection, deleting all actions.
     */
    void clear();

    /**
     * Associate all actions in this collection to the given \a widget.
     * Unlike addAssociatedWidget, this method only adds all current actions
     * in the collection to the given widget. Any action added after this call
     * will not be added to the given widget automatically.
     * So this is just a shortcut for a foreach loop and a widget->addAction call.
     */
    void associateWidget(QWidget *widget) const;

    /**
     * Associate all actions in this collection to the given \a widget, including any actions
     * added after this association is made.
     *
     * This does not change the action's shortcut context, so if you need to have the actions only
     * trigger when the widget has focus, you'll need to set the shortcut context on each action
     * to Qt::WidgetShortcut (or better still, Qt::WidgetWithChildrenShortcut with Qt 4.4+)
     */
    void addAssociatedWidget(QWidget *widget);

    /**
     * Remove an association between all actions in this collection and the given \a widget, i.e.
     * remove those actions from the widget, and stop associating newly added actions as well.
     */
    void removeAssociatedWidget(QWidget *widget);

    /**
     * Return a list of all associated widgets.
     */
    QList<QWidget *> associatedWidgets() const;

    /**
     * Clear all associated widgets and remove the actions from those widgets.
     */
    void clearAssociatedWidgets();

    /**
     * Returns the KConfig group with which settings will be loaded and saved.
     */
    QString configGroup() const;

    /**
     * Sets \a group as the KConfig group with which settings will be loaded and saved.
     */
    void setConfigGroup(const QString &group);

    /**
      * Read all key associations from @p config.
      *
      * If @p config is zero, read all key associations from the
      * application's configuration file KSharedConfig::openConfig(),
      * in the group set by setConfigGroup().
      */
    void readSettings();

    /**
     * Update shortcuts from the KisActionRegistry.
     */
    void updateShortcuts();

    /**
      * Write the current configurable key associations. If @a is nonzero, use
      * that configuration group.
      *
      * Otherwise, the output file is determined as follows. If this action
      * collection belongs to a KXMLGuiClient the setting are saved to the
      * kxmlgui definition file. If not the settings are written to the
      * applications config file.
      *
      * \note oneAction() and writeDefaults() have no meaning for the kxmlgui
      * configuration file.
      *
      * \param config Config object to save to, or null (see above)
      * \param writeScheme set to true if we export all settings as new shortcut scheme
      * \param oneAction pass an action here if you just want to save the values for one action, eg.
      *                  if you know that action is the only one which has changed.
      */
    void writeSettings(KConfigGroup *config = 0, bool writeScheme = false, QAction *oneAction = 0) const;

    /**
     * Returns the number of actions in the collection.
     *
     * This is equivalent to actions().count().
     */
    int count() const;

    /**
     * Returns whether the action collection is empty or not.
     */
    bool isEmpty() const;

    /**
     * Return the QAction* at position "index" in the action collection.
     *
     * This is equivalent to actions().value(index);
     */
    QAction *action(int index) const;

    /**
     * Get the action with the given \a name from the action collection.
     *
     * @param name Name of the QAction
     * @return A pointer to the QAction in the collection which matches the parameters or
     * null if nothing matches.
     */
    QAction *action(const QString &name) const;

    /**
     * Returns the list of QActions which belong to this action collection.
     *
     * The list is guaranteed to be in the same order the action were put into
     * the collection.
     */
    QList<QAction *> actions() const;

    /**
     * Returns the list of QActions without an QAction::actionGroup() which belong to this action collection.
     */
    const QList<QAction *> actionsWithoutGroup() const;

    /**
     * Returns the list of all QActionGroups associated with actions in this action collection.
     */
    const QList<QActionGroup *> actionGroups() const;

    /**
     * Set the \a componentName associated with this action collection.
     *
     * \warning Don't call this method on a KActionCollection that contains
     * actions. This is not supported.
     *
     * \param componentName the name which is to be associated with this action collection,
     * or QString() to indicate the app name. This is used to load/save settings into XML files.
     * KXmlGuiClient::setComponentName takes care of calling this.
     */
    void setComponentName(const QString &componentName);

    /** The component name with which this class is associated. */
    QString componentName() const;

    /**
     * Set the component display name associated with this action collection.
     * (e.g. for the toolbar editor)
     * KXmlGuiClient::setComponentName takes care of calling this.
     */
    void setComponentDisplayName(const QString &displayName);

    /** The display name for the associated component. */
    QString componentDisplayName() const;

    /**
     * The parent KXMLGUIClient, or null if not available.
     */
    const KXMLGUIClient *parentGUIClient() const;


    /**
     * Returns the KActionCategories inside this collection
     */
    QList<KActionCategory *> categories() const;


    /**
     * Gets a category with name @p name inside this collection.
     *
     * Creates a new category if one does not exist.
     */
    KActionCategory *getCategory(const QString &categoryName);

Q_SIGNALS:
    /**
     * Indicates that \a action was inserted into this action collection.
     */
    void inserted(QAction *action);

    /**
     * Indicates that \a action was removed from this action collection.
     * @deprecated
     */
    QT_MOC_COMPAT void removed(QAction *action);

    /**
     * Indicates that \a action was highlighted (hovered over).
     * @deprecated Replaced by actionHovered(QAction* action);
     */
    QT_MOC_COMPAT void actionHighlighted(QAction *action);

    /**
     * Indicates that \a action was hovered.
     */
    void actionHovered(QAction *action);

    /**
     * Indicates that \a action was triggered
     */
    void actionTriggered(QAction *action);

protected:
    /// Overridden to perform connections when someone wants to know whether an action was highlighted or triggered
    void connectNotify(const QMetaMethod &signal) override;

protected Q_SLOTS:
    virtual void slotActionTriggered();

    /**
     * @internal
     * @deprecated Replaced by slotActionHovered();
     */
    QT_MOC_COMPAT virtual void slotActionHighlighted();

private Q_SLOTS:
    void slotActionHovered();

public:
    /**
     * Add an action under the given name to the collection.
     *
     * Inserting an action that was previously inserted under a different name will replace the
     * old entry, i.e. the action will not be available under the old name anymore but only under
     * the new one.
     *
     * Inserting an action under a name that is already used for another action will replace
     * the other action in the collection (but will not delete it).
     *
     * If KAuthorized::authorizeKAction() reports that the action is not
     * authorized, it will be disabled and hidden.
     *
     * @param name The name by which the action be retrieved again from the collection.
     * @param action The action to add.
     * @return the same as the action given as parameter. This is just for convenience
     * (chaining calls) and consistency with the other addAction methods, you can also
     * simply ignore the return value.
     */
    Q_INVOKABLE QAction *addAction(const QString &name, QAction *action);

    /**
     * Adds a new action to the collection in category @p category.
     *
     * The category will be created if it does not already exist.
     */
    Q_INVOKABLE QAction *addCategorizedAction(const QString &name, QAction *action, const QString &categoryName);

    /**
     * Adds a list of actions to the collection.
     *
     * The objectName of the actions is used as their internal name in the collection.
     *
     * Uses addAction(QString, QAction*).
     *
     * @param actions the list of the actions to add.
     *
     * @see addAction()
     * @since 5.0
     */
    void addActions(const QList<QAction *> &actions);

    /**
     * Removes an action from the collection and deletes it.
     * @param action The action to remove.
     */
    void removeAction(QAction *action);

    /**
     * Removes an action from the collection.
     * @param action the action to remove.
     */
    QAction *takeAction(QAction *action);

    /**
     * Creates a new standard action, adds it to the collection and connects the
     * action's triggered(bool) signal to the specified receiver/member. The
     * newly created action is also returned.
     *
     * Note: Using KStandardAction::OpenRecent will cause a different signal than
     * triggered(bool) to be used, see KStandardAction for more information.
     *
     * The action can be retrieved later from the collection by its standard name as per
     * KStandardAction::stdName.
     *
     * @param actionType The standard action type of the action to create.
     * @param receiver The QObject to connect the triggered(bool) signal to.  Leave 0 if no
     *                 connection is desired.
     * @param member The SLOT to connect the triggered(bool) signal to.  Leave 0 if no
     *               connection is desired.
     * @return new action of the given type ActionType.
     */
    QAction *addAction(KStandardAction::StandardAction actionType, const QObject *receiver = 0, const char *member = 0);

    /**
     * Creates a new standard action, adds to the collection under the given name
     * and connects the action's triggered(bool) signal to the specified
     * receiver/member. The newly created action is also returned.
     *
     * Note: Using KStandardAction::OpenRecent will cause a different signal than
     * triggered(bool) to be used, see KStandardAction for more information.
     *
     * The action can be retrieved later from the collection by the specified name.
     *
     * @param actionType The standard action type of the action to create.
     * @param name The name by which the action be retrieved again from the collection.
     * @param receiver The QObject to connect the triggered(bool) signal to.  Leave 0 if no
     *                 connection is desired.
     * @param member The SLOT to connect the triggered(bool) signal to.  Leave 0 if no
     *               connection is desired.
     * @return new action of the given type ActionType.
     */
    QAction *addAction(KStandardAction::StandardAction actionType, const QString &name,
                       const QObject *receiver = 0, const char *member = 0);

    /**
     * Creates a new action under the given name to the collection and connects
     * the action's triggered(bool) signal to the specified receiver/member. The
     * newly created action is returned.
     *
     * NOTE: KDE prior to 4.2 used the triggered() signal instead of the triggered(bool)
     * signal.
     *
     * Inserting an action that was previously inserted under a different name will replace the
     * old entry, i.e. the action will not be available under the old name anymore but only under
     * the new one.
     *
     * Inserting an action under a name that is already used for another action will replace
     * the other action in the collection.
     *
     * @param name The name by which the action be retrieved again from the collection.
     * @param receiver The QObject to connect the triggered(bool) signal to.  Leave 0 if no
     *                 connection is desired.
     * @param member The SLOT to connect the triggered(bool) signal to.  Leave 0 if no
     *               connection is desired.
     * @return new action of the given type ActionType.
     */
    QAction *addAction(const QString &name, const QObject *receiver = 0, const char *member = 0);

    /**
     * Creates a new action under the given name, adds it to the collection and connects the action's triggered(bool)
     * signal to the specified receiver/member. The receiver slot may accept either a bool or no
     * parameters at all (i.e. slotTriggered(bool) or slotTriggered() ).
     * The type of the action is specified by the template parameter ActionType.
     *
     * NOTE: KDE prior to 4.2 connected the triggered() signal instead of the triggered(bool)
     * signal.
     *
     * @param name The internal name of the action (e.g. "file-open").
     * @param receiver The QObject to connect the triggered(bool) signal to.  Leave 0 if no
     *                 connection is desired.
     * @param member The SLOT to connect the triggered(bool) signal to.  Leave 0 if no
     *               connection is desired.
     * @return new action of the given type ActionType.
     *
     * @see addAction()
     */
    template<class ActionType>
    ActionType *add(const QString &name, const QObject *receiver = 0, const char *member = 0)
    {
        ActionType *a = new ActionType(this);
        if (receiver && member) {
            connect(a, SIGNAL(triggered(bool)), receiver, member);
        }
        addAction(name, a);
        return a;
    }

    /**
     * Get the default primary shortcut for the given action.
     *
     * @param action the action for which the default primary shortcut should be returned.
     * @return the default primary shortcut of the given action
     * @since 5.0
     */
    QKeySequence defaultShortcut(QAction *action) const;

    /**
     * Get the default shortcuts for the given action.
     *
     * @param action the action for which the default shortcuts should be returned.
     * @return the default shortcuts of the given action
     * @since 5.0
     */
    QList<QKeySequence> defaultShortcuts(QAction *action) const;

    //TODO KF6: Make setDefaultShortcut static
    /**
     * Set the default shortcut for the given action.
     * Since 5.2, this also calls action->setShortcut(shortcut), i.e. the default shortcut is
     * made active initially.
     *
     * @param action the action for which the default shortcut should be set.
     * @param shortcut the shortcut to use for the given action in its specified shortcutContext()
     * @since 5.0
     */
    void setDefaultShortcut(QAction *action, const QKeySequence &shortcut);

    /**
     * Set the default shortcuts for the given action.
     * Since 5.2, this also calls action->setShortcuts(shortcuts), i.e. the default shortcut is
     * made active initially.
     *
     * @param action the action for which the default shortcut should be set.
     * @param shortcuts the shortcuts to use for the given action in its specified shortcutContext()
     * @since 5.0
     */
    Q_INVOKABLE void setDefaultShortcuts(QAction *action, const QList<QKeySequence> &shortcuts);

    /**
     * Returns true if the given action's shortcuts may be configured by the user.
     *
     * @param action the action for the hint should be verified.
     * @since 5.0
     */
    bool isShortcutsConfigurable(QAction *action) const;

    /**
     * Indicate whether the user may configure the action's shortcuts.
     *
     * @param action the action for the hint should be verified.
     * @param configurable set to true if the shortcuts of the given action may be configured by the user, otherwise false.
     * @since 5.0
     */
    void setShortcutsConfigurable(QAction *action, bool configurable);

private:
    Q_PRIVATE_SLOT(d, void _k_actionDestroyed(QObject *))
    Q_PRIVATE_SLOT(d, void _k_associatedWidgetDestroyed(QObject *))

    KActionCollection(const KXMLGUIClient *parent);   // used by KXMLGUIClient

    friend class KActionCollectionPrivate;
    class KActionCollectionPrivate *const d;
};

#endif
