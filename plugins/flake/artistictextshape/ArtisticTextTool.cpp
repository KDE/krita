/* This file is part of the KDE project
 * Copyright (C) 2007,2011 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 Rob Buis <buis@kde.org>
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

#include "ArtisticTextTool.h"
#include "ArtisticTextToolSelection.h"
#include "AttachTextToPathCommand.h"
#include "DetachTextFromPathCommand.h"
#include "AddTextRangeCommand.h"
#include "RemoveTextRangeCommand.h"
#include "ArtisticTextShapeConfigWidget.h"
#include "ArtisticTextShapeOnPathWidget.h"
#include "MoveStartOffsetStrategy.h"
#include "SelectTextStrategy.h"
#include "ChangeTextOffsetCommand.h"
#include "ChangeTextFontCommand.h"
#include "ChangeTextAnchorCommand.h"
#include "ReplaceTextRangeCommand.h"

#include <KoCanvasBase.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoSelectedShapesProxy.h>
#include <KoPointerEvent.h>
#include <KoPathShape.h>
#include <KoShapeBackground.h>
#include <KoShapeController.h>
#include <KoShapeContainer.h>
#include <KoInteractionStrategy.h>
#include <KoIcon.h>
#include <KoViewConverter.h>
#include "kis_action_registry.h"

#include <klocalizedstring.h>
#include <kstandardaction.h>
#include <QAction>
#include <QDebug>

#include <QGridLayout>
#include <QToolButton>
#include <QCheckBox>
#include <QPainter>
#include <QPainterPath>
#include <kundo2command.h>

#include <float.h>
#include <math.h>

const int BlinkInterval = 500;

static bool hit(const QKeySequence &input, KStandardShortcut::StandardShortcut shortcut)
{
    foreach (const QKeySequence &ks, KStandardShortcut::shortcut(shortcut)) {
        if (input == ks) {
            return true;
        }
    }
    return false;
}

ArtisticTextTool::ArtisticTextTool(KoCanvasBase *canvas)
    : KoToolBase(canvas)
    , m_selection(canvas, this)
    , m_currentShape(0)
    , m_hoverText(0)
    , m_hoverPath(0)
    , m_hoverHandle(false)
    , m_textCursor(-1)
    , m_showCursor(true)
    , m_currentStrategy(0)
{
    KisActionRegistry *actionRegistry = KisActionRegistry::instance();
    m_detachPath  = actionRegistry->makeQAction("artistictext_detach_from_path", this);
    m_detachPath->setEnabled(false);
    connect(m_detachPath, SIGNAL(triggered()), this, SLOT(detachPath()));
//    addAction("artistictext_detach_from_path", m_detachPath);

    m_convertText  = actionRegistry->makeQAction("artistictext_convert_to_path", this);
    m_convertText->setEnabled(false);
    connect(m_convertText, SIGNAL(triggered()), this, SLOT(convertText()));
//    addAction("artistictext_convert_to_path", m_convertText);

    m_fontBold = actionRegistry->makeQAction("artistictext_font_bold", this);
    connect(m_fontBold, SIGNAL(toggled(bool)), this, SLOT(toggleFontBold(bool)));
//    addAction("artistictext_font_bold", m_fontBold);

    m_fontItalic = actionRegistry->makeQAction("artistictext_font_italic", this);
    connect(m_fontItalic, SIGNAL(toggled(bool)), this, SLOT(toggleFontItalic(bool)));
//    addAction("artistictext_font_italic", m_fontItalic);

    m_superScript = actionRegistry->makeQAction("artistictext_superscript", this);
    connect(m_superScript, SIGNAL(triggered()), this, SLOT(setSuperScript()));
//    addAction("artistictext_superscript", m_superScript);

    m_subScript = actionRegistry->makeQAction("artistictext_subscript", this);
    connect(m_subScript, SIGNAL(triggered()), this, SLOT(setSubScript()));
//    addAction("artistictext_subscript", m_subScript);

    QAction *anchorStart = actionRegistry->makeQAction("artistictext_anchor_start", this);
    anchorStart->setData(ArtisticTextShape::AnchorStart);
//    addAction("artistictext_anchor_start", anchorStart);

    QAction *anchorMiddle = actionRegistry->makeQAction("artistictext_anchor_middle", this);
    anchorMiddle->setData(ArtisticTextShape::AnchorMiddle);
//    addAction("artistictext_anchor_middle", anchorMiddle);

    QAction *anchorEnd = actionRegistry->makeQAction("artistictext_anchor_end", this);
    anchorEnd->setData(ArtisticTextShape::AnchorEnd);
//    addAction("artistictext_anchor_end", anchorEnd);

    m_anchorGroup = new QActionGroup(this);
    m_anchorGroup->setExclusive(true);
    m_anchorGroup->addAction(anchorStart);
    m_anchorGroup->addAction(anchorMiddle);
    m_anchorGroup->addAction(anchorEnd);
    connect(m_anchorGroup, SIGNAL(triggered(QAction*)), this, SLOT(anchorChanged(QAction*)));

    connect(canvas->selectedShapesProxy(), SIGNAL(selectionContentChanged()), this, SLOT(textChanged()));

//    addAction("edit_select_all", KStandardAction::selectAll(this, SLOT(selectAll()), this));
//    addAction("edit_deselect_all", KStandardAction::deselect(this, SLOT(deselectAll()), this));

    setTextMode(true);
}

ArtisticTextTool::~ArtisticTextTool()
{
    delete m_currentStrategy;
}

QTransform ArtisticTextTool::cursorTransform() const
{
    if (!m_currentShape) {
        return QTransform();
    }

    QTransform transform;

    const int textLength = m_currentShape->plainText().length();
    if (m_textCursor <= textLength) {
        const QPointF pos = m_currentShape->charPositionAt(m_textCursor);
        const qreal angle = m_currentShape->charAngleAt(m_textCursor);
        QFontMetrics metrics(m_currentShape->fontAt(m_textCursor));

        transform.translate(pos.x() - 1, pos.y());
        transform.rotate(360. - angle);
        transform.translate(0, metrics.descent());
    } else if (m_textCursor <= textLength + m_linefeedPositions.size()) {
        const QPointF pos = m_linefeedPositions.value(m_textCursor - textLength - 1);
        QFontMetrics metrics(m_currentShape->fontAt(textLength - 1));
        transform.translate(pos.x(), pos.y());
        transform.translate(0, metrics.descent());
    }

    return transform * m_currentShape->absoluteTransformation(0);
}

void ArtisticTextTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    if (! m_currentShape) {
        return;
    }

    if (m_showCursor && m_blinkingCursor.isActive() && !m_currentStrategy) {
        painter.save();
        m_currentShape->applyConversion(painter, converter);
        painter.setBrush(Qt::black);
        painter.setWorldTransform(cursorTransform(), true);
        painter.setClipping(false);
        painter.drawPath(m_textCursorShape);
        painter.restore();
    }
    m_showCursor = !m_showCursor;

    if (m_currentShape->isOnPath()) {
        painter.save();
        m_currentShape->applyConversion(painter, converter);
        if (!m_currentShape->baselineShape()) {
            painter.setPen(Qt::DotLine);
            painter.setBrush(Qt::NoBrush);
            painter.drawPath(m_currentShape->baseline());
        }
        painter.setPen(Qt::blue);
        painter.setBrush(m_hoverHandle ? Qt::red : Qt::white);
        painter.drawPath(offsetHandleShape());
        painter.restore();
    }
    if (m_selection.hasSelection()) {
        painter.save();
        m_selection.paint(painter, converter);
        painter.restore();
    }
}

void ArtisticTextTool::repaintDecorations()
{
    canvas()->updateCanvas(offsetHandleShape().boundingRect());
    if (m_currentShape && m_currentShape->isOnPath()) {
        if (!m_currentShape->baselineShape()) {
            canvas()->updateCanvas(m_currentShape->baseline().boundingRect());
        }
    }
    m_selection.repaintDecoration();
}

int ArtisticTextTool::cursorFromMousePosition(const QPointF &mousePosition)
{
    if (!m_currentShape) {
        return -1;
    }

    const QPointF pos = m_currentShape->documentToShape(mousePosition);
    const int len = m_currentShape->plainText().length();
    int hit = -1;
    qreal mindist = DBL_MAX;
    for (int i = 0; i <= len; ++i) {
        QPointF center = pos - m_currentShape->charPositionAt(i);
        if ((fabs(center.x()) + fabs(center.y())) < mindist) {
            hit = i;
            mindist = fabs(center.x()) + fabs(center.y());
        }
    }
    return hit;
}

void ArtisticTextTool::mousePressEvent(KoPointerEvent *event)
{
    if (m_hoverHandle) {
        m_currentStrategy = new MoveStartOffsetStrategy(this, m_currentShape);
    }
    if (m_hoverText) {
        KoSelection *selection = canvas()->selectedShapesProxy()->selection();
        if (m_hoverText != m_currentShape) {
            // if we hit another text shape, select that shape
            selection->deselectAll();
            setCurrentShape(m_hoverText);
            selection->select(m_currentShape);
        }
        // change the text cursor position
        int hitCursorPos = cursorFromMousePosition(event->point);
        if (hitCursorPos >= 0) {
            setTextCursorInternal(hitCursorPos);
            m_selection.clear();
        }
        m_currentStrategy = new SelectTextStrategy(this, m_textCursor);
    }
    event->ignore();
}

void ArtisticTextTool::mouseMoveEvent(KoPointerEvent *event)
{
    m_hoverPath = 0;
    m_hoverText = 0;

    if (m_currentStrategy) {
        m_currentStrategy->handleMouseMove(event->point, event->modifiers());
        return;
    }

    const bool textOnPath = m_currentShape && m_currentShape->isOnPath();
    if (textOnPath) {
        QPainterPath handle = offsetHandleShape();
        QPointF handleCenter = handle.boundingRect().center();
        if (handleGrabRect(event->point).contains(handleCenter)) {
            // mouse is on offset handle
            if (!m_hoverHandle) {
                canvas()->updateCanvas(handle.boundingRect());
            }
            m_hoverHandle = true;
        } else {
            if (m_hoverHandle) {
                canvas()->updateCanvas(handle.boundingRect());
            }
            m_hoverHandle = false;
        }
    }
    if (!m_hoverHandle) {
        // find text or path shape at cursor position
        QList<KoShape *> shapes = canvas()->shapeManager()->shapesAt(handleGrabRect(event->point));
        if (shapes.contains(m_currentShape)) {
            m_hoverText = m_currentShape;
        } else {
            Q_FOREACH (KoShape *shape, shapes) {
                ArtisticTextShape *text = dynamic_cast<ArtisticTextShape *>(shape);
                if (text && !m_hoverText) {
                    m_hoverText = text;
                }
                KoPathShape *path = dynamic_cast<KoPathShape *>(shape);
                if (path && !m_hoverPath) {
                    m_hoverPath = path;
                }
                if (m_hoverPath && m_hoverText) {
                    break;
                }
            }
        }
    }

    const bool hoverOnBaseline = textOnPath && m_currentShape && m_currentShape->baselineShape() == m_hoverPath;
    // update cursor and status text
    if (m_hoverText) {
        useCursor(QCursor(Qt::IBeamCursor));
        if (m_hoverText == m_currentShape) {
            emit statusTextChanged(i18n("Click to change cursor position."));
        } else {
            emit statusTextChanged(i18n("Click to select text shape."));
        }
    } else if (m_hoverPath && m_currentShape && !hoverOnBaseline) {
        useCursor(QCursor(Qt::PointingHandCursor));
        emit statusTextChanged(i18n("Double click to put text on path."));
    } else  if (m_hoverHandle) {
        useCursor(QCursor(Qt::ArrowCursor));
        emit statusTextChanged(i18n("Drag handle to change start offset."));
    } else {
        useCursor(QCursor(Qt::ArrowCursor));
        if (m_currentShape) {
            emit statusTextChanged(i18n("Press escape to finish editing."));
        } else {
            emit statusTextChanged(QString());
        }
    }
}

void ArtisticTextTool::mouseReleaseEvent(KoPointerEvent *event)
{
    if (m_currentStrategy) {
        m_currentStrategy->finishInteraction(event->modifiers());
        KUndo2Command *cmd = m_currentStrategy->createCommand();
        if (cmd) {
            canvas()->addCommand(cmd);
        }
        delete m_currentStrategy;
        m_currentStrategy = 0;
    }
    updateActions();
}

void ArtisticTextTool::shortcutOverrideEvent(QKeyEvent *event)
{
    QKeySequence item(event->key() | ((Qt::ControlModifier | Qt::AltModifier) & event->modifiers()));
    if (hit(item, KStandardShortcut::Begin) ||
            hit(item, KStandardShortcut::End)) {
        event->accept();
    }
}

void ArtisticTextTool::mouseDoubleClickEvent(KoPointerEvent */*event*/)
{
    if (m_hoverPath && m_currentShape) {
        if (!m_currentShape->isOnPath() || m_currentShape->baselineShape() != m_hoverPath) {
            m_blinkingCursor.stop();
            m_showCursor = false;
            updateTextCursorArea();
            canvas()->addCommand(new AttachTextToPathCommand(m_currentShape, m_hoverPath));
            m_blinkingCursor.start(BlinkInterval);
            updateActions();
            m_hoverPath = 0;
            m_linefeedPositions.clear();
            return;
        }
    }
}

