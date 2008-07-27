/* This file is part of the KDE project
 * Copyright (C) 2008 Florian Merz <florianmerz@gmx.de>
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

#include "TextShape.h"
#include "dialogs/ParagraphSettingsDialog.h"

#include <KoCanvasBase.h>
#include <KoParagraphStyle.h>
#include <KoPointerEvent.h>
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
#include <QColor>
#include <QGridLayout>
#include <QLabel>
#include <QPen>
#include <QString>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextLine>
#include <QTextList>
#include <QVectorIterator>

/* FIXME:
 * - make sure that rulers only draw into the right area, pass a bounding rectangle to the drawing method for this
 *
 * TODO:
 * - add accessibility support
 * - remove hard-coded colors (use KDE theme or adjust to document/shape background?)
 * - add RTL support
 * - add undo support
 * - add linespacing support
 * - add more feature via the options docker
 * - add proper relayouting (when relayouting is done by the tool it doesn't flow around other shapes)
 * - think about a method to give instructions to new users (the bubble used by okular might ne a good way to do this)
 */
bool shapeContainsBlock(const TextShape *textShape, QTextBlock textBlock)
{
    QTextLayout *layout = textBlock.layout();
    qreal blockStart = layout->lineAt(0).y();

    QTextLine endLine = layout->lineAt(layout->lineCount()-1);
    qreal blockEnd = endLine.y() + endLine.height();

    KoTextShapeData *textShapeData = static_cast<KoTextShapeData*> (textShape->userData());
    qreal shapeStart = textShapeData->documentOffset();
    qreal shapeEnd = shapeStart + textShape->size().height();

    return (blockEnd >= shapeStart && blockStart < shapeEnd);
}

ParagraphTool::ParagraphTool(KoCanvasBase *canvas)
    : KoTool(canvas),
    m_paragraphStyle(NULL),
    m_activeRuler(noRuler),
    m_highlightRuler(noRuler),
    m_textBlockValid(false),
    m_needsRepaint(false),
    m_smoothMovement(false)
{
    initializeRuler(m_rulers[firstIndentRuler]);
    initializeRuler(m_rulers[followingIndentRuler]);
    initializeRuler(m_rulers[rightMarginRuler]);
    initializeRuler(m_rulers[topMarginRuler], Ruler::drawSides);
    initializeRuler(m_rulers[bottomMarginRuler], Ruler::drawSides);

    QAction *action = new QAction(i18n("Apply parent style to ruler"), this);
    action->setShortcut(Qt::ALT + Qt::CTRL + Qt::Key_P);
    addAction("apply_parent_style_to_active_ruler", action);
    connect(action, SIGNAL(triggered()), this, SLOT(applyParentStyleToActiveRuler()));
}

ParagraphTool::~ParagraphTool()
{}

// helper function to initalize rulers
void ParagraphTool::initializeRuler(Ruler &ruler, int options)
{
    ruler.setParent(this);
    ruler.setOptions(options);
    ruler.setUnit(canvas()->unit());
    ruler.setMinimumValue(0.0);
    connect(&ruler, SIGNAL(needsRepaint()), this, SLOT(scheduleRepaint()));
    connect(&ruler, SIGNAL(valueChanged(qreal)), this, SLOT(updateLayout()));
}

void ParagraphTool::initParagraphProperties()
{
    m_isSingleLine = (textBlock().layout()->lineCount() == 1);

    KoTextBlockData *blockData = static_cast<KoTextBlockData*> (textBlock().userData());
    m_isList = (blockData != NULL);
}

