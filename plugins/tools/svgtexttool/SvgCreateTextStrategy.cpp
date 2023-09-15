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
#include "commands/KoKeepShapesSelectedCommand.h"
#include "kis_global.h"
#include "kundo2command.h"

SvgCreateTextStrategy::SvgCreateTextStrategy(SvgTextTool *tool, const QPointF &clicked, double pressure)
    : KoInteractionStrategy(tool)
    , m_dragStart(clicked)
    , m_dragEnd(clicked)
    , m_pressure(pressure)
{
}

void SvgCreateTextStrategy::paint(QPainter &painter, const KoViewConverter &converter)
{
    const QTransform originalPainterTransform = painter.transform();
    painter.setTransform(converter.documentToView(), true);
    KisHandlePainterHelper handlePainter(&painter, originalPainterTransform, 0.0);

    const QPolygonF poly(QRectF(m_dragStart, m_dragEnd));
    handlePainter.setHandleStyle(KisHandleStyle::primarySelection());
    handlePainter.drawRubberLine(poly);
}

void SvgCreateTextStrategy::handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers)
{
    m_dragEnd = mouseLocation;
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
    if (rectangle.width() < lineHeight && rectangle.height() < lineHeight) {
        unwrappedText = true;
        // The pressure check might need to be configured somehow.
        if (rectangle.width() < 1 && rectangle.height() < 1 && m_pressure < 0.02) {
            tool->canvas()->updateCanvas(rectangle);
            return nullptr;
        }
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

    {
        const Qt::Alignment halign = tool->horizontalAlign();
        const bool isRtl = tool->isRtl();

        if (writingMode == KoSvgText::HorizontalTB) {
            rectangle.setTop(rectangle.top() + ascender);
            if (halign & Qt::AlignCenter) {
                rectangle.setLeft(rectangle.center().x());
            } else if ((halign & Qt::AlignRight && !isRtl) || (halign & Qt::AlignLeft && isRtl)) {
                qreal right = rectangle.right();
                rectangle.setRight(right+10);
                rectangle.setLeft(right);
            }
        } else {
            if (writingMode == KoSvgText::VerticalRL) {
                qreal right = rectangle.right() - (lineHeight*0.5);
                rectangle.setRight(right+10);
                rectangle.setLeft(right);

            } else {
                rectangle.setLeft(rectangle.left() + (lineHeight*0.5));
            }

            if (halign & Qt::AlignCenter) {
                rectangle.setTop(rectangle.center().y());
            } else if (halign & Qt::AlignRight) {
                qreal bottom = rectangle.bottom();
                rectangle.setBottom(bottom+10);
                rectangle.setTop(bottom);
            }
        }

        params->setProperty("shapeRect", QVariant(rectangle));
    }

    KoShape *textShape = factory->createShape( params, tool->canvas()->shapeController()->resourceManager());

    KUndo2Command *parentCommand = new KUndo2Command();

    new KoKeepShapesSelectedCommand(tool->koSelection()->selectedShapes(), {}, tool->canvas()->selectedShapesProxy(), false, parentCommand);

    KUndo2Command *cmd = tool->canvas()->shapeController()->addShape(textShape, 0, parentCommand);
    parentCommand->setText(cmd->text());

    new KoKeepShapesSelectedCommand({}, {textShape}, tool->canvas()->selectedShapesProxy(), true, parentCommand);

    return parentCommand;
}

void SvgCreateTextStrategy::cancelInteraction()
{
    const QRectF updateRect = QRectF(m_dragStart, m_dragEnd).normalized();
    tool()->canvas()->updateCanvas(updateRect);
}

void SvgCreateTextStrategy::finishInteraction(Qt::KeyboardModifiers modifiers)
{
    m_modifiers = modifiers;
}