void ArtisticTextTool::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        event->ignore();
        return;
    }

    event->accept();
    if (m_currentShape && textCursor() > -1) {
        switch (event->key()) {
        case Qt::Key_Delete:
            if (m_selection.hasSelection()) {
                removeFromTextCursor(m_selection.selectionStart(), m_selection.selectionCount());
            } else if (textCursor() >= 0 && textCursor() < m_currentShape->plainText().length()) {
                removeFromTextCursor(textCursor(), 1);
            }
            break;
        case Qt::Key_Backspace:
            if (m_selection.hasSelection()) {
                removeFromTextCursor(m_selection.selectionStart(), m_selection.selectionCount());
            } else {
                removeFromTextCursor(textCursor() - 1, 1);
            }
            break;
        case Qt::Key_Right:
            if (event->modifiers() & Qt::ShiftModifier) {
                int selectionStart, selectionEnd;
                if (m_selection.hasSelection()) {
                    selectionStart = m_selection.selectionStart();
                    selectionEnd = selectionStart + m_selection.selectionCount();
                    if (textCursor() == selectionStart) {
                        selectionStart = textCursor() + 1;
                    } else if (textCursor() == selectionEnd) {
                        selectionEnd = textCursor() + 1;
                    }
                } else {
                    selectionStart = textCursor();
                    selectionEnd = textCursor() + 1;
                }
                m_selection.selectText(selectionStart, selectionEnd);
            } else {
                m_selection.clear();
            }
            setTextCursor(m_currentShape, textCursor() + 1);
            break;
        case Qt::Key_Left:
            if (event->modifiers() & Qt::ShiftModifier) {
                int selectionStart, selectionEnd;
                if (m_selection.hasSelection()) {
                    selectionStart = m_selection.selectionStart();
                    selectionEnd = selectionStart + m_selection.selectionCount();
                    if (textCursor() == selectionStart) {
                        selectionStart = textCursor() - 1;
                    } else if (textCursor() == selectionEnd) {
                        selectionEnd = textCursor() - 1;
                    }
                } else {
                    selectionEnd = textCursor();
                    selectionStart = textCursor() - 1;
                }
                m_selection.selectText(selectionStart, selectionEnd);
            } else {
                m_selection.clear();
            }
            setTextCursor(m_currentShape, textCursor() - 1);
            break;
        case Qt::Key_Home:
            if (event->modifiers() & Qt::ShiftModifier) {
                const int selectionStart = 0;
                const int selectionEnd = m_selection.hasSelection() ? m_selection.selectionStart() + m_selection.selectionCount() : m_textCursor;
                m_selection.selectText(selectionStart, selectionEnd);
            } else {
                m_selection.clear();
            }
            setTextCursor(m_currentShape, 0);
            break;
        case Qt::Key_End:
            if (event->modifiers() & Qt::ShiftModifier) {
                const int selectionStart = m_selection.hasSelection() ? m_selection.selectionStart() : m_textCursor;
                const int selectionEnd = m_currentShape->plainText().length();
                m_selection.selectText(selectionStart, selectionEnd);
            } else {
                m_selection.clear();
            }
            setTextCursor(m_currentShape, m_currentShape->plainText().length());
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter: {
            const int textLength = m_currentShape->plainText().length();
            if (m_textCursor >= textLength) {
                // get font metrics for last character
                QFontMetrics metrics(m_currentShape->fontAt(textLength - 1));
                const qreal offset = m_currentShape->size().height() + (m_linefeedPositions.size() + 1) * metrics.height();
                m_linefeedPositions.append(QPointF(0, offset));
                setTextCursor(m_currentShape, textCursor() + 1);
            }
            break;
        }
        default:
            if (event->text().isEmpty()) {
                event->ignore();
                return;
            }
            if (m_selection.hasSelection()) {
                removeFromTextCursor(m_selection.selectionStart(), m_selection.selectionCount());
            }
            addToTextCursor(event->text());
        }
    } else {
        event->ignore();
    }
}

