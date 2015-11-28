/* This file is part of the KDE project
 * Copyright (C) 2008,2011 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2009 Carlos Licea <carlos.licea@kdemail.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "GuidesTool.h"
#include "GuidesToolFactory.h" // for the Id
#include "GuidesToolOptionWidget.h"
#include "InsertGuidesToolOptionWidget.h"

#include <KoPointerEvent.h>
#include <KoCanvasBase.h>
#include <KoCanvasController.h>
#include <KoCanvasResourceManager.h>
#include <KoViewConverter.h>
#include <KoGuidesData.h>
#include <KoToolManager.h>

#include <QDebug>

#include <QPainter>

GuidesTool::GuidesTool(KoCanvasBase *canvas)
    : KoToolBase(canvas)
    , m_orientation(Qt::Horizontal)
    , m_index(-1)
    , m_position(0)
    , m_mode(EditGuide)
    , m_options(0)
    , m_isMoving(false)
{
}

GuidesTool::~GuidesTool()
{
}

void GuidesTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    if (m_mode == EditGuide && m_index == -1) {
        return;
    }

    KoCanvasController *controller = canvas()->canvasController();
    QPoint documentOrigin = canvas()->documentOrigin();
    QPoint canvasOffset(controller->canvasOffsetX(), controller->canvasOffsetY());

    QPointF start, end;
    if (m_orientation == Qt::Horizontal) {
        qreal left = -canvasOffset.x() - documentOrigin.x();
        qreal right = left + canvas()->canvasWidget()->width();
        start = QPointF(left, converter.documentToViewY(m_position));
        end = QPointF(right, converter.documentToViewY(m_position));
    } else {
        qreal top = -canvasOffset.y() - documentOrigin.y();
        qreal bottom = top + canvas()->canvasWidget()->height();
        start = QPointF(converter.documentToViewX(m_position), top);
        end = QPointF(converter.documentToViewX(m_position), bottom);
    }
    painter.setPen(Qt::red);
    painter.drawLine(start, end);
}

void GuidesTool::repaintDecorations()
{
    canvas()->updateCanvas(updateRectFromGuideLine(m_position, m_orientation));
}

QRectF GuidesTool::updateRectFromGuideLine(qreal position, Qt::Orientation orientation)
{
    QRectF rect;
    KoCanvasController *controller = canvas()->canvasController();
    QPoint documentOrigin = canvas()->documentOrigin();
    QPoint canvasOffset(controller->canvasOffsetX(), controller->canvasOffsetY());
    if (orientation == Qt::Horizontal) {
        qreal pixelBorder = canvas()->viewConverter()->viewToDocumentY(2.0);
        rect.setTop(position - pixelBorder);
        rect.setBottom(position + pixelBorder);
        rect.setLeft(canvas()->viewConverter()->viewToDocumentX(-canvasOffset.x() - documentOrigin.x()));
        rect.setWidth(canvas()->viewConverter()->viewToDocumentX(canvas()->canvasWidget()->width()));
    } else {
        qreal pixelBorder = canvas()->viewConverter()->viewToDocumentX(2.0);
        rect.setLeft(position - pixelBorder);
        rect.setRight(position + pixelBorder);
        rect.setTop(canvas()->viewConverter()->viewToDocumentY(-canvasOffset.y() - documentOrigin.y()));
        rect.setHeight(canvas()->viewConverter()->viewToDocumentY(canvas()->canvasWidget()->height()));
    }

    return rect;
}

void GuidesTool::activate(ToolActivation /*toolActivation*/, const QSet<KoShape *> &)
{
    if (m_index >= 0) {
        useCursor(m_orientation == Qt::Horizontal ? Qt::SizeVerCursor : Qt::SizeHorCursor);
    } else {
        useCursor(Qt::ArrowCursor);
    }

    if (m_options) {
        KoGuidesData *guidesData = canvas()->guidesData();
        if (!guidesData) {
            return;
        }
        m_options->setHorizontalGuideLines(guidesData->horizontalGuideLines());
        m_options->setVerticalGuideLines(guidesData->verticalGuideLines());
        m_options->selectGuideLine(m_orientation, m_index);
        m_options->setUnit(canvas()->unit());
    }
}

