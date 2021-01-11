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
#include "kis_processing_applicator.h"


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

    m_shapeOptionsWidget->angleSelectorRotation->setIncreasingDirection(KisAngleGauge::IncreasingDirection_Clockwise);

    m_shapeOptionsWidget->sldScale->setSuffix("%");
    m_shapeOptionsWidget->sldScale->setRange(0.0, 500.0, 2);
    m_shapeOptionsWidget->sldScale->setSingleStep(1.0);

    //connect two combo box event. Inherited classes can call the slots to make appropriate changes
    connect(m_shapeOptionsWidget->cmbOutline, SIGNAL(currentIndexChanged(int)), this, SLOT(outlineSettingChanged(int)));
    connect(m_shapeOptionsWidget->cmbFill, SIGNAL(currentIndexChanged(int)), this, SLOT(fillSettingChanged(int)));
    connect(m_shapeOptionsWidget->angleSelectorRotation, SIGNAL(angleChanged(qreal)), this, SLOT(patternRotationSettingChanged(qreal)));
    connect(m_shapeOptionsWidget->sldScale, SIGNAL(valueChanged(qreal)), this, SLOT(patternScaleSettingChanged(qreal)));

    m_shapeOptionsWidget->cmbOutline->setCurrentIndex(m_configGroup.readEntry("outlineType", 0));
    m_shapeOptionsWidget->cmbFill->setCurrentIndex(m_configGroup.readEntry("fillType", 0));
    m_shapeOptionsWidget->sldScale->setValue(m_configGroup.readEntry("patternTransformScale", 100));
    m_shapeOptionsWidget->angleSelectorRotation->setAngle(m_configGroup.readEntry("patternTransformRotation", 0));
    m_shapeOptionsWidget->angleSelectorRotation->setFlipOptionsMode(KisAngleSelector::FlipOptionsMode_MenuButton);

    //if both settings are empty, force the outline to brush so the tool will work when first activated
    if (  m_shapeOptionsWidget->cmbFill->currentIndex() == 0 &&
          m_shapeOptionsWidget->cmbOutline->currentIndex() == 0)
    {
        m_shapeOptionsWidget->cmbOutline->setCurrentIndex(1); // brush
    }

    bool enablePatternTransform = (m_shapeOptionsWidget->cmbFill->currentIndex() == int(KisToolShapeUtils::FillStylePattern));
    m_shapeOptionsWidget->gbPatternTransform->setEnabled(enablePatternTransform);

    return m_shapeOptionsWidget;
}

void KisToolShape::outlineSettingChanged(int value)
{
    m_configGroup.writeEntry("outlineType", value);
}

void KisToolShape::fillSettingChanged(int value)
{
    m_configGroup.writeEntry("fillType", value);
    bool enable = (value == int(KisToolShapeUtils::FillStylePattern));
    m_shapeOptionsWidget->gbPatternTransform->setEnabled(enable);
}

void KisToolShape::patternRotationSettingChanged(qreal value)
{
    m_configGroup.writeEntry("patternTransformRotation", value);
}

void KisToolShape::patternScaleSettingChanged(qreal value)
{
    m_configGroup.writeEntry("patternTransformScale", value);
}

KisToolShapeUtils::FillStyle KisToolShape::fillStyle()
{
    if (m_shapeOptionsWidget) {
        return static_cast<KisToolShapeUtils::FillStyle>(m_shapeOptionsWidget->cmbFill->currentIndex());
    } else {
        return KisToolShapeUtils::FillStyleNone;
    }
}

KisToolShapeUtils::StrokeStyle KisToolShape::strokeStyle()
{
    if (m_shapeOptionsWidget) {
        return static_cast<KisToolShapeUtils::StrokeStyle>(m_shapeOptionsWidget->cmbOutline->currentIndex());
    } else {
        return KisToolShapeUtils::StrokeStyleNone;
    }
}

QTransform KisToolShape::fillTransform()
{
    QTransform transform;

    if (m_shapeOptionsWidget) {
        transform.rotate(m_shapeOptionsWidget->angleSelectorRotation->angle());
        qreal scale = m_shapeOptionsWidget->sldScale->value()*0.01;
        transform.scale(scale, scale);
    }

    return transform;
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
    using namespace KisToolShapeUtils;

    KoImageCollection* imageCollection = canvas()->shapeController()->resourceManager()->imageCollection();
    switch(fillStyle()) {
        case FillStyleForegroundColor:
            shape->setBackground(QSharedPointer<KoColorBackground>(new KoColorBackground(currentFgColor().toQColor())));
            break;
        case FillStyleBackgroundColor:
            shape->setBackground(QSharedPointer<KoColorBackground>(new KoColorBackground(currentBgColor().toQColor())));
            break;
        case FillStylePattern:
            if (imageCollection) {
                QSharedPointer<KoPatternBackground> fill(new KoPatternBackground(imageCollection));
                if (currentPattern()) {
                    fill->setPattern(currentPattern()->pattern());
                    fill->setTransform(fillTransform());
                    shape->setBackground(fill);
                }
            } else {
                shape->setBackground(QSharedPointer<KoShapeBackground>(0));
            }
            break;
        case FillStyleNone:
            shape->setBackground(QSharedPointer<KoShapeBackground>(0));
            break;
    }

    switch (strokeStyle()) {
    case KisToolShapeUtils::StrokeStyleNone:
        shape->setStroke(KoShapeStrokeModelSP());
        break;
    case KisToolShapeUtils::StrokeStyleForeground:
    case KisToolShapeUtils::StrokeStyleBackground: {
        KoShapeStrokeSP stroke(new KoShapeStroke());
        stroke->setLineWidth(currentStrokeWidth());
        const QColor color = strokeStyle() == KisToolShapeUtils::StrokeStyleForeground ?
                    canvas()->resourceManager()->foregroundColor().toQColor() :
                    canvas()->resourceManager()->backgroundColor().toQColor();
        stroke->setColor(color);
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

    KisProcessingApplicator::runSingleCommandStroke(image(), cmd, KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::EXCLUSIVE);
}

void KisToolShape::addPathShape(KoPathShape* pathShape, const KUndo2MagicString& name)
{
    KisNodeSP node = currentNode();
    if (!node) {
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
                                           fillStyle(),
                                           fillTransform());
        helper.paintPainterPath(mapedOutline);
    } else if (node->inherits("KisShapeLayer")) {
        pathShape->normalize();
        addShape(pathShape);

    }
}