void ArtisticTextTool::activate(ToolActivation activation, const QSet<KoShape *> &shapes)
{
    KoToolBase::activate(activation, shapes);

    foreach (KoShape *shape, shapes) {
        ArtisticTextShape *text = dynamic_cast<ArtisticTextShape *>(shape);
        if (text) {
            setCurrentShape(text);
            break;
        }
    }
    if (!m_currentShape) {
        // none found
        emit done();
        return;
    }

    m_hoverText = 0;
    m_hoverPath = 0;

    updateActions();
    emit statusTextChanged(i18n("Press return to finish editing."));
    repaintDecorations();

    connect(canvas()->selectedShapesProxy(), SIGNAL(selectionChanged()), this, SLOT(shapeSelectionChanged()));
}

void ArtisticTextTool::blinkCursor()
{
    updateTextCursorArea();
}

void ArtisticTextTool::deactivate()
{
    if (m_currentShape) {
        if (m_currentShape->plainText().isEmpty()) {
            canvas()->addCommand(canvas()->shapeController()->removeShape(m_currentShape));
        }
        setCurrentShape(0);
    }
    m_hoverPath = 0;
    m_hoverText = 0;

    disconnect(canvas()->selectedShapesProxy(), SIGNAL(selectionChanged()), this, SLOT(shapeSelectionChanged()));

    KoToolBase::deactivate();
}