void GuidesTool::deactivate()
{
    canvas()->canvasWidget()->releaseMouse();
    m_mode = EditGuide;
    m_index = -1;
}

void GuidesTool::mousePressEvent(KoPointerEvent *event)
{
    GuideLine line = guideLineAtPosition(event->point);
    if (line.second >= 0) {
        guideLineSelected(line.first, static_cast<int>(line.second));
        m_isMoving = true;
    }
}

void GuidesTool::mouseMoveEvent(KoPointerEvent *event)
{
    if (m_mode == EditGuide && ! m_isMoving) {
        GuideLine line = guideLineAtPosition(event->point);
        if (line.second < 0) {
            useCursor(Qt::ArrowCursor);
            setStatusText(i18n("Double click to add guide line."));
        } else {
            useCursor(line.first == Qt::Horizontal ? Qt::SizeVerCursor : Qt::SizeHorCursor);
            setStatusText(i18n("Click and drag to move guide line. Double click to remove guide line."));
        }
    } else {
        setStatusText("");
        repaintDecorations();
        m_position = m_orientation == Qt::Horizontal ? event->point.y() : event->point.x();
        updateGuidePosition(m_position);
        repaintDecorations();
    }
}

void GuidesTool::mouseReleaseEvent(KoPointerEvent *event)
{
    Q_UNUSED(event);

    KoGuidesData *guidesData = canvas()->guidesData();
    if (!guidesData) {
        event->ignore();
        return;
    }

    if (m_mode == AddGuide) {
        // add the new guide line
        guidesData->addGuideLine(m_orientation, m_position);
    } else if (m_mode == EditGuide) {
        if (m_isMoving) {
            m_isMoving = false;
            if (m_orientation == Qt::Horizontal) {
                m_options->setHorizontalGuideLines(guidesData->horizontalGuideLines());
            } else {
                m_options->setVerticalGuideLines(guidesData->verticalGuideLines());
            }
            m_options->selectGuideLine(m_orientation, m_index);
        }
    }

    if (m_mode != EditGuide) {
        emit done();
    }
}

void GuidesTool::mouseDoubleClickEvent(KoPointerEvent *event)
{
    KoGuidesData *guidesData = canvas()->guidesData();
    if (!guidesData) {
        event->ignore();
        return;
    }

    repaintDecorations();

    // get guide line at position
    GuideLine line = guideLineAtPosition(event->point);
    if (line.second < 0) {
        // no guide line hit -> insert a new one
        m_orientation = m_options->orientation();
        m_position = m_orientation == Qt::Horizontal ? event->point.y() : event->point.x();
        // no guide line hit -> insert a new one
        guidesData->addGuideLine(m_orientation, m_position);
        if (m_orientation == Qt::Horizontal) {
            m_options->setHorizontalGuideLines(guidesData->horizontalGuideLines());
            m_index = guidesData->horizontalGuideLines().count() - 1;
        } else {
            m_options->setVerticalGuideLines(guidesData->verticalGuideLines());
            m_index = guidesData->verticalGuideLines().count() - 1;
        }
        m_options->selectGuideLine(m_orientation, m_index);
    } else {
        // guide line hit -> remove it
        QList<qreal> lines;
        if (line.first == Qt::Horizontal) {
            lines = guidesData->horizontalGuideLines();
            lines.removeAt(line.second);
            guidesData->setHorizontalGuideLines(lines);
            m_options->setHorizontalGuideLines(lines);
            m_index = -1;
        } else {
            lines = guidesData->verticalGuideLines();
            lines.removeAt(line.second);
            guidesData->setVerticalGuideLines(lines);
            m_options->setVerticalGuideLines(lines);
            m_index = -1;
        }
    }

    repaintDecorations();
}

