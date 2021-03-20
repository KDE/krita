/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    virtual void refreshGraphAsync(KisNodeSP root, const QVector<QRect> &rc, const QRect &cropRect) = 0;

    virtual KisProjectionUpdatesFilterCookie addProjectionUpdatesFilter(KisProjectionUpdatesFilterSP filter) = 0;
    virtual KisProjectionUpdatesFilterSP removeProjectionUpdatesFilter(KisProjectionUpdatesFilterCookie cookie) = 0;
    virtual KisProjectionUpdatesFilterCookie currentProjectionUpdatesFilter() const = 0;

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
