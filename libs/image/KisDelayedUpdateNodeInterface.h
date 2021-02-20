/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISDELAYEDUPDATENODEINTERFACE_H
#define KISDELAYEDUPDATENODEINTERFACE_H

#include "kritaimage_export.h"


/**
 * @brief The KisDelayedUpdateNodeInterface class is an interface for
 * nodes that dealy their real updates with KisSignalCompressor. Some
 * operations need explicit regeneration before they can proceed.
 */
class KRITAIMAGE_EXPORT KisDelayedUpdateNodeInterface
{
public:
    virtual ~KisDelayedUpdateNodeInterface();

    /**
     * @brief forceUpdateTimedNode forrces the node to regenerate its project. The update might
     * be asynchronous, so you should call image->waitForDone() after that.
     */
    virtual void forceUpdateTimedNode() = 0;

    /**
     * @return true if forceUpdateTimedNode() is going to
     * produce any real updates, that is the node has any
     * updates still pending
     */
    virtual bool hasPendingTimedUpdates() const = 0;
};

#endif // KISDELAYEDUPDATENODEINTERFACE_H
