/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2020-2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_IMPORT_GMIC_PROCESSING_VISITOR_H
#define __KIS_IMPORT_GMIC_PROCESSING_VISITOR_H

#include <processing/kis_simple_processing_visitor.h>

#include <QVector>
#include <QSharedPointer>

#include <kis_node.h>

#include "gmic.h"


class KisUndoAdapter;

class KisImportQmicProcessingVisitor : public KisSimpleProcessingVisitor
{
public:
    KisImportQmicProcessingVisitor(const KisNodeListSP nodes,
                                   QVector<gmic_image<float> *> images,
                                   const QRect &dstRect,
                                   const KisSelectionSP selection
                                  );

    static void applyLayerNameChanges(const gmic_image<float> &srcGmicImage,
                                   KisNode *node,
                                   KisPaintDeviceSP dst
                                   );

    static void gmicImageToPaintDevice(gmic_image<float>& srcGmicImage,
                                       KisPaintDeviceSP dstPaintDevice,
                                       KisSelectionSP selection = 0,
                                       const QRect &dstRect = QRect());


protected:

    void visitNodeWithPaintDevice(KisNode *node, KisUndoAdapter *undoAdapter);
    void visitExternalLayer(KisExternalLayer *layer, KisUndoAdapter *undoAdapter);
    void visitColorizeMask(KisColorizeMask *mask, KisUndoAdapter *undoAdapter);

private:
    const KisNodeListSP m_nodes;
    QVector<gmic_image<float> *> m_images;
    QRect m_dstRect;
    const KisSelectionSP m_selection;
};

#endif /* __KIS_IMPORT_GMIC_PROCESSING_VISITOR_H */