void ArtisticTextTool::updateActions()
{
    if (m_currentShape) {
        const QFont font = m_currentShape->fontAt(textCursor());
        const CharIndex index = m_currentShape->indexOfChar(textCursor());
        ArtisticTextRange::BaselineShift baselineShift = ArtisticTextRange::None;
        if (index.first >= 0) {
            baselineShift = m_currentShape->text().at(index.first).baselineShift();
        }
        m_fontBold->blockSignals(true);
        m_fontBold->setChecked(font.bold());
        m_fontBold->blockSignals(false);
        m_fontBold->setEnabled(true);
        m_fontItalic->blockSignals(true);
        m_fontItalic->setChecked(font.italic());
        m_fontItalic->blockSignals(false);
        m_fontItalic->setEnabled(true);
        m_detachPath->setEnabled(m_currentShape->isOnPath());
        m_convertText->setEnabled(true);
        m_anchorGroup->blockSignals(true);
        Q_FOREACH (QAction *action, m_anchorGroup->actions()) {
            if (action->data().toInt() == m_currentShape->textAnchor()) {
                action->setChecked(true);
            }
        }
        m_anchorGroup->blockSignals(false);
        m_anchorGroup->setEnabled(true);
        m_superScript->blockSignals(true);
        m_superScript->setChecked(baselineShift == ArtisticTextRange::Super);
        m_superScript->blockSignals(false);
        m_subScript->blockSignals(true);
        m_subScript->setChecked(baselineShift == ArtisticTextRange::Sub);
        m_subScript->blockSignals(false);
        m_superScript->setEnabled(true);
        m_subScript->setEnabled(true);
    } else {
        m_detachPath->setEnabled(false);
        m_convertText->setEnabled(false);
        m_fontBold->setEnabled(false);
        m_fontItalic->setEnabled(false);
        m_anchorGroup->setEnabled(false);
        m_superScript->setEnabled(false);
        m_subScript->setEnabled(false);
    }
}

