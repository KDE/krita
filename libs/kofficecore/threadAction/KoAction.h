/*
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef TW_ACTION_H
#define TW_ACTION_H

#include <Job.h>
#include "KoJobsListPolicy.h"

#include <koffice_export.h>

class KoExecutePolicy;
class DirectPolicy;

/**
 * This class represent the glue between a user event and a piece of controlling logic.
 * Events based programming creates the need to execute certain pieces of logic based
 * on the incoming event, conveniently grouped per action. An action can be something
 * like 'print'.
 * This action uses a ThreadWeaver object to handle the actions and therefor this
 * action is implicitly multithreading.
 * Example usage:
 * @code
    Action *myAction = new Action();
    myAction->setWeaver(m_weaver);
    myAction->connect(myAction, SIGNAL(triggered(const QVariant&)),
        target, SLOT(slot()), Qt::DirectConnection);

    myAction.execute();
 * @endcode
 * In this example the method 'slot' will be called each time
 * the 'execute()' method is called on the action. It will be called in a differen
 * thread from the main thread and you get the guarentee that it will
 * never be called before a previous call has ended.
 * The execute() method can be called from any thread to simply move control to
 * an anonymous thread.
 *
 * Notice that the default version uses the SimpleQueuedPolicy.
 */
class KOFFICECORE_EXPORT KoAction : public QObject {
    Q_OBJECT
public:
    /**
     * Create a new Action object.
     * @param parent the parent QObject, for memory mangement purposes.
     */
    KoAction(QObject *parent = 0);
    virtual ~KoAction() {}

    /**
     * Set a ThreadWeaver on this action which is used to execute the action in a
     * different thread when it is activated;
     * @param weaver the weaver to be used
     */
    void setWeaver(ThreadWeaver::WeaverInterface *weaver) { m_weaver = weaver; }

    /**
     * Return the currently set threadWeaver
     */
    ThreadWeaver::WeaverInterface *weaver() const { return m_weaver; }

    /**
     * Set a new policy for this action.
     */
    void setExecutePolicy(KoExecutePolicy *policy) { m_policy = policy; }

    /**
     * Enable disable this action and all its registered components. Incoming
     * events will not cause the action to be committed when the action is disabled.
     * @param enabled the new state of the action.
     */
    void setEnabled(bool enabled) { m_enabled = enabled; }

    /**
     * return if the action is enabled.
     */
    bool isEnabled() const { return m_enabled; }

    /**
     * Return te amount of executes there are still to finish.
     */
    int jobCount() { return m_jobsQueue.count(); }

signals:
    /**
     * The signal emitted when the action is triggered.
     * Note that this signal is queued and can very well be in a different thread
     * based on the current executePolicy.
     * @param params a variant that equals the variant given in the execute() signal
     */
    void triggered(const QVariant &params);

    /**
     * The signal emitted directly after the triggered() signal, but this signal
     * is guaranteed to be in the Gui thread and can be used to update the user
     * interface after an action has been completed.
     * @param params a variant that equals the variant given in the execute() signal
     */
    void updateUi(const QVariant &params);

public slots:
    /**
     * Call this to request the action to be executed. The request will be handled
     * according to the current ExecutePolicy
     */
    void execute();

    /**
     * Call this to request the action to be executed. The request will be handled
     * according to the current ExecutePolicy
     * @param params a variant with a parameter that will be emitted in the
     * triggered() and updateUi() signals.
     */
    void execute(QVariant *params);

private:
    friend class ActionJob;
    void doAction(QVariant *params); // called from ActionJob
    void doActionUi(QVariant *params); // called from ActionJob

    KoExecutePolicy *m_policy;
    ThreadWeaver::WeaverInterface *m_weaver;
    bool m_enabled;
    KoJobsListPolicy m_jobsQueue;
};

#endif
