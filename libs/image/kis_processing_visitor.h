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

#ifndef __KIS_PROCESSING_VISITOR_H
#define __KIS_PROCESSING_VISITOR_H

#include "kritaimage_export.h"
#include "kis_shared.h"

#include <QMutex>

class KisNode;
class KoUpdater;
class KoProgressUpdater;
class KisUndoAdapter;
class KisPaintLayer;
class KisGroupLayer;
class KisAdjustmentLayer;
class KisExternalLayer;
class KisCloneLayer;
class KisFilterMask;
class KisTransformMask;
class KisTransparencyMask;
class KisSelectionMask;
class KisGeneratorLayer;
class KisColorizeMask;
class KUndo2Command;

/**
 * A visitor that processes a single layer; it does not recurse into the
 * layer's children. Classes inheriting KisProcessingVisitor must not
 * emit signals or ask the image to update the projection.
 */
class KRITAIMAGE_EXPORT KisProcessingVisitor : public KisShared
{
public:
    virtual ~KisProcessingVisitor();

    virtual void visit(KisNode *node, KisUndoAdapter *undoAdapter) = 0;
    virtual void visit(KisPaintLayer *layer, KisUndoAdapter *undoAdapter) = 0;
    virtual void visit(KisGroupLayer *layer, KisUndoAdapter *undoAdapter) = 0;
    virtual void visit(KisAdjustmentLayer *layer, KisUndoAdapter *undoAdapter) = 0;
    virtual void visit(KisExternalLayer *layer, KisUndoAdapter *undoAdapter) = 0;
    virtual void visit(KisGeneratorLayer *layer, KisUndoAdapter *undoAdapter) = 0;
    virtual void visit(KisCloneLayer *layer, KisUndoAdapter *undoAdapter) = 0;
    virtual void visit(KisFilterMask *mask, KisUndoAdapter *undoAdapter) = 0;
    virtual void visit(KisTransformMask *mask, KisUndoAdapter *undoAdapter) = 0;
    virtual void visit(KisTransparencyMask *mask, KisUndoAdapter *undoAdapter) = 0;
    virtual void visit(KisColorizeMask *mask, KisUndoAdapter *undoAdapter) = 0;
    virtual void visit(KisSelectionMask *mask, KisUndoAdapter *undoAdapter) = 0;

    /**
     * Create a command that initializes the processing visitor before running
     * on all the layers. The command is executed sequentially, non-exclusively
     * on the image by applicator.
     */
    virtual KUndo2Command* createInitCommand();

public:
    class KRITAIMAGE_EXPORT ProgressHelper {
    public:
        ProgressHelper(const KisNode *node);
        ~ProgressHelper();

        KoUpdater* updater() const;
    private:
        KoProgressUpdater *m_progressUpdater;
        mutable QMutex m_progressMutex;
    };
};

#endif /* __KIS_PROCESSING_VISITOR_H */
