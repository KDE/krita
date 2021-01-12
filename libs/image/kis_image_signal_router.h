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

#ifndef __KIS_IMAGE_SIGNAL_ROUTER_H
#define __KIS_IMAGE_SIGNAL_ROUTER_H

#include <QObject>
#include "KisImageSignals.h"

class KoColorSpace;
class KoColorProfile;


class KRITAIMAGE_EXPORT KisImageSignalRouter : public QObject
{
    Q_OBJECT

public:
    KisImageSignalRouter(KisImageWSP image);
    ~KisImageSignalRouter() override;

    void emitNotification(KisImageSignalType type);
    void emitNotifications(KisImageSignalVector notifications);

    void emitNodeChanged(KisNodeSP node);
    void emitNodeHasBeenAdded(KisNode *parent, int index);
    void emitAboutToRemoveANode(KisNode *parent, int index);

    void emitRequestLodPlanesSyncBlocked(bool value);
    void emitNotifyBatchUpdateStarted();
    void emitNotifyBatchUpdateEnded();

private Q_SLOTS:
    void slotNotification(KisImageSignalType type);

Q_SIGNALS:

    void sigNotification(KisImageSignalType type);

    /**
     * Emitted whenever the image wants to update Lod0 plane of the canvas. Blocking
     * synching effectively means that the canvas will not try to read from these planes
     * until the **all** the data is loaded. Otherwise the user will see weird flickering
     * because of partially loaded lod0 tiles.
     *
     * NOTE: while the sync is blockes, the canvas is considered to use LodN planes
     *       that are expected to contain valid data.
     */
    void sigRequestLodPlanesSyncBlocked(bool value);

    /**
     * Emitted whenever the image is going to issue a lot of canvas update signals and
     * it it a good idea to group then and rerender the canvas in one go. The canvas
     * should initiate new rerenders while the batch is in progress.
     *
     * NOTE: even though the batched updates will not initiate a rerender, it does
     *       **not** guarantee that there will be processed during the batch. The
     *       updates may come from other sources, e.g. from mouse moves.
     *
     * NOTE: this feature is used to avoid flickering when switching
     *       back from lodN plane back to lod0. All the texture tiles should
     *       be loaded with new information before mipmaps can be regenerated.
     */
    void sigNotifyBatchUpdateStarted();

    /**
     * \see sigNotifyBatchUpdateStarted()
     */
    void sigNotifyBatchUpdateEnded();

    // Notifications
    void sigImageModified();
    void sigImageModifiedWithoutUndo();

    void sigSizeChanged(const QPointF &oldStillPoint, const QPointF &newStillPoint);
    void sigProfileChanged(const KoColorProfile *  profile);
    void sigColorSpaceChanged(const KoColorSpace*  cs);
    void sigResolutionChanged(double xRes, double yRes);
    void sigRequestNodeReselection(KisNodeSP activeNode, const KisNodeList &selectedNodes);

    // Graph change signals
    void sigNodeChanged(KisNodeSP node);
    void sigNodeAddedAsync(KisNodeSP node);
    void sigRemoveNodeAsync(KisNodeSP node);
    void sigLayersChangedAsync();

private:
    KisImageWSP m_image;
};

#endif /* __KIS_IMAGE_SIGNAL_ROUTER_H */