void ArtisticTextTool::detachPath()
{
    if (m_currentShape && m_currentShape->isOnPath()) {
        canvas()->addCommand(new DetachTextFromPathCommand(m_currentShape));
        updateActions();
    }
}

void ArtisticTextTool::convertText()
{
    if (! m_currentShape) {
        return;
    }

    KoPathShape *path = KoPathShape::createShapeFromPainterPath(m_currentShape->outline());
    path->setZIndex(m_currentShape->zIndex());
    path->setStroke(m_currentShape->stroke());
    path->setBackground(m_currentShape->background());
    path->setTransformation(m_currentShape->transformation());
    path->setShapeId(KoPathShapeId);

    KUndo2Command *cmd = canvas()->shapeController()->addShapeDirect(path, 0);
    cmd->setText(kundo2_i18n("Convert to Path"));
    canvas()->shapeController()->removeShape(m_currentShape, cmd);
    canvas()->addCommand(cmd);

    emit done();
}

QList<QPointer<QWidget> > ArtisticTextTool::createOptionWidgets()
{
    QList<QPointer<QWidget> > widgets;

    ArtisticTextShapeConfigWidget *configWidget = new ArtisticTextShapeConfigWidget(this);
    configWidget->setObjectName("ArtisticTextConfigWidget");
    configWidget->setWindowTitle(i18n("Text Properties"));
    connect(configWidget, SIGNAL(fontFamilyChanged(QFont)), this, SLOT(setFontFamiliy(QFont)));
    connect(configWidget, SIGNAL(fontSizeChanged(int)), this, SLOT(setFontSize(int)));
    connect(this, SIGNAL(shapeSelected()), configWidget, SLOT(updateWidget()));
    connect(canvas()->selectedShapesProxy(), SIGNAL(selectionContentChanged()),
            configWidget, SLOT(updateWidget()));
    widgets.append(configWidget);

    ArtisticTextShapeOnPathWidget *pathWidget = new ArtisticTextShapeOnPathWidget(this);
    pathWidget->setObjectName("ArtisticTextPathWidget");
    pathWidget->setWindowTitle(i18n("Text On Path"));
    connect(pathWidget, SIGNAL(offsetChanged(int)), this, SLOT(setStartOffset(int)));
    connect(this, SIGNAL(shapeSelected()), pathWidget, SLOT(updateWidget()));
    connect(canvas()->selectedShapesProxy(), SIGNAL(selectionContentChanged()),
            pathWidget, SLOT(updateWidget()));
    widgets.append(pathWidget);

    if (m_currentShape) {
        pathWidget->updateWidget();
        configWidget->updateWidget();
    }

    return widgets;
}

