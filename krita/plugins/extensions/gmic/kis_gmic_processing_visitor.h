/*
 *  Copyright (c) 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef __KIS_GMIC_PROCESSING_VISITOR_H
#define __KIS_GMIC_PROCESSING_VISITOR_H

#include "kis_processing_visitor.h"
#include <kis_types.h>
#include <QRect>

#include <gmic.h>

class KisView2;

class QString;
class QImage;

class KisGmicProcessingVisitor : public KisProcessingVisitor
{
public:
    KisGmicProcessingVisitor(QString gmicCommandStr, KisView2 * view);

    void visit(KisNode *node, KisUndoAdapter *undoAdapter);
    void visit(KisPaintLayer *layer, KisUndoAdapter *undoAdapter);
    void visit(KisGroupLayer *layer, KisUndoAdapter *undoAdapter);
    void visit(KisAdjustmentLayer *layer, KisUndoAdapter *undoAdapter);
    void visit(KisExternalLayer *layer, KisUndoAdapter *undoAdapter);
    void visit(KisGeneratorLayer *layer, KisUndoAdapter *undoAdapter);
    void visit(KisCloneLayer *layer, KisUndoAdapter *undoAdapter);
    void visit(KisFilterMask *mask, KisUndoAdapter *undoAdapter);
    void visit(KisTransparencyMask *mask, KisUndoAdapter *undoAdapter);
    void visit(KisSelectionMask *mask, KisUndoAdapter *undoAdapter);

private:
    void process(KisNode *node, KisUndoAdapter *undoAdapter);

    // apply gmic command to one layer
    void applyGmicToDevice(KisPaintDeviceSP src, QString gmicCommandStr);


    QImage convertToQImage(gmic_image<float>& gmicImage);
    void convertFromQImage(const QImage &image, gmic_image<float>& gmicImage);
    void convertToGmicImage(KisPaintDeviceSP dev, gmic_image<float>& gmicImage);
    void convertToGmicImageOpti(KisPaintDeviceSP dev, gmic_image<float>& gmicImage);
    KisPaintDeviceSP convertFromGmicImage(gmic_image<float>& gmicImage);



private:
    QString m_gmicCommandStr;
    KisView2 * m_view;

};

#endif /* __KIS_GMIC_PROCESSING_VISITOR_H */
