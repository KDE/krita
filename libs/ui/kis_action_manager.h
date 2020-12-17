/*
 *  SPDX-FileCopyrightText: 2013 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#ifndef KIS_ACTION_MANAGER_H
#define KIS_ACTION_MANAGER_H

#include <kritaui_export.h>

#include <QPointer>

#include "KisView.h"

#include "kstandardaction.h"
#include "kis_action_registry.h"
#include "operations/kis_operation_configuration.h"

class KisViewManager;
class KisAction;
class KisOperationUIFactory;
class KisOperation;

/**
 * @brief A KisActionManager class keeps track of KisActions.
 * These actions are always associated with the GUI. That means each MainWindow
 * will create its own duplicate of these actions.
 *
 * KisActionManager enables and disables actions, to grey out buttons according
 * to the state of the application.
 *
 * Some of the primitive actions (load/save and so on) are not defined as
 * KisActions, but instead KActions, automacially registered through KXMLGUI.
 * It tracks these actions through the KActionCollection owned by the window.
 * Ultimately it would be nice to unify these things more fully.
 *
 */
class KRITAUI_EXPORT KisActionManager : public QObject
{
    Q_OBJECT
public:
    KisActionManager(KisViewManager* viewManager, KActionCollection *actionCollection);
    ~KisActionManager() override;

    void setView(QPointer<KisView> imageView);

    /**
     * Add an existing action to the action manager.
     */
    void addAction(const QString& name, KisAction* action);

    /**
     * Stop managing an action.
     */
    void takeAction(KisAction* action);

    /**
     * Create a new KisAction.  Looks up data from the .action data files.
     */
    KisAction *createAction(const QString &name);

    /**
     * Look up an action by name.
     */
    KisAction *actionByName(const QString &name) const;

    void registerOperationUIFactory(KisOperationUIFactory* factory);
    void registerOperation(KisOperation* operation);
    void runOperation(const QString &id);
    void runOperationFromConfiguration(KisOperationConfigurationSP config);

    /**
     * Update actions handled by kis_action_manager to set enabled.
     * This is used to grey out buttons that can't be pressed.
     */
    void updateGUI();

    /**
     * Create a KisAction based on a KStandardAction. The KStandardAction is deleted.
     */
    KisAction *createStandardAction(KStandardAction::StandardAction,
                                    const QObject *receiver, const char *member);

    static void safePopulateMenu(QMenu *menu, const QString &actionId, KisActionManager *actionManager);

private Q_SLOTS:
    void slotActionAddedToCollection(QAction *action);

private:
    void dumpActionFlags();

    class Private;
    Private* const d;
};

#endif // KIS_ACTION_MANAGER_H