void GuidesTool::createGuideLine(Qt::Orientation orientation, qreal position)
{
    m_orientation = orientation;
    m_index = -1;
    m_position = position;
    m_mode = AddGuide;

    KoToolManager::instance()->switchToolRequested(GuidesTool_ID);

    // grab the mouse so we get mouse events as the dragging started on a ruler
    canvas()->canvasWidget()->grabMouse();
}

void GuidesTool::moveGuideLine(Qt::Orientation orientation, int index)
{
    m_orientation = orientation;
    m_index = index;
    m_mode = MoveGuide;
}

void GuidesTool::editGuideLine(Qt::Orientation orientation, int index)
{
    m_orientation = orientation;
    m_index = index;
    m_mode = EditGuide;
}

void GuidesTool::updateGuidePosition(qreal position)
{
    if (m_mode == MoveGuide || m_mode == EditGuide) {
        KoGuidesData *guidesData = canvas()->guidesData();
        if (guidesData) {
            if (m_orientation == Qt::Horizontal) {
                QList<qreal> guideLines = guidesData->horizontalGuideLines();
                if (m_index >= 0 && m_index < guideLines.count()) {
                    guideLines[m_index] = position;
                    guidesData->setHorizontalGuideLines(guideLines);
                }
            } else {
                QList<qreal> guideLines = guidesData->verticalGuideLines();
                if (m_index >= 0 && m_index < guideLines.count()) {
                    guideLines[m_index] = position;
                    guidesData->setVerticalGuideLines(guideLines);
                }
            }
        }
    }
}

void GuidesTool::guideLineSelected(Qt::Orientation orientation, int index)
{
    KoGuidesData *guidesData = canvas()->guidesData();
    if (!guidesData) {
        return;
    }

    repaintDecorations();

    m_orientation = orientation;
    m_index = index;

    if (m_orientation == Qt::Horizontal) {
        m_position = guidesData->horizontalGuideLines().value(index);
    } else {
        m_position = guidesData->verticalGuideLines().value(index);
    }

    repaintDecorations();
}

void GuidesTool::guideLinesChanged(Qt::Orientation orientation)
{
    KoGuidesData *guidesData = canvas()->guidesData();
    if (!guidesData) {
        return;
    }

    repaintDecorations();

    if (orientation == Qt::Horizontal) {
        guidesData->setHorizontalGuideLines(m_options->horizontalGuideLines());
    } else {
        guidesData->setVerticalGuideLines(m_options->verticalGuideLines());
    }

    if (orientation == m_orientation) {
        QList<qreal> lines;
        if (m_orientation == Qt::Horizontal) {
            lines = guidesData->horizontalGuideLines();
        } else {
            lines = guidesData->verticalGuideLines();
        }

        int oldIndex = m_index;

        if (lines.count() == 0) {
            m_index = -1;
        } else if (m_index >= lines.count()) {
            m_index = 0;
        }

        if (m_index >= 0) {
            m_position = lines[m_index];
        }

        if (oldIndex != m_index) {
            m_options->selectGuideLine(m_orientation, m_index);
        }
    }

    repaintDecorations();
}

GuidesTool::GuideLine GuidesTool::guideLineAtPosition(const QPointF &position)
{
    int index = -1;
    Qt::Orientation orientation = Qt::Horizontal;

    // check if we are on a guide line
    KoGuidesData *guidesData = canvas()->guidesData();
    if (guidesData && guidesData->showGuideLines()) {
        qreal minDistance = canvas()->viewConverter()->viewToDocumentX(handleRadius());
        int i = 0;
        foreach (qreal guidePos, guidesData->horizontalGuideLines()) {
            qreal distance = qAbs(guidePos - position.y());
            if (distance < minDistance) {
                orientation = Qt::Horizontal;
                index = i;
                minDistance = distance;
            }
            i++;
        }
        i = 0;
        foreach (qreal guidePos, guidesData->verticalGuideLines()) {
            qreal distance = qAbs(guidePos - position.x());
            if (distance < minDistance) {
                orientation = Qt::Vertical;
                index = i;
                minDistance = distance;
            }
            i++;
        }
    }

    return QPair<Qt::Orientation, int>(orientation, index);
}

