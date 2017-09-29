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

#include "KisPasteActionFactory.h"

#include "kis_image.h"
#include "KisViewManager.h"
#include "kis_tool_proxy.h"
#include "kis_canvas2.h"
#include "kis_canvas_controller.h"
#include "kis_paint_device.h"
#include "kis_paint_layer.h"
#include "kis_shape_layer.h"
#include "kis_import_catcher.h"
#include "kis_clipboard.h"
#include "commands/kis_image_layer_add_command.h"
#include "kis_processing_applicator.h"

#include <KoShapeController.h>
#include <KoShapeManager.h>
#include <KoSelection.h>
#include "kis_algebra_2d.h"
#include <KoShapeMoveCommand.h>

namespace {
QPointF getFittingOffset(QList<KoShape*> shapes,
                         const QPointF &shapesOffset,
                         const QRectF &documentRect,
                         const qreal fitRatio)
{
    QPointF accumulatedFitOffset;

    Q_FOREACH (KoShape *shape, shapes) {
        const QRectF bounds = shape->boundingRect();

        const QPointF center = bounds.center() + shapesOffset;

        const qreal wMargin = (0.5 - fitRatio) * bounds.width();
        const qreal hMargin = (0.5 - fitRatio) * bounds.height();
        const QRectF allowedRect = documentRect.adjusted(-wMargin, -hMargin, wMargin, hMargin);

        const QPointF fittedCenter = KisAlgebra2D::clampPoint(center, allowedRect);

        accumulatedFitOffset += fittedCenter - center;
    }

    return accumulatedFitOffset;
}
}

void KisPasteActionFactory::run(bool pasteAtCursorPosition, KisViewManager *view)
{
    KisImageSP image = view->image();
    if (!image) return;

    const QRect fittingBounds = pasteAtCursorPosition ? QRect() : image->bounds();
    KisPaintDeviceSP clip = KisClipboard::instance()->clip(fittingBounds, true);

    if (clip) {
        if (pasteAtCursorPosition) {
            const QPointF docPos = view->canvasBase()->canvasController()->currentCursorPosition();
            const QPointF imagePos = view->canvasBase()->coordinatesConverter()->documentToImage(docPos);

            const QPointF offset = (imagePos - QRectF(clip->exactBounds()).center()).toPoint();

            clip->setX(clip->x() + offset.x());
            clip->setY(clip->y() + offset.y());
        }

        KisImportCatcher::adaptClipToImageColorSpace(clip, image);
        KisPaintLayer *newLayer = new KisPaintLayer(image.data(), image->nextLayerName() + i18n("(pasted)"), OPACITY_OPAQUE_U8, clip);
        KisNodeSP aboveNode = view->activeLayer();
        KisNodeSP parentNode = aboveNode ? aboveNode->parent() : image->root();

        KUndo2Command *cmd = new KisImageLayerAddCommand(image, newLayer, parentNode, aboveNode);
        KisProcessingApplicator *ap = beginAction(view, cmd->text());
        ap->applyCommand(cmd, KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::NORMAL);
        endAction(ap, KisOperationConfiguration(id()).toXML());
    } else {
        // XXX: "Add saving of XML data for Paste of shapes"
        view->canvasBase()->toolProxy()->paste();
    }
}
