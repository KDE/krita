/*
 * SPDX-FileCopyrightText: 2023 Alvin Wong <alvin@alvinhc.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "SvgCreateTextStrategy.h"
#include "SvgTextTool.h"

#include <KoFontRegistry.h>

#include "KisHandlePainterHelper.h"
#include "KoCanvasBase.h"
#include "KoProperties.h"
#include "KoSelection.h"
#include "KoShapeController.h"
#include "KoShapeFactoryBase.h"
#include "KoShapeRegistry.h"
#include "KoToolBase.h"
#include "KoViewConverter.h"
#include "KoSnapGuide.h"
#include "commands/KoKeepShapesSelectedCommand.h"
#include "commands/KoShapeMoveCommand.h"
#include "KoSvgChangeTextContoursCommand.h"
#include "kis_global.h"
#include "kundo2command.h"

SvgCreateTextStrategy::SvgCreateTextStrategy(SvgTextTool *tool, const QPointF &clicked, KoShape *shape)
    : KoInteractionStrategy(tool)
    , m_dragStart(clicked)
    , m_dragEnd(clicked)
    , m_flowShape(shape)
{
    KoSvgTextProperties properties = tool->propertiesForNewText();
    properties.inheritFrom(KoSvgTextProperties::defaultProperties(), true);
    const KoSvgText::FontMetrics fontMetrics = properties.metrics(true);
    const qreal ftMultiplier = properties.fontSize().value / fontMetrics.fontSize;
    const double lineHeight = (fontMetrics.lineGap+fontMetrics.ascender+fontMetrics.descender)*ftMultiplier;
    m_minSizeInline = {lineHeight, lineHeight};
}

void SvgCreateTextStrategy::paint(QPainter &painter, const KoViewConverter &converter)
{
    const QTransform originalPainterTransform = painter.transform();
    painter.setTransform(converter.documentToView(), true);
    KisHandlePainterHelper handlePainter(&painter, originalPainterTransform, 0.0, decorationThickness());

    const QPolygonF poly(QRectF(m_dragStart, m_dragEnd));
    handlePainter.setHandleStyle(KisHandleStyle::primarySelection());
    handlePainter.drawRubberLine(poly);
}

void SvgCreateTextStrategy::handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers)
{
    m_dragEnd = this->tool()->canvas()->snapGuide()->snap(mouseLocation, modifiers);
    m_modifiers = modifiers;
    const QRectF updateRect = QRectF(m_dragStart, m_dragEnd).normalized();
    tool()->canvas()->updateCanvas(kisGrowRect(updateRect, 100));
}

KUndo2Command *SvgCreateTextStrategy::createCommand()
{
    SvgTextTool *const tool = qobject_cast<SvgTextTool *>(this->tool());

    QRectF rectangle = QRectF(m_dragStart, m_dragEnd).normalized();

    KoSvgTextProperties properties = tool->propertiesForNewText();
    KoSvgTextProperties resolvedProperties = properties;
    resolvedProperties.inheritFrom(KoSvgTextProperties::defaultProperties(), true);

    const KoSvgText::FontMetrics fontMetrics = properties.metrics(true);
    const qreal ftMultiplier = resolvedProperties.fontSize().value / fontMetrics.fontSize;
    double ascender = fontMetrics.ascender;
    ascender += fontMetrics.lineGap/2;
    ascender *= ftMultiplier;
    const double lineHeight = m_minSizeInline.width();
    const KoSvgText::WritingMode writingMode = KoSvgText::WritingMode(properties.propertyOrDefault(KoSvgTextProperties::WritingModeId).toInt());

    bool unwrappedText = m_modifiers.testFlag(Qt::ControlModifier);
    if (rectangle.width() < m_minSizeInline.width() && rectangle.height() < m_minSizeInline.height()) {
        unwrappedText = true;
    }
    if (!unwrappedText) {
        KoSvgText::AutoValue val;
        val.isAuto = false;
        val.customValue = writingMode == KoSvgText::HorizontalTB? rectangle.width(): rectangle.height();
        properties.setProperty(KoSvgTextProperties::InlineSizeId, QVariant::fromValue(val));
    }
    if (writingMode != KoSvgText::HorizontalTB) {
        properties.setProperty(KoSvgTextProperties::TextOrientationId, KoSvgText::OrientationUpright);
    }
    // Ensure white space is set to pre-wrap if unspecified.
    if (!properties.hasProperty(KoSvgTextProperties::TextCollapseId)) {
        properties.setProperty(KoSvgTextProperties::TextCollapseId, KoSvgText::Preserve);
    }
    if (!properties.hasProperty(KoSvgTextProperties::TextWrapId)) {
        properties.setProperty(KoSvgTextProperties::TextWrapId, KoSvgText::Wrap);
    }

    KoShapeFactoryBase *factory = KoShapeRegistry::instance()->value("KoSvgTextShapeID");
    KoProperties *params = new KoProperties();//Fill these with "svgText", "defs" and "shapeRect"
    params->setProperty("defs", QVariant(tool->generateDefs(properties)));

    QPointF origin = rectangle.topLeft();

    {
        const KoSvgText::TextAnchor halign = KoSvgText::TextAnchor(properties.propertyOrDefault(KoSvgTextProperties::TextAnchorId).toInt());
        const bool isRtl = KoSvgText::Direction(properties.propertyOrDefault(KoSvgTextProperties::DirectionId).toInt()) == KoSvgText::DirectionRightToLeft;

        if (writingMode == KoSvgText::HorizontalTB) {
            origin.setY(rectangle.top() + ascender);
            if (halign == KoSvgText::AnchorMiddle) {
                origin.setX(rectangle.center().x());
            } else if ((halign == KoSvgText::AnchorEnd && !isRtl) || (halign == KoSvgText::AnchorStart && isRtl)) {
                origin.setX(rectangle.right());
            }
        } else {
            if (writingMode == KoSvgText::VerticalRL) {
                origin.setX(rectangle.right() - (lineHeight*0.5));
            } else {
                origin.setX(rectangle.left() + (lineHeight*0.5));
            }

            if (halign == KoSvgText::AnchorMiddle) {
                origin.setY(rectangle.center().y());
            } else if (halign == KoSvgText::AnchorEnd) {
                origin.setY(rectangle.bottom());
            }
        }
    }
    if (!rectangle.contains(origin) && unwrappedText) {
        origin = writingMode == KoSvgText::HorizontalTB? QPointF(origin.x(), rectangle.bottom()): QPointF(rectangle.center().x(), origin.y());
    }
    params->setProperty("shapeRect", QVariant(rectangle));
    params->setProperty("origin", QVariant(origin));

    KoSvgTextShape *textShape = dynamic_cast<KoSvgTextShape *>(factory->createShape( params, tool->canvas()->shapeController()->resourceManager()));

    KUndo2Command *parentCommand = new KUndo2Command();

    new KoKeepShapesSelectedCommand(tool->koSelection()->selectedShapes(), {}, tool->canvas()->selectedShapesProxy(), false, parentCommand);

    KUndo2Command *cmd = tool->canvas()->shapeController()->addShape(textShape, 0, parentCommand);
    parentCommand->setText(cmd->text());

    if (m_flowShape) {
        textShape->setPosition(m_flowShape->absolutePosition(KoFlake::TopLeft));
        QList<QPointF> pos {m_flowShape->absolutePosition(KoFlake::TopLeft)};
        QList<QPointF> newPos {QPointF()};
        QList<KoShape*> shapes{m_flowShape};
        new KoShapeMoveCommand(shapes, pos, newPos, KoFlake::TopLeft, parentCommand);
        if (m_flowShape->parent()) {
            // text is not a shape container, but we do need to remove the shape from the canvas properly.
            tool->canvas()->shapeController()->removeShape(m_flowShape, parentCommand);
        }
        new KoSvgChangeTextContoursCommand(textShape, shapes, true, parentCommand);
    }

    new KoKeepShapesSelectedCommand({}, {textShape}, tool->canvas()->selectedShapesProxy(), true, parentCommand);
    tool->canvas()->snapGuide()->reset();

    return parentCommand;
}

void SvgCreateTextStrategy::cancelInteraction()
{
    tool()->canvas()->snapGuide()->reset();
    const QRectF updateRect = QRectF(m_dragStart, m_dragEnd).normalized();
    tool()->canvas()->updateCanvas(updateRect);
}

void SvgCreateTextStrategy::finishInteraction(Qt::KeyboardModifiers modifiers)
{
    m_modifiers = modifiers;
}

bool SvgCreateTextStrategy::draggingInlineSize()
{
    QRectF rectangle = QRectF(m_dragStart, m_dragEnd).normalized();
    return (rectangle.width() >= m_minSizeInline.width() || rectangle.height() >= m_minSizeInline.height()) && !m_modifiers.testFlag(Qt::ControlModifier);
}

bool SvgCreateTextStrategy::hasWrappingShape()
{
    return (m_flowShape)? true: false;
}
