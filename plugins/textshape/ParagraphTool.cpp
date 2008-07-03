/* This file is part of the KDE project
 * Copyright (C) 2008 Florian Merz <florianmerz@web.de>
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

#include "ParagraphTool.h"
#include "dialogs/ParagraphSettingsDialog.h"

#include <KoCanvasBase.h>
#include <KoParagraphStyle.h>
#include <KoPointerEvent.h>
#include <KoShapeBorderModel.h>
#include <KoShapeManager.h>
#include <KoStyleManager.h>
#include <KoTextBlockData.h>
#include <KoTextDocumentLayout.h>
#include <KoUnit.h>

#include <KDebug>
#include <KLocalizedString>

#include <QAbstractTextDocumentLayout>
#include <QAction>
#include <QCheckBox>
#include <QLabel>
#include <QPen>
#include <QString>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextLine>
#include <QTextList>
#include <QGridLayout>

ParagraphTool::ParagraphTool(KoCanvasBase *canvas)
    : KoTool(canvas),
    m_textShape(NULL),
    m_paragraphStyle(NULL),
    m_textBlockValid(false),
    m_needsRepaint(false),
    m_smoothMovement(false),
    m_firstIndentRuler(this),
    m_followingIndentRuler(this),
    m_rightMarginRuler(this),
    m_topMarginRuler(this),
    m_bottomMarginRuler(this),
    m_activeRuler(NULL),
    m_hoverRuler(NULL)
{
    initializeRuler(&m_firstIndentRuler);
    initializeRuler(&m_followingIndentRuler);
    initializeRuler(&m_rightMarginRuler);
    initializeRuler(&m_topMarginRuler, Ruler::drawSides);
    initializeRuler(&m_bottomMarginRuler, Ruler::drawSides);

    QAction *action = new QAction(i18n("Apply parent style to ruler"), this);
    action->setShortcut(Qt::ALT + Qt::CTRL + Qt::Key_P);
    addAction("apply_parent_style_to_active_ruler", action);
    connect(action, SIGNAL(triggered()), this, SLOT(applyParentStyleToActiveRuler()));
}

ParagraphTool::~ParagraphTool()
{}

// helper function to initalize rulers
void ParagraphTool::initializeRuler(Ruler *ruler, int options)
{
    ruler->setOptions(options);
    ruler->setUnit(canvas()->unit());
    ruler->setMinimumValue(0.0);
    connect(ruler, SIGNAL(needsRepaint()), this, SLOT(scheduleRepaint()));
    connect(ruler, SIGNAL(valueChanged(qreal)), this, SLOT(updateLayout()));
}

QWidget *ParagraphTool::createOptionWidget()
{
    // TODO: move this to a ui file, can't do right now, because Qt's designer is broken on my system
    QWidget *widget = new QWidget();
    QGridLayout *layout = new QGridLayout;

    QLabel *styleNameLabel = new QLabel(i18n("Paragraph Style:"));
    layout->addWidget(styleNameLabel, 0, 0);

    QLabel *styleName = new QLabel(i18n("n/a"));
    layout->addWidget(styleName, 0, 1);
    connect(this, SIGNAL(styleNameChanged(const QString&)), styleName, SLOT(setText(const QString &)));

    QCheckBox *applyToParent = new QCheckBox(i18n("Apply to all of paragraphs of this style"));
    applyToParent->setDisabled(true);
    layout->addWidget(applyToParent, 2, 0, 1, -1);

    QCheckBox *smoothCheckBox = new QCheckBox(i18n("Enable Smooth Movement"));
    layout->addWidget(smoothCheckBox, 1, 0, 1, -1);
    connect(this, SIGNAL(smoothMovementChanged(bool)), smoothCheckBox, SLOT(setChecked(bool)));
    connect(smoothCheckBox, SIGNAL(clicked(bool)), this, SLOT(setSmoothMovement(bool)));

    layout->setRowStretch(3, 1);
    widget->setLayout(layout);
    return widget;
}

void ParagraphTool::loadDimensions()
{
    m_singleLine = (textLayout()->lineCount() == 1);

    // border rectangle left and right
    m_border.setLeft(0.0);
    m_border.setRight(textShape()->size().width());

    // first line rectangle
    m_firstLine = textLayout()->lineAt(0).rect();

    // counter rectangle 
    KoTextBlockData *blockData = static_cast<KoTextBlockData*> (textBlock().userData());
    if (blockData != NULL) {
        m_counter = QRectF(blockData->counterPosition(), QSizeF(blockData->counterWidth() - blockData->counterSpacing(),
                    m_firstLine.height()));
        m_isList = true;
    }
    else {
        m_isList = false;
    }

    // folowing lines rectangle
    if (!m_singleLine) {
        m_followingLines = QRectF(textLayout()->lineAt(1).rect().topLeft(),
                textLayout()->lineAt(textLayout()->lineCount() - 1).rect().bottomRight());
    }
    else {
        m_followingLines = m_firstLine;
    }

    // border rectangle top and bottom
    m_border.setTop(m_firstLine.top() - m_paragraphStyle->topMargin());
    m_border.setBottom(m_singleLine ? m_firstLine.bottom() + m_paragraphStyle->bottomMargin()
            : m_followingLines.bottom() + m_paragraphStyle->bottomMargin());

    // workaround: the lines overlap slightly so right now we simply calculate the mean of the two y-values
    if (!m_singleLine) {
        qreal lineBreak((m_firstLine.bottom() + m_followingLines.top()) / 2.0);
        m_firstLine.setBottom(lineBreak);
        m_counter.setBottom(lineBreak);
        m_followingLines.setTop(lineBreak);
    }
}

void ParagraphTool::loadRulers()
{
    m_rightMarginRuler.setBaseline(QPointF(m_border.right(), m_followingLines.bottom()),
            QPointF(m_border.right(), m_firstLine.top()));
    m_rightMarginRuler.setValue(m_paragraphStyle->rightMargin());

    m_topMarginRuler.setBaseline(QPointF(m_border.right(), m_border.top()),
                QPointF(m_border.left(), m_border.top()));
    m_topMarginRuler.setValue(m_paragraphStyle->topMargin());

    m_bottomMarginRuler.setBaseline(QPointF(m_border.right(), m_followingLines.bottom()), 
                QPointF(m_border.left(), m_followingLines.bottom()));
    m_bottomMarginRuler.setValue(m_paragraphStyle->bottomMargin());

    if (m_singleLine) {
        m_firstIndentRuler.setBaseline(QPointF(m_border.left(), m_firstLine.top()), 
                QPointF(m_border.left(), m_firstLine.bottom()));
        m_firstIndentRuler.setValue(m_paragraphStyle->leftMargin() + m_paragraphStyle->textIndent());

        m_followingIndentRuler.hide();
    }
    else {
        m_firstIndentRuler.setBaseline(QPointF(m_border.left(), m_firstLine.top()),
                   QPointF(m_border.left(), m_firstLine.bottom()));
        m_firstIndentRuler.setValue(m_paragraphStyle->leftMargin() + m_paragraphStyle->textIndent());

        m_followingIndentRuler.setBaseline(QPointF(m_border.left(), m_followingLines.top()),
                    QPointF(m_border.left(), m_followingLines.bottom()));
        m_followingIndentRuler.setValue(m_paragraphStyle->leftMargin());
        m_followingIndentRuler.show();
    }
}


void ParagraphTool::saveRulers()
{
    // TODO: split this into separate functions for each property, that way we can apply properties to parent styles
    // how do we handle following line indent if we want to apply it to the parent ruler? we always need to apply both
    // at the same time. maybe it's best to return followingIndent to the way the backend does it
    Q_ASSERT(m_paragraphStyle != NULL);

    m_paragraphStyle->setLeftMargin(m_followingIndentRuler.value());
    m_paragraphStyle->setRightMargin(m_rightMarginRuler.value());
    m_paragraphStyle->setTopMargin(m_topMarginRuler.value());
    m_paragraphStyle->setBottomMargin(m_bottomMarginRuler.value());
    m_paragraphStyle->setTextIndent(m_firstIndentRuler.value() - m_followingIndentRuler.value());

    QTextBlockFormat format;
    m_paragraphStyle->applyStyle(format);

    QTextCursor cursor(textBlock());
    cursor.mergeBlockFormat(format);

    m_block = cursor.block();
}

QString ParagraphTool::styleName()
{
    KoParagraphStyle *style = m_paragraphStyle;
    while (style != NULL) {
        QString name = style->name();
        if (!name.isNull() && !name.isEmpty()) {
            return name;
        }
        style = style->parent();
    }

    return QString(i18n("None"));
}

void ParagraphTool::updateLayout()
{
    saveRulers();

    if (!shapeContainsBlock()) {
        // for now, if the block moved to a different shape, we deselect the block
        // in the future we should switch to the new shape that contains the text block
        kDebug() << "Block moved to different shape: deslecting block";
        deselectTextBlock();
        repaintDecorations();
        return;
    }

    static_cast<KoTextDocumentLayout*>(document()->documentLayout())->layout();
    loadDimensions();
    loadRulers();

    repaintDecorations();
}

void ParagraphTool::paintLabel(QPainter &painter, const QMatrix &matrix, const Ruler *ruler) const
{
    QColor foregroundColor(ruler == m_activeRuler ? ruler->activeColor() : ruler->highlightColor());
    QString text(ruler->valueString());
    QLineF connector(matrix.map(ruler->labelConnector()));
    connector.setLength(10.0);

    painter.save();

    painter.resetMatrix();

    QColor foreground(foregroundColor);
    painter.setBrush(Qt::white);
    painter.setPen(foreground);

    QRectF label(connector.p2().x(), connector.p2().y(), 0.0, 0.0);
    label = painter.boundingRect(label, Qt::AlignCenter | Qt::AlignVCenter, text);
    label.adjust(-4.0, 0.0, 4.0, 0.0);

    // adjust label so that the connector ends on the edge of the label
    if (abs(connector.dx()) > abs(connector.dy())) {
        qreal halfWidth = connector.dy() < 0.0 ? label.width() / 2.0 : -label.width() / 2.0;
        label.adjust(halfWidth, 0.0, halfWidth, 0.0);
    }
    else {
        qreal halfHeight = connector.dy() >= 0.0 ? label.height() / 2.0 : -label.height() / 2.0;
        label.adjust(0.0, halfHeight, 0.0, halfHeight);
    }

    painter.drawLine(connector);
    painter.drawRoundRect(label, 720.0 / label.width(), 720.0 /  label.height());
    painter.drawText(label, Qt::AlignHCenter | Qt::AlignVCenter, text);
    painter.restore();
}

void ParagraphTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    if (!m_textBlockValid)
        return;

    painter.save();

    // transform painter from view coordinate system to text block coordinate system
    painter.setMatrix(textShape()->absoluteTransformation(&converter) * painter.matrix());
    KoShape::applyConversion(painter, converter);
    painter.translate(0.0, -shapeStartOffset());

    painter.setPen(Qt::darkGray);

    foreach (Ruler *ruler, findChildren<Ruler *>()) {
        if (ruler->isVisible())
            ruler->paint(painter);
    }

    // paint line between first line and following lines
    if (!m_singleLine) {
        painter.drawLine(m_border.left(), m_firstLine.bottom(), m_firstLine.right(), m_firstLine.bottom());
    }

    QMatrix matrix(painter.combinedMatrix());

    painter.restore();

    Ruler *labeledRuler = m_activeRuler != NULL ? m_activeRuler : m_hoverRuler;
    if (labeledRuler != NULL) {
        paintLabel(painter, matrix, labeledRuler);
    }
}

void ParagraphTool::repaintDecorations()
{
    if (!m_needsRepaint || m_textShape == NULL)
        return;

    QRectF boundingRect( QPointF(0, 0), textShape()->size() );
    if(textShape()->border()) {
        KoInsets insets;
        textShape()->border()->borderInsets(textShape(), insets);
        boundingRect.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
    }

    // adjust for arrow heads and label (although we can't be sure about the label)
    boundingRect.adjust(-50.0, -50.0, 50.0, 50.0);

    boundingRect = textShape()->absoluteTransformation(0).mapRect(boundingRect);

    // TODO: should add previous and current label position to the repaint area, too.
 
    canvas()->updateCanvas(boundingRect);

    m_needsRepaint = false;
}

void ParagraphTool::scheduleRepaint()
{
    m_needsRepaint = true;
}

qreal ParagraphTool::shapeStartOffset() const
{
    KoTextShapeData *textShapeData = static_cast<KoTextShapeData*> (textShape()->userData());

    return textShapeData->documentOffset();
}

qreal ParagraphTool::shapeEndOffset() const
{
    KoTextShapeData *textShapeData = static_cast<KoTextShapeData*> (textShape()->userData());

    return textShapeData->documentOffset() + textShape()->size().height();
}

bool ParagraphTool::shapeContainsBlock()
{
    QTextLayout *layout = textLayout();
    qreal blockStart = layout->lineAt(0).y();

    QTextLine endLine = layout->lineAt(layout->lineCount()-1);
    qreal blockEnd = endLine.y() + endLine.height();

    qreal shapeStart = shapeStartOffset();
    qreal shapeEnd = shapeEndOffset();

    if (blockEnd < shapeStart || blockStart > shapeEnd) {
        return false;
    }
    else {
        return true;
    }
}

QPointF ParagraphTool::mapDocumentToTextBlock(QPointF point) const
{
    QMatrix matrix = textShape()->absoluteTransformation(NULL);
    matrix.translate(0.0, -shapeStartOffset());
    return matrix.inverted().map(point);
}

void ParagraphTool::selectTextBlock(TextShape *newTextShape, QTextBlock block)
{
    // the text block is already selected, no need for a repaint and all that
    if (m_textBlockValid && block == m_block && newTextShape == textShape())
        return;

    m_textBlockValid = true;

    m_textShape = newTextShape;
    m_block = block;
    m_paragraphStyle = KoParagraphStyle::fromBlock(textBlock());

    emit styleNameChanged(styleName());

    scheduleRepaint();

    loadDimensions();
    loadRulers();
}

void ParagraphTool::deselectTextBlock()
{
    if (!m_textBlockValid)
        return;

    emit styleNameChanged(QString(i18n("n/a")));

    m_textBlockValid = false;
    scheduleRepaint();
}

void ParagraphTool::activateRuler(Ruler *ruler)
{
    m_activeRuler = ruler;
    m_activeRuler->setActive(true);

    // disable hovering if we have an active ruler
    // hovering over the wrong ruler confuses the user
    if (m_hoverRuler != NULL) {
        m_hoverRuler->setHighlighted(false);
        m_hoverRuler = NULL;
    }
}

void ParagraphTool::deactivateActiveRuler()
{
    if (m_activeRuler == NULL) {
        return;
    }

    m_activeRuler->setActive(false);
    m_activeRuler = NULL;

    // there's no active ruler anymore, so we have to if we hover somewhere
    QPointF point(mapDocumentToTextBlock(m_mousePosition));
    foreach (Ruler *ruler, findChildren<Ruler *>()) {
        if (ruler->hitTest(point)) {
            m_hoverRuler = ruler;
            m_hoverRuler->setHighlighted(true);
            break;
        }
    }
}

void ParagraphTool::resetActiveRuler()
{
    if (m_activeRuler != NULL) {
        m_activeRuler->reset();
        deactivateActiveRuler();
    }
}

void ParagraphTool::applyParentStyleToActiveRuler()
{
    if (m_activeRuler == NULL) {
        return;
    }

    if (m_activeRuler == &m_firstIndentRuler) {
        m_paragraphStyle->remove(QTextFormat::TextIndent);
        m_activeRuler->setValue(m_paragraphStyle->textIndent());
    }
    else if (m_activeRuler == &m_followingIndentRuler) {
        m_paragraphStyle->remove(QTextFormat::BlockLeftMargin);
        m_activeRuler->setValue(m_paragraphStyle->leftMargin());
    }
    else if (m_activeRuler == &m_rightMarginRuler) {
        m_paragraphStyle->remove(QTextFormat::BlockRightMargin);
        m_activeRuler->setValue(m_paragraphStyle->rightMargin());
    }
    else if (m_activeRuler == &m_topMarginRuler) {
        m_paragraphStyle->remove(QTextFormat::BlockTopMargin);
        m_activeRuler->setValue(m_paragraphStyle->topMargin());
    }
    else if (m_activeRuler == &m_bottomMarginRuler) {
        m_paragraphStyle->remove(QTextFormat::BlockBottomMargin);
        m_activeRuler->setValue(m_paragraphStyle->bottomMargin());
    }

    deactivateActiveRuler();

    // we need to call the updateLayout() slot manually, it is not emitted if
    // the ruler has been changed by calling setValue()
    // TODO: maybe setValue() should emit valueChanged(), too
    // but make sure that this doesn't trigger additional repaints or relayouts
    updateLayout();
}

void ParagraphTool::mousePressEvent(KoPointerEvent *event)
{
    m_mousePosition = event->point;

    if (event->button() == Qt::LeftButton) {

        // we check if the mouse pointer pressed on one of the current textblock's rulers
        if (m_textBlockValid) {

            QPointF point(mapDocumentToTextBlock(event->point));

            foreach (Ruler *ruler, findChildren<Ruler *>()) {
                if (ruler->hitTest(point)) {
                    activateRuler(ruler);

                    repaintDecorations();
                    return;
                }
            }

            m_activeRuler = NULL;
        }

        // if there is a new text block (possibly in another shape) under the mouse, then we select that block
        TextShape *textShape = dynamic_cast<TextShape*> (canvas()->shapeManager()->shapeAt(event->point));
        if (textShape) {
            KoTextShapeData *textShapeData = static_cast<KoTextShapeData*> (textShape->userData());
            QTextDocument *document = textShapeData->document();

            int position = document->documentLayout()->hitTest(textShape->convertScreenPos(event->point), Qt::ExactHit);
            if (position != -1) {
                selectTextBlock(textShape, document->findBlock(position));
                repaintDecorations();
                return;
            }
        }

        // if there is no ruler and no text block under the mouse, then we simply deselect the current text block
        deselectTextBlock();
    }
    else if (event->button() == Qt::RightButton) {
        resetActiveRuler();
    }
    else if(event->button() == Qt::MidButton) {
        applyParentStyleToActiveRuler();
    }

    repaintDecorations();
}

void ParagraphTool::mouseReleaseEvent(KoPointerEvent *event)
{
    m_mousePosition = event->point;

    if (m_textBlockValid && m_activeRuler != NULL) {
        deactivateActiveRuler();
    }

    repaintDecorations();
}

void ParagraphTool::mouseMoveEvent(KoPointerEvent *event)
{
    // map to the same coordinate system as the paint process
    m_mousePosition = event->point;

    if (m_textBlockValid) {
        QPointF point(mapDocumentToTextBlock(event->point));

        // send a mouseMoveEvent to the activeRuler
        // do this first so the active control can resize if necessary
        if (m_activeRuler != NULL) {
            m_activeRuler->moveRuler(point, smoothMovement());
        }
        // we don't want to hover if we have an active ruler
        else {
            // check if we left the element over which we were hovering
            if (m_hoverRuler != NULL && !m_hoverRuler->hitTest(point)) {
                m_hoverRuler->setHighlighted(false);
                m_hoverRuler = NULL;
            }

            // check if we are hovering over a new control
            if (m_hoverRuler == NULL) {
                foreach (Ruler *ruler, findChildren<Ruler *>()) {
                    if (ruler->hitTest(point)) {
                        m_hoverRuler = ruler;
                        m_hoverRuler->setHighlighted(true);
                        break;
                    }
                }
            }
        }
    }

    repaintDecorations();
}

void ParagraphTool::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
        case Qt::Key_Shift:
            toggleSmoothMovement();
            break;
        case Qt::Key_Escape:
            resetActiveRuler();
            break;
        case Qt::Key_Delete:
        case Qt::Key_BackSpace:
            applyParentStyleToActiveRuler();
            break;
        default:
            break;
    }

    repaintDecorations();
}

void ParagraphTool::keyReleaseEvent( QKeyEvent *event)
{
    if (event->key() == Qt::Key_Shift)
        toggleSmoothMovement();
}

void  ParagraphTool::activate( bool )
{
    // don't know why force=true is needed and what it does, but almost everyone else uses it...
    useCursor(Qt::ArrowCursor, true);
}

void ParagraphTool::deactivate()
{
    // the document might have changed, so we have to deselect the text block
    deselectTextBlock();
}

