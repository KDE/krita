/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2020-2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_QMIC_PROCESSING_VISITOR_H
#define KIS_QMIC_PROCESSING_VISITOR_H

#include <processing/kis_simple_processing_visitor.h>

#include <QVector>

#include <kis_node.h>

#include "kis_qmic_interface.h"

class KisQmicProcessingVisitor : public KisSimpleProcessingVisitor
{
public:
    KisQmicProcessingVisitor(const KisNodeListSP nodes,
                             QVector<KisQMicImageSP> images,
                             const QRect &dstRect,
                             const KisSelectionSP selection);

    ~KisQmicProcessingVisitor() override;

protected:
    void visitNodeWithPaintDevice(KisNode *node, KisUndoAdapter *undoAdapter) override;
    void visitExternalLayer(KisExternalLayer *layer, KisUndoAdapter *undoAdapter) override;
    void visitColorizeMask(KisColorizeMask *mask, KisUndoAdapter *undoAdapter) override;

private:
    struct Private;
    Private const *d;
};

#endif /* KIS_QMIC_PROCESSING_VISITOR_H */