KoToolSelection *ArtisticTextTool::selection()
{
    return &m_selection;
}

void ArtisticTextTool::enableTextCursor(bool enable)
{
    if (enable) {
        if (m_currentShape) {
            setTextCursorInternal(m_currentShape->plainText().length());
        }
        connect(&m_blinkingCursor, SIGNAL(timeout()), this, SLOT(blinkCursor()));
        m_blinkingCursor.start(BlinkInterval);
    } else {
        m_blinkingCursor.stop();
        disconnect(&m_blinkingCursor, SIGNAL(timeout()), this, SLOT(blinkCursor()));
        setTextCursorInternal(-1);
        m_showCursor = false;
    }
}

void ArtisticTextTool::setTextCursor(ArtisticTextShape *textShape, int textCursor)
{
    if (!m_currentShape || textShape != m_currentShape) {
        return;
    }
    if (m_textCursor == textCursor || textCursor < 0) {
        return;
    }
    const int textLength = m_currentShape->plainText().length() + m_linefeedPositions.size();
    if (textCursor > textLength) {
        return;
    }
    setTextCursorInternal(textCursor);
}

int ArtisticTextTool::textCursor() const
{
    return m_textCursor;
}

void ArtisticTextTool::updateTextCursorArea() const
{
    if (! m_currentShape || m_textCursor < 0) {
        return;
    }

    QRectF bbox = cursorTransform().mapRect(m_textCursorShape.boundingRect());
    canvas()->updateCanvas(bbox);
}

void ArtisticTextTool::setCurrentShape(ArtisticTextShape *currentShape)
{
    if (m_currentShape == currentShape) {
        return;
    }
    enableTextCursor(false);
    m_currentShape = currentShape;
    m_selection.setSelectedShape(m_currentShape);
    if (m_currentShape) {
        enableTextCursor(true);
    }
    emit shapeSelected();
}

void ArtisticTextTool::setTextCursorInternal(int textCursor)
{
    updateTextCursorArea();
    m_textCursor = textCursor;
    createTextCursorShape();
    updateTextCursorArea();
    updateActions();
    emit shapeSelected();
}

void ArtisticTextTool::createTextCursorShape()
{
    if (m_textCursor < 0 || ! m_currentShape) {
        return;
    }
    const QRectF extents = m_currentShape->charExtentsAt(m_textCursor);
    m_textCursorShape = QPainterPath();
    m_textCursorShape.addRect(0, 0, 1, -extents.height());
    m_textCursorShape.closeSubpath();
}

void ArtisticTextTool::removeFromTextCursor(int from, unsigned int count)
{
    if (from >= 0) {
        if (m_selection.hasSelection()) {
            // clear selection before text is removed, or else selection will be invalid
            m_selection.clear();
        }
        KUndo2Command *cmd = new RemoveTextRangeCommand(this, m_currentShape, from, count);
        canvas()->addCommand(cmd);
    }
}