void GuidesTool::canvasResourceChanged(int key, const QVariant &res)
{
    if (key == KoCanvasResourceManager::Unit) {
        if (m_options) {
            m_options->setUnit(res.value<KoUnit>());
        }
    }
}

QList<QPointer<QWidget> > GuidesTool::createOptionWidgets()
{
    QList<QPointer<QWidget> > optionwidgets;
    m_options = new GuidesToolOptionWidget();
    m_options->setWindowTitle(i18n("Guides Editor"));
    connect(m_options, SIGNAL(guideLineSelected(Qt::Orientation,int)),
            this, SLOT(guideLineSelected(Qt::Orientation,int)));
    connect(m_options, SIGNAL(guideLinesChanged(Qt::Orientation)),
            this, SLOT(guideLinesChanged(Qt::Orientation)));
    optionwidgets.append(m_options);

    m_insert = new InsertGuidesToolOptionWidget();
    m_insert->setWindowTitle(i18n("Guides Insertor"));
    connect(m_insert, SIGNAL(createGuides(GuidesTransaction*)),
            this, SLOT(insertorCreateGuidesSlot(GuidesTransaction*)));
    optionwidgets.append(m_insert);

    return optionwidgets;
}

void GuidesTool::insertorCreateGuidesSlot(GuidesTransaction *result)
{
    KoGuidesData *guidesData = canvas()->guidesData();
    const QSizeF pageSize = canvas()->resourceManager()->sizeResource(KoCanvasResourceManager::PageSize);

    QList< qreal > verticalLines;
    QList< qreal > horizontalLines;
    //save previous lines if requested
    if (!result->erasePreviousGuides) {
        verticalLines.append(guidesData->verticalGuideLines());
        horizontalLines.append(guidesData->horizontalGuideLines());
    } else {
        // trigger repaint at old guide positions
        Q_FOREACH (qreal pos, guidesData->verticalGuideLines()) {
            canvas()->updateCanvas(updateRectFromGuideLine(pos, Qt::Vertical));
        }
        Q_FOREACH (qreal pos, guidesData->horizontalGuideLines()) {
            canvas()->updateCanvas(updateRectFromGuideLine(pos, Qt::Horizontal));
        }
    }

    //vertical guides
    if (result->insertVerticalEdgesGuides) {
        verticalLines << 0 << pageSize.width();
    }

    int lastI = result->verticalGuides;
    qreal verticalJumps = pageSize.width() / (qreal)(result->verticalGuides + 1);
    for (int i = 1; i <= lastI; ++i) {
        verticalLines << verticalJumps * (qreal)i;
    }
    guidesData->setVerticalGuideLines(verticalLines);

    //horizontal guides
    lastI = result->horizontalGuides;
    if (result->insertHorizontalEdgesGuides) {
        horizontalLines << 0 << pageSize.height();
    }

    qreal horizontalJumps = pageSize.height() / (qreal)(result->horizontalGuides + 1);
    for (int i = 1; i <= lastI; ++i) {
        horizontalLines << horizontalJumps * (qreal)i;
    }
    guidesData->setHorizontalGuideLines(horizontalLines);

    // trigger repaint at new guide positions
    Q_FOREACH (qreal pos, guidesData->verticalGuideLines()) {
        canvas()->updateCanvas(updateRectFromGuideLine(pos, Qt::Vertical));
    }
    Q_FOREACH (qreal pos, guidesData->horizontalGuideLines()) {
        canvas()->updateCanvas(updateRectFromGuideLine(pos, Qt::Horizontal));
    }

    m_orientation = m_options->orientation();
    m_index = m_orientation == Qt::Horizontal ? horizontalLines.count() - 1 : verticalLines.count() - 1;
    m_options->setHorizontalGuideLines(horizontalLines);
    m_options->setVerticalGuideLines(verticalLines);
    m_options->selectGuideLine(m_orientation, m_index);

    delete result;
}
