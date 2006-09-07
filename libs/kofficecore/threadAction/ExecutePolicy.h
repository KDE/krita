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

#ifndef TW_EXECUTEPOLICY_H
#define TW_EXECUTEPOLICY_H

class QVariant;

namespace ThreadWeaver {
    class Action;
    class JobsListPolicy;

    class OnlyLastPolicy;
    class DirectPolicy;
    class QueuedPolicy;
    class SimpleQueuedPolicy;

    /**
     * Each action that is called 'execute()' on will be executed according to policy.
     * Each action will have a policy set; that policy will be used on execution of that
     * action to determine what will happen next.
     */
    class ExecutePolicy {
    public:
        ExecutePolicy() {}
        virtual ~ExecutePolicy() {}
        /**
         * schedule an action according to policy.
         * @param action the action to be scheduled.
         * @param jobsList the list of jobs currently associated with the action.
         * @param params a parameters object that belongs with the current action.
         */
        virtual void schedule(Action *action, JobsListPolicy *jobsList, QVariant *params) = 0;

        static ExecutePolicy *const onlyLastPolicy;
        static ExecutePolicy *const directPolicy;
        static ExecutePolicy *const queuedPolicy;
        static ExecutePolicy *const simpleQueuedPolicy;
        // TODO alter to staticDeleter when we depend on kdelibs.
    };


    /**
     * This policy will enqueue the action; but will remove others that would call the
     * same target method. In the case of a delayed-initialisation action being executed
     * from one or even several actions it is unwanted to let the actions queue-up and be
     * executed serially; this policy will prevent that.
     * (Example)
     * Consider having a list of items that will be previed when you click it. The
     * creation of the preview is a slow process. In this example it is possible for the
     * user to select 5 items in a row which will then be previewed one after another;
     * making the update of the preview incredably slow.  If you use this policy only the
     * last action will be executed and the other actions will be discarded. Effect is
     * that only the last preview will be generated and the first 4 ignored.
     */
    class OnlyLastPolicy : public ExecutePolicy {
        void schedule(Action *action, JobsListPolicy *jobsList, QVariant *params);
    };

    /**
     * This policy will execute the action in the calling thread.
     */
    class DirectPolicy : public ExecutePolicy {
        void schedule(Action *action, JobsListPolicy *jobsList, QVariant *params);
    };

    /**
     * This policy will queue each action to be executed serially, while disabling the
     * action when running. When an action comes in it will be disabled and queued; after
     * executing it will be enabled again. This way only one action can be executed at the
     * same time and additional executes will be ignored until the first is done.
     */
    class QueuedPolicy : public ExecutePolicy {
        void schedule(Action *action, JobsListPolicy *jobsList, QVariant *params);
    };

    /**
     * This policy will queue each action to be executed serially.
     */
    class SimpleQueuedPolicy : public ExecutePolicy {
        void schedule(Action *action, JobsListPolicy *jobsList, QVariant *params);
    };
}

#endif
