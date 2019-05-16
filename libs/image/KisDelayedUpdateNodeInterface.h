/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
