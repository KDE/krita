/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_IMAGE_INTERFACES_H
#define __KIS_IMAGE_INTERFACES_H

#include "kis_types.h"
#include <kritaimage_export.h>

class QRect;
class KisStrokeStrategy;
class KisStrokeJobData;
class KisPostExecutionUndoAdapter;


class KRITAIMAGE_EXPORT KisStrokesFacade
{
public:
    virtual ~KisStrokesFacade();
    virtual KisStrokeId startStroke(KisStrokeStrategy *strokeStrategy) = 0;
    virtual void addJob(KisStrokeId id, KisStrokeJobData *data) = 0;
    virtual void endStroke(KisStrokeId id) = 0;
    virtual bool cancelStroke(KisStrokeId id) = 0;
};

class KRITAIMAGE_EXPORT KisUpdatesFacade
{
public:
    virtual ~KisUpdatesFacade();

    virtual void blockUpdates() = 0;
    virtual void unblockUpdates() = 0;

    virtual void disableUIUpdates() = 0;
    virtual QVector<QRect> enableUIUpdates() = 0;

    virtual bool hasUpdatesRunning() const = 0;

    virtual void notifyBatchUpdateStarted() = 0;
    virtual void notifyBatchUpdateEnded() = 0;
    virtual void notifyUIUpdateCompleted(const QRect &rc) = 0;

    virtual QRect bounds() const = 0;

    virtual void disableDirtyRequests() = 0;
    virtual void enableDirtyRequests() = 0;

    virtual void refreshGraphAsync(KisNodeSP root) = 0;
    virtual void refreshGraphAsync(KisNodeSP root, const QRect &rc) = 0;
    virtual void refreshGraphAsync(KisNodeSP root, const QRect &rc, const QRect &cropRect) = 0;

    virtual void setProjectionUpdatesFilter(KisProjectionUpdatesFilterSP filter) = 0;
    virtual KisProjectionUpdatesFilterSP projectionUpdatesFilter() const = 0;
};

class KRITAIMAGE_EXPORT KisProjectionUpdateListener
{
public:
    virtual ~KisProjectionUpdateListener();
    virtual void notifyProjectionUpdated(const QRect &rc) = 0;
};

class KRITAIMAGE_EXPORT KisStrokeUndoFacade
{
public:
    virtual ~KisStrokeUndoFacade();
    virtual KisPostExecutionUndoAdapter* postExecutionUndoAdapter() const = 0;
    virtual const KUndo2Command* lastExecutedCommand() const = 0;
};

#endif /* __KIS_IMAGE_INTERFACES_H */