void ArtisticTextTool::addToTextCursor(const QString &str)
{
    if (!str.isEmpty() && m_textCursor > -1) {
        QString printable;
        for (int i = 0; i < str.length(); i++) {
            if (str[i].isPrint()) {
                printable.append(str[i]);
            }
        }
        unsigned int len = printable.length();
        if (len) {
            const int textLength = m_currentShape->plainText().length();
            if (m_textCursor <= textLength) {
                KUndo2Command *cmd = new AddTextRangeCommand(this, m_currentShape, printable, m_textCursor);
                canvas()->addCommand(cmd);
            } else if (m_textCursor <= textLength + m_linefeedPositions.size()) {
                const QPointF pos = m_linefeedPositions.value(m_textCursor - textLength - 1);
                ArtisticTextRange newLine(printable, m_currentShape->fontAt(textLength - 1));
                newLine.setXOffsets(QList<qreal>() << pos.x(), ArtisticTextRange::AbsoluteOffset);
                newLine.setYOffsets(QList<qreal>() << pos.y() - m_currentShape->baselineOffset(), ArtisticTextRange::AbsoluteOffset);
                KUndo2Command *cmd = new AddTextRangeCommand(this, m_currentShape, newLine, m_textCursor);
                canvas()->addCommand(cmd);
                m_linefeedPositions.clear();
            }
        }
    }
}

void ArtisticTextTool::textChanged()
{
    if (!m_currentShape) {
        return;
    }

    const QString currentText = m_currentShape->plainText();
    if (m_textCursor > currentText.length()) {
        setTextCursorInternal(currentText.length());
    }
}

void ArtisticTextTool::shapeSelectionChanged()
{
    KoSelection *selection = canvas()->selectedShapesProxy()->selection();
    if (selection->isSelected(m_currentShape)) {
        return;
    }

    foreach (KoShape *shape, selection->selectedShapes()) {
        ArtisticTextShape *text = dynamic_cast<ArtisticTextShape *>(shape);
        if (text) {
            setCurrentShape(text);
            break;
        }
    }
}

QPainterPath ArtisticTextTool::offsetHandleShape()
{
    QPainterPath handle;
    if (!m_currentShape || !m_currentShape->isOnPath()) {
        return handle;
    }

    const QPainterPath baseline = m_currentShape->baseline();
    const qreal offset = m_currentShape->startOffset();
    QPointF offsetPoint = baseline.pointAtPercent(offset);
    QSizeF paintSize = handlePaintRect(QPointF()).size();

    handle.moveTo(0, 0);
    handle.lineTo(0.5 * paintSize.width(), paintSize.height());
    handle.lineTo(-0.5 * paintSize.width(), paintSize.height());
    handle.closeSubpath();

    QTransform transform;
    transform.translate(offsetPoint.x(), offsetPoint.y());
    transform.rotate(360. - baseline.angleAtPercent(offset));

    return transform.map(handle);
}

void ArtisticTextTool::setStartOffset(int offset)
{
    if (!m_currentShape || !m_currentShape->isOnPath()) {
        return;
    }

    const qreal newOffset = static_cast<qreal>(offset) / 100.0;
    if (newOffset != m_currentShape->startOffset()) {
        canvas()->addCommand(new ChangeTextOffsetCommand(m_currentShape, m_currentShape->startOffset(), newOffset));
    }
}

void ArtisticTextTool::changeFontProperty(FontProperty property, const QVariant &value)
{
    if (!m_currentShape || !m_selection.hasSelection()) {
        return;
    }

    // build font ranges
    const int selectedCharCount = m_selection.selectionCount();
    const int selectedCharStart = m_selection.selectionStart();
    QList<ArtisticTextRange> ranges = m_currentShape->text();
    CharIndex index = m_currentShape->indexOfChar(selectedCharStart);
    if (index.first < 0) {
        return;
    }

    KUndo2Command *cmd = new KUndo2Command;
    int collectedCharCount = 0;
    while (collectedCharCount < selectedCharCount) {
        ArtisticTextRange &range = ranges[index.first];
        QFont font = range.font();
        switch (property) {
        case BoldProperty:
            font.setBold(value.toBool());
            break;
        case ItalicProperty:
            font.setItalic(value.toBool());
            break;
        case FamiliyProperty:
            font.setFamily(value.toString());
            break;
        case SizeProperty:
            font.setPointSize(value.toInt());
            break;
        }

        const int changeCount = qMin(selectedCharCount - collectedCharCount, range.text().count() - index.second);
        const int changeStart = selectedCharStart + collectedCharCount;
        new ChangeTextFontCommand(m_currentShape, changeStart, changeCount, font, cmd);
        index.first++;
        index.second = 0;
        collectedCharCount += changeCount;
    }

    canvas()->addCommand(cmd);
}