bool ParagraphTool::createShapeList()
{
    m_shapes.clear();

    KoTextDocumentLayout *layout = static_cast<KoTextDocumentLayout*>(textBlock().document()->documentLayout());

    QList<KoShape*> shapes = layout->shapes();
    foreach (KoShape *shape, shapes) {
        if (shapeContainsBlock(static_cast<TextShape*>(shape), textBlock())) {
            m_shapes << ShapeSpecificData(m_rulers, static_cast<TextShape*>(shape), textBlock(), m_paragraphStyle);
        }
    }

    return true;
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

void ParagraphTool::loadRulers()
{
    m_rulers[firstIndentRuler].setValue(m_paragraphStyle->leftMargin() + m_paragraphStyle->textIndent());
    m_rulers[followingIndentRuler].setValue(m_paragraphStyle->leftMargin());
    m_rulers[rightMarginRuler].setValue(m_paragraphStyle->rightMargin());
    m_rulers[topMarginRuler].setValue(m_paragraphStyle->topMargin());
    m_rulers[bottomMarginRuler].setValue(m_paragraphStyle->bottomMargin());

    m_rulers[followingIndentRuler].setVisible(!m_isSingleLine);

    scheduleRepaint();
}


void ParagraphTool::saveRulers()
{
    m_paragraphStyle->setLeftMargin(m_rulers[followingIndentRuler].value());
    m_paragraphStyle->setRightMargin(m_rulers[rightMarginRuler].value());
    m_paragraphStyle->setTopMargin(m_rulers[topMarginRuler].value());
    m_paragraphStyle->setBottomMargin(m_rulers[bottomMarginRuler].value());
    m_paragraphStyle->setTextIndent(m_rulers[firstIndentRuler].value() - m_rulers[followingIndentRuler].value());

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

/* slot which is called when the value of one of the rulers changed
 * causes storing and reloading of the positions and values of the rulers from the file */
void ParagraphTool::updateLayout()
{
    saveRulers();

    static_cast<KoTextDocumentLayout*>(textBlock().document()->documentLayout())->layout();

    initParagraphProperties();
    if (createShapeList()) {
        loadRulers();
    }
    else {
        deselectTextBlock();
    }

    repaintDecorations();
}

void ParagraphTool::paintLabel(QPainter &painter, const KoViewConverter &converter) const
{
    RulerIndex ruler;
    ShapeSpecificData *shape;
    QColor foregroundColor;

    if (hasActiveRuler()) {
        ruler = m_activeRuler;
        shape = m_activeShape;
        foregroundColor = m_rulers[ruler].activeColor();
    }
    else if (hasHighlightedRuler()) {
        ruler = m_highlightRuler;
        shape = m_highlightShape;
        foregroundColor = m_rulers[ruler].highlightColor();
    }
    else {
        return;
    }

    painter.save();

    // transform painter from view coordinate system to shape coordinate system
    painter.setMatrix(shape->textShape()->absoluteTransformation(&converter) * painter.matrix());
    KoShape::applyConversion(painter, converter);
    painter.translate(0.0, -shape->shapeStartOffset());

    QMatrix matrix(painter.combinedMatrix());

    QString text(m_rulers[ruler].valueString());
    QLineF connector(matrix.map(m_rulers[ruler].labelConnector(shape->baseline(ruler))));
    connector.setLength(10.0);

    painter.resetMatrix();

    painter.setBrush(Qt::white);
    painter.setPen(foregroundColor);

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

    foreach (ShapeSpecificData shape, m_shapes) {
        shape.paint(painter, converter);
    }

    paintLabel(painter, converter);
}

void ParagraphTool::repaintDecorations()
{
    if (!m_needsRepaint) {
        return;
    }

    // TODO: should add previous and current label positions to the repaint area
    QRectF repaintRectangle = m_storedRepaintRectangle;
    m_storedRepaintRectangle = QRectF();
    foreach (ShapeSpecificData shape, m_shapes) {
        m_storedRepaintRectangle |= shape.dirtyRectangle();
    }
    repaintRectangle |= m_storedRepaintRectangle;

    canvas()->updateCanvas(repaintRectangle);

    m_needsRepaint = false;
}

void ParagraphTool::scheduleRepaint()
{
    m_needsRepaint = true;
}

void ParagraphTool::deselectTextBlock()
{
    if (!m_textBlockValid)
        return;

    emit styleNameChanged(QString(i18n("n/a")));

    deactivateRuler();
    dehighlightRuler();

    m_textBlockValid = false;
    scheduleRepaint();
}

bool ParagraphTool::activateRulerAt(const QPointF &point)
{
    if (!m_textBlockValid) {
        // can't select a ruler without a textblock
        return false;
    }

    for (int shape = 0; shape != m_shapes.size(); ++shape) {
        QPointF mappedPoint(m_shapes[shape].mapDocumentToShape(point));

        for (int ruler = 0; ruler != maxRuler; ++ruler) {
            if (hitTest((RulerIndex)ruler, m_shapes[shape], mappedPoint)) {
                activateRuler((RulerIndex)ruler, &m_shapes[shape]);

                return true;
            }
        }
    }

    m_activeRuler = noRuler;
    return false;
}


void ParagraphTool::activateRuler(RulerIndex ruler, ShapeSpecificData *shape)
{
    m_activeRuler = ruler;
    m_activeShape = shape;
    m_rulers[m_activeRuler].setActive(true);

    // disable hovering if we have an active ruler
    // hovering over the wrong ruler confuses the user
    if (hasHighlightedRuler()) {
        dehighlightRuler();
    }
}

void ParagraphTool::deactivateRuler()
{
    if (!hasActiveRuler()) {
        return;
    }

    RulerIndex deactivateRuler = m_activeRuler;
    m_activeRuler = noRuler;
    m_rulers[deactivateRuler].setActive(false);

    // there's no active ruler anymore, so we have to check if we hover somewhere
    highlightRulerAt(m_mousePosition);
}

void ParagraphTool::resetActiveRuler()
{
    if (hasActiveRuler()) {
        m_rulers[m_activeRuler].reset();
        deactivateRuler();
    }
}

void ParagraphTool::highlightRulerAt(const QPointF &point)
{
    // check if we were already hovering over an element
    if (hasHighlightedRuler()) {
        QPointF mappedPoint(m_highlightShape->mapDocumentToShape(point));

        // check if we are still over the same element
        if (hitTest((RulerIndex)m_highlightRuler, *m_highlightShape, mappedPoint)) {
            return;
        }
        else {
            // stop hovering over the element
            dehighlightRuler();
        }
    }

    // check if we are hovering over a new control
    for (int shape = 0; shape != m_shapes.size(); ++shape) {
        QPointF mappedPoint(m_shapes[shape].mapDocumentToShape(point));

        for (int ruler = 0; ruler != maxRuler; ++ruler) {
            if (hitTest((RulerIndex)ruler, m_shapes[shape], mappedPoint)) {
                m_highlightRuler = (RulerIndex)ruler;
                m_highlightShape = &m_shapes[shape];
                m_rulers[m_highlightRuler].setHighlighted(true);
                break;
            }
        }
    }
}

void ParagraphTool::dehighlightRuler()
{
    if (hasHighlightedRuler()) {
        m_rulers[m_highlightRuler].setHighlighted(false);
        m_highlightRuler = noRuler;
    }
}

void ParagraphTool::applyParentStyleToActiveRuler()
{
    if (!hasActiveRuler()) {
        return;
    }

    if (m_activeRuler == firstIndentRuler) {
        m_paragraphStyle->remove(QTextFormat::TextIndent);
        m_rulers[m_activeRuler].setValue(m_paragraphStyle->textIndent());
    }
    else if (m_activeRuler == followingIndentRuler) {
        m_paragraphStyle->remove(QTextFormat::BlockLeftMargin);
        m_rulers[m_activeRuler].setValue(m_paragraphStyle->leftMargin());
    }
    else if (m_activeRuler == rightMarginRuler) {
        m_paragraphStyle->remove(QTextFormat::BlockRightMargin);
        m_rulers[m_activeRuler].setValue(m_paragraphStyle->rightMargin());
    }
    else if (m_activeRuler == topMarginRuler) {
        m_paragraphStyle->remove(QTextFormat::BlockTopMargin);
        m_rulers[m_activeRuler].setValue(m_paragraphStyle->topMargin());
    }
    else if (m_activeRuler == bottomMarginRuler) {
        m_paragraphStyle->remove(QTextFormat::BlockBottomMargin);
        m_rulers[m_activeRuler].setValue(m_paragraphStyle->bottomMargin());
    }

    deactivateRuler();

    // we need to call the updateLayout() slot manually, it is not emitted if
    // the ruler has been changed by calling setValue()
    // TODO: maybe setValue() should emit valueChanged(), too
    // but make sure that this doesn't trigger additional repaints or relayouts
    updateLayout();
}

// try to find and select a text block below the cursor. return true if successfull
bool ParagraphTool::selectTextBlockAt(const QPointF &point)
{
    TextShape *textShape = dynamic_cast<TextShape*> (canvas()->shapeManager()->shapeAt(point));
    if (!textShape) {
        // the shape below the cursor is not a text shape
        return false;
    }

    KoTextShapeData *textShapeData = static_cast<KoTextShapeData*> (textShape->userData());
    QTextDocument *document = textShapeData->document();

    int position = document->documentLayout()->hitTest(textShape->convertScreenPos(point), Qt::ExactHit);
    if (position == -1) {
        // there is no text below the cursor
        return false;
    }

    QTextBlock textBlock(document->findBlock(position));
    if (!textBlock.isValid()) {
        // the text block is not valid, this shouldn't really happen
        return false;
    }

    // the text block is already selected, no need for a repaint and all that
    if (m_textBlockValid && textBlock == m_block) {
        return true;
    }

    m_textBlockValid = true;
    m_block = textBlock;
    m_paragraphStyle = KoParagraphStyle::fromBlock(textBlock);

    initParagraphProperties();
    if (createShapeList()) {
        loadRulers();
    }
    else {
        deselectTextBlock();
    }

    emit styleNameChanged(styleName());

    return true;
}

void ParagraphTool::mousePressEvent(KoPointerEvent *event)
{
    m_mousePosition = event->point;

    if (event->button() == Qt::LeftButton) {

        bool activated = activateRulerAt(event->point);

        if (!activated) {
            deselectTextBlock();
            selectTextBlockAt(event->point);
        }
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

    if (m_textBlockValid) {
        deactivateRuler();
    }

    repaintDecorations();
}

void ParagraphTool::mouseMoveEvent(KoPointerEvent *event)
{
    m_mousePosition = event->point;

    if (m_textBlockValid) {
        if (hasActiveRuler()) {
            moveActiveRulerTo(event->point);
        }
        else {
            highlightRulerAt(event->point);
        }

        repaintDecorations();
    }
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

bool ParagraphTool::hitTest(RulerIndex ruler, const ShapeSpecificData &shape, const QPointF &point) const
{
    return m_rulers[ruler].hitTest(point, shape.baseline(ruler));
}

void ParagraphTool::moveActiveRulerTo(const QPointF &point)
{
    QPointF mappedPoint(m_activeShape->mapDocumentToShape(point));
    m_rulers[m_activeRuler].moveRuler(mappedPoint, smoothMovement(), m_activeShape->baseline(m_activeRuler));
}

