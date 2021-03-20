/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_ITERATOR_COMPLETE_LISTENER_H
#define KIS_ITERATOR_COMPLETE_LISTENER_H


/**
 * @brief The KisIteratorCompleteListener struct is a special interface for
 * notifying the paint device that an iterator has completed its execution.
 */
struct KisIteratorCompleteListener {
    virtual ~KisIteratorCompleteListener() {}
    virtual void notifyWritableIteratorCompleted() = 0;
};

#endif // KIS_ITERATOR_COMPLETE_LISTENER_H

