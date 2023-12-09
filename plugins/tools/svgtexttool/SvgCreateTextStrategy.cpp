/*
 * SPDX-FileCopyrightText: 2023 Alvin Wong <alvin@alvinhc.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "SvgCreateTextStrategy.h"
#include "SvgTextTool.h"

#include <QFontDatabase>
#include <QRectF>
#include <QTimer>

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
#include "kis_global.h"
#include "kundo2command.h"

SvgCreateTextStrategy::SvgCreateTextStrategy(SvgTextTool *tool, const QPointF &clicked)
    : KoInteractionStrategy(tool)
    , m_dragStart(clicked)
    , m_dragEnd(clicked)
{
    const QFontMetrics fontMetrics = QFontMetrics(tool->defaultFont());
    double lineHeight = (fontMetrics.lineSpacing() / fontMetrics.fontDpi()) * 72.0;
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

    const QFontMetrics fontMetrics = QFontMetrics(tool->defaultFont());
    double ascender = fontMetrics.ascent();
    ascender += fontMetrics.leading()/2;
    ascender = (ascender / fontMetrics.fontDpi()) * 72.0; // 72 points in an inch.
    double lineHeight = (fontMetrics.lineSpacing() / fontMetrics.fontDpi()) * 72.0;
    const KoSvgText::WritingMode writingMode = KoSvgText::WritingMode(tool->writingMode());

    bool unwrappedText = m_modifiers.testFlag(Qt::ControlModifier);
    if (rectangle.width() < m_minSizeInline.width() && rectangle.height() < m_minSizeInline.height()) {
        unwrappedText = true;
    }
    QString extraProperties;
    if (!unwrappedText) {
        if (writingMode == KoSvgText::HorizontalTB) {
            extraProperties = QLatin1String("inline-size:%1;").arg(QString::number(rectangle.width()));
        } else {
            extraProperties = QLatin1String("inline-size:%1;").arg(QString::number(rectangle.height()));
        }
    }
    KoShapeFactoryBase *factory = KoShapeRegistry::instance()->value("KoSvgTextShapeID");
    KoProperties *params = new KoProperties();//Fill these with "svgText", "defs" and "shapeRect"
    params->setProperty("defs", QVariant(tool->generateDefs(extraProperties)));

    QPointF origin = rectangle.topLeft();

    {
        const Qt::Alignment halign = tool->horizontalAlign();
        const bool isRtl = tool->isRtl();

        if (writingMode == KoSvgText::HorizontalTB) {
            origin.setY(rectangle.top() + ascender);
            if (halign & Qt::AlignCenter) {
                origin.setX(rectangle.center().x());
            } else if ((halign & Qt::AlignRight && !isRtl) || (halign & Qt::AlignLeft && isRtl)) {
                origin.setX(rectangle.right());
            }
        } else {
            if (writingMode == KoSvgText::VerticalRL) {
                origin.setX(rectangle.right() - (lineHeight*0.5));
            } else {
                origin.setX(rectangle.left() + (lineHeight*0.5));
            }

            if (halign & Qt::AlignCenter) {
                origin.setY(rectangle.center().y());
            } else if (halign & Qt::AlignRight) {
                origin.setY(rectangle.bottom());
            }
        }
    }
    if (!rectangle.contains(origin) && unwrappedText) {
        origin = writingMode == KoSvgText::HorizontalTB? QPointF(origin.x(), rectangle.bottom()): QPointF(rectangle.center().x(), origin.y());
    }
    params->setProperty("shapeRect", QVariant(rectangle));
    params->setProperty("origin", QVariant(origin));

    KoShape *textShape = factory->createShape( params, tool->canvas()->shapeController()->resourceManager());

    KUndo2Command *parentCommand = new KUndo2Command();

    new KoKeepShapesSelectedCommand(tool->koSelection()->selectedShapes(), {}, tool->canvas()->selectedShapesProxy(), false, parentCommand);

    KUndo2Command *cmd = tool->canvas()->shapeController()->addShape(textShape, 0, parentCommand);
    parentCommand->setText(cmd->text());

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
