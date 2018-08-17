/*
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
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

#include "kis_tool_shape.h"

#include <QWidget>
#include <QLayout>
#include <QComboBox>
#include <QLabel>
#include <QGridLayout>

#include <KoUnit.h>
#include <KoShape.h>
#include <KoGradientBackground.h>
#include <KoCanvasBase.h>
#include <KoShapeController.h>
#include <KoColorBackground.h>
#include <KoPatternBackground.h>
#include <KoShapeStroke.h>
#include <KoDocumentResourceManager.h>
#include <KoPathShape.h>

#include <klocalizedstring.h>
#include <ksharedconfig.h>

#include <kis_debug.h>
#include <kis_canvas_resource_provider.h>
#include <brushengine/kis_paintop_registry.h>
#include <kis_paint_layer.h>
#include <kis_paint_device.h>
#include "kis_figure_painting_tool_helper.h"
#include <kis_node_query_path.h>

#include <KoSelectedShapesProxy.h>
#include <KoSelection.h>
#include <commands/KoKeepShapesSelectedCommand.h>
#include "kis_selection_mask.h"
#include "kis_shape_selection.h"


KisToolShape::KisToolShape(KoCanvasBase * canvas, const QCursor & cursor)
        : KisToolPaint(canvas, cursor)
{
    m_shapeOptionsWidget = 0;
}

KisToolShape::~KisToolShape()
{
    // in case the widget hasn't been shown
    if (m_shapeOptionsWidget && !m_shapeOptionsWidget->parent()) {
        delete m_shapeOptionsWidget;
    }
}

void KisToolShape::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{
    KisToolPaint::activate(toolActivation, shapes);
    m_configGroup =  KSharedConfig::openConfig()->group(toolId());
}


int KisToolShape::flags() const
{
    return KisTool::FLAG_USES_CUSTOM_COMPOSITEOP|KisTool::FLAG_USES_CUSTOM_PRESET
           |KisTool::FLAG_USES_CUSTOM_SIZE;
}

QWidget * KisToolShape::createOptionWidget()
{
    m_shapeOptionsWidget = new WdgGeometryOptions(0);

    m_shapeOptionsWidget->cmbOutline->setCurrentIndex(KisPainter::StrokeStyleBrush);

    //connect two combo box event. Inherited classes can call the slots to make appropriate changes
    connect(m_shapeOptionsWidget->cmbOutline, SIGNAL(currentIndexChanged(int)), this, SLOT(outlineSettingChanged(int)));
    connect(m_shapeOptionsWidget->cmbFill, SIGNAL(currentIndexChanged(int)), this, SLOT(fillSettingChanged(int)));

    m_shapeOptionsWidget->cmbOutline->setCurrentIndex(m_configGroup.readEntry("outlineType", 0));
    m_shapeOptionsWidget->cmbFill->setCurrentIndex(m_configGroup.readEntry("fillType", 0));

    //if both settings are empty, force the outline to brush so the tool will work when first activated
    if (  m_shapeOptionsWidget->cmbFill->currentIndex() == 0 &&
          m_shapeOptionsWidget->cmbOutline->currentIndex() == 0)
    {
        m_shapeOptionsWidget->cmbOutline->setCurrentIndex(1); // brush
    }

    return m_shapeOptionsWidget;
}

void KisToolShape::outlineSettingChanged(int value)
{
    m_configGroup.writeEntry("outlineType", value);
}

void KisToolShape::fillSettingChanged(int value)
{
    m_configGroup.writeEntry("fillType", value);
}

KisPainter::FillStyle KisToolShape::fillStyle(void)
{
    if (m_shapeOptionsWidget) {
        return static_cast<KisPainter::FillStyle>(m_shapeOptionsWidget->cmbFill->currentIndex());
    } else {
        return KisPainter::FillStyleNone;
    }
}

KisPainter::StrokeStyle KisToolShape::strokeStyle(void)
{
    if (m_shapeOptionsWidget) {
        return static_cast<KisPainter::StrokeStyle>(m_shapeOptionsWidget->cmbOutline->currentIndex());
    } else {
        return KisPainter::StrokeStyleNone;
    }
}

qreal KisToolShape::currentStrokeWidth() const
{
    const qreal sizeInPx =
        canvas()->resourceManager()->resource(KisCanvasResourceProvider::Size).toReal();

    return canvas()->unit().fromUserValue(sizeInPx);
}

KisToolShape::ShapeAddInfo KisToolShape::shouldAddShape(KisNodeSP currentNode) const
{
    ShapeAddInfo info;

    if (currentNode->inherits("KisShapeLayer")) {
        info.shouldAddShape = true;
    } else if (KisSelectionMask *mask = dynamic_cast<KisSelectionMask*>(currentNode.data())) {
        if (mask->selection()->hasShapeSelection()) {
            info.shouldAddShape = true;
            info.shouldAddSelectionShape = true;
        }
    }

    return info;
}

void KisToolShape::ShapeAddInfo::markAsSelectionShapeIfNeeded(KoShape *shape) const
{
    if (this->shouldAddSelectionShape) {
        shape->setUserData(new KisShapeSelectionMarker());
    }
}

void KisToolShape::addShape(KoShape* shape)
{
    KoImageCollection* imageCollection = canvas()->shapeController()->resourceManager()->imageCollection();
    switch(fillStyle()) {
        case KisPainter::FillStyleForegroundColor:
            shape->setBackground(QSharedPointer<KoColorBackground>(new KoColorBackground(currentFgColor().toQColor())));
            break;
        case KisPainter::FillStyleBackgroundColor:
            shape->setBackground(QSharedPointer<KoColorBackground>(new KoColorBackground(currentBgColor().toQColor())));
            break;
        case KisPainter::FillStylePattern:
            if (imageCollection) {
                QSharedPointer<KoPatternBackground> fill(new KoPatternBackground(imageCollection));
                if (currentPattern()) {
                    fill->setPattern(currentPattern()->pattern());
                    shape->setBackground(fill);
                }
            } else {
                shape->setBackground(QSharedPointer<KoShapeBackground>(0));
            }
            break;
        case KisPainter::FillStyleGradient:
            {
                QLinearGradient *gradient = new QLinearGradient(QPointF(0, 0), QPointF(1, 1));
                gradient->setCoordinateMode(QGradient::ObjectBoundingMode);
                gradient->setStops(currentGradient()->toQGradient()->stops());
                QSharedPointer<KoGradientBackground>  gradientFill(new KoGradientBackground(gradient));
                shape->setBackground(gradientFill);
            }
            break;
        case KisPainter::FillStyleNone:
        default:
            shape->setBackground(QSharedPointer<KoShapeBackground>(0));
            break;
    }

    switch (strokeStyle()) {
    case KisPainter::StrokeStyleNone:
        shape->setStroke(KoShapeStrokeModelSP());
        break;
    case KisPainter::StrokeStyleBrush: {
        KoShapeStrokeSP stroke(new KoShapeStroke());
        stroke->setLineWidth(currentStrokeWidth());
        stroke->setColor(canvas()->resourceManager()->foregroundColor().toQColor());
        shape->setStroke(stroke);
        break;
    }
    }

    KUndo2Command *parentCommand = new KUndo2Command();

    KoSelection *selection = canvas()->selectedShapesProxy()->selection();
    const QList<KoShape*> oldSelectedShapes = selection->selectedShapes();

    // reset selection on the newly added shape :)
    // TODO: think about moving this into controller->addShape?
    new KoKeepShapesSelectedCommand(oldSelectedShapes, {shape}, canvas()->selectedShapesProxy(), false, parentCommand);
    KUndo2Command *cmd = canvas()->shapeController()->addShape(shape, 0, parentCommand);
    parentCommand->setText(cmd->text());
    new KoKeepShapesSelectedCommand(oldSelectedShapes, {shape}, canvas()->selectedShapesProxy(), true, parentCommand);

    canvas()->addCommand(parentCommand);
}

void KisToolShape::addPathShape(KoPathShape* pathShape, const KUndo2MagicString& name)
{
    KisNodeSP node = currentNode();
    if (!node || !blockUntilOperationsFinished()) {
        return;
    }

    // Compute the outline
    KisImageSP image = this->image();
    QTransform matrix;
    matrix.scale(image->xRes(), image->yRes());
    matrix.translate(pathShape->position().x(), pathShape->position().y());
    QPainterPath mapedOutline = matrix.map(pathShape->outline());

    if (node->hasEditablePaintDevice()) {
        KisFigurePaintingToolHelper helper(name,
                                           image,
                                           node,
                                           canvas()->resourceManager(),
                                           strokeStyle(),
                                           fillStyle());
        helper.paintPainterPath(mapedOutline);
    } else if (node->inherits("KisShapeLayer")) {
        pathShape->normalize();
        addShape(pathShape);

    }

    notifyModified();
}