void ArtisticTextTool::toggleFontBold(bool enabled)
{
    changeFontProperty(BoldProperty, QVariant(enabled));
}

void ArtisticTextTool::toggleFontItalic(bool enabled)
{
    changeFontProperty(ItalicProperty, QVariant(enabled));
}

void ArtisticTextTool::anchorChanged(QAction *action)
{
    if (!m_currentShape) {
        return;
    }

    ArtisticTextShape::TextAnchor newAnchor = static_cast<ArtisticTextShape::TextAnchor>(action->data().toInt());
    if (newAnchor != m_currentShape->textAnchor()) {
        canvas()->addCommand(new ChangeTextAnchorCommand(m_currentShape, newAnchor));
    }
}

void ArtisticTextTool::setFontFamiliy(const QFont &font)
{
    changeFontProperty(FamiliyProperty, QVariant(font.family()));
}

void ArtisticTextTool::setFontSize(int size)
{
    changeFontProperty(SizeProperty, QVariant(size));
}

void ArtisticTextTool::setSuperScript()
{
    toggleSubSuperScript(ArtisticTextRange::Super);
}

void ArtisticTextTool::setSubScript()
{
    toggleSubSuperScript(ArtisticTextRange::Sub);
}

void ArtisticTextTool::toggleSubSuperScript(ArtisticTextRange::BaselineShift mode)
{
    if (!m_currentShape || !m_selection.hasSelection()) {
        return;
    }

    const int from = m_selection.selectionStart();
    const int count = m_selection.selectionCount();

    QList<ArtisticTextRange> ranges = m_currentShape->copyText(from, count);
    const int rangeCount = ranges.count();
    if (!rangeCount) {
        return;
    }

    // determine if we want to disable the specified mode
    const bool disableMode = ranges.first().baselineShift() == mode;
    const qreal fontSize = m_currentShape->defaultFont().pointSizeF();

    for (int i = 0; i < rangeCount; ++i) {
        ArtisticTextRange &currentRange = ranges[i];
        QFont font = currentRange.font();
        if (disableMode) {
            currentRange.setBaselineShift(ArtisticTextRange::None);
            font.setPointSizeF(fontSize);
        } else {
            currentRange.setBaselineShift(mode);
            font.setPointSizeF(fontSize * ArtisticTextRange::subAndSuperScriptSizeFactor());
        }
        currentRange.setFont(font);
    }
    canvas()->addCommand(new ReplaceTextRangeCommand(m_currentShape, ranges, from, count, this));
}

void ArtisticTextTool::selectAll()
{
    if (m_currentShape) {
        m_selection.selectText(0, m_currentShape->plainText().count());
    }
}

void ArtisticTextTool::deselectAll()
{
    if (m_currentShape) {
        m_selection.clear();
    }
}

QVariant ArtisticTextTool::inputMethodQuery(Qt::InputMethodQuery query, const KoViewConverter &converter) const
{
    if (!m_currentShape) {
        return QVariant();
    }

    switch (query) {
    case Qt::ImMicroFocus: {
        // The rectangle covering the area of the input cursor in widget coordinates.
        QRectF rect = m_textCursorShape.boundingRect();
        rect.moveTop(rect.bottom());
        QTransform shapeMatrix = m_currentShape->absoluteTransformation(&converter);
        qreal zoomX, zoomY;
        converter.zoom(&zoomX, &zoomY);
        shapeMatrix.scale(zoomX, zoomY);
        rect = shapeMatrix.mapRect(rect);
        return rect.toRect();
    }
    case Qt::ImFont:
        // The currently used font for text input.
        return m_currentShape->fontAt(m_textCursor);
    case Qt::ImCursorPosition:
        // The logical position of the cursor within the text surrounding the input area (see ImSurroundingText).
        return m_currentShape->charPositionAt(m_textCursor);
    case Qt::ImSurroundingText:
        // The plain text around the input area, for example the current paragraph.
        return m_currentShape->plainText();
    case Qt::ImCurrentSelection:
        // The currently selected text.
        return QVariant();
    default:
        ; // Qt 4.6 adds ImMaximumTextLength and ImAnchorPosition
    }
    return QVariant();
}
