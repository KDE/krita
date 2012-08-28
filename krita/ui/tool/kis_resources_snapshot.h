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

#ifndef __KIS_RESOURCES_SNAPSHOT_H
#define __KIS_RESOURCES_SNAPSHOT_H

#include "kis_shared.h"
#include "kis_shared_ptr.h"
#include "kis_types.h"
#include "krita_export.h"
#include "kis_painter.h"


class KoCanvasResourceManager;
class KoCompositeOp;
class KisPainter;
class KisPostExecutionUndoAdapter;
class KisRecordedPaintAction;


class KRITAUI_EXPORT KisResourcesSnapshot : public KisShared
{
public:
    KisResourcesSnapshot(KisImageWSP image, KisPostExecutionUndoAdapter *undoAdapter, KoCanvasResourceManager *resourceManager);
    ~KisResourcesSnapshot();

    void setupPainter(KisPainter *painter);
    void KDE_DEPRECATED setupPaintAction(KisRecordedPaintAction *action);


    KisPostExecutionUndoAdapter* postExecutionUndoAdapter() const;
    void setCurrentNode(KisNodeSP node);
    void setStrokeStyle(KisPainter::StrokeStyle strokeStyle);
    void setFillStyle(KisPainter::FillStyle fillStyle);

    KisNodeSP currentNode() const;
    KisImageWSP image() const;
    bool needsIndirectPainting() const;

    bool needsAirbrushing() const;
    int airbrushingRate() const;

    quint8 opacity() const;
    const KoCompositeOp* compositeOp() const;

private:
    struct Private;
    Private * const m_d;
};

typedef KisSharedPtr<KisResourcesSnapshot> KisResourcesSnapshotSP;


#endif /* __KIS_RESOURCES_SNAPSHOT_H */
