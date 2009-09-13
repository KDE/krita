/* This file is part of the KDE project
 * Copyright (C) 2008 Florian Merz <florianmerz@gmx.de>
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

#include "ParagraphEditor.h"
#include "Label.h"
#include "dialogs/OptionWidget.h"

#include <KoCanvasBase.h>
#include <KoParagraphStyle.h>
#include <KoShape.h>
#include <KoShapeBorderModel.h>
#include <KoShapeManager.h>
#include <KoStyleManager.h>
#include <KoTextShapeData.h>
#include <KoTextDocumentLayout.h>
#include <KoUnit.h>
#include <KoViewConverter.h>

#include <KLocalizedString>

#include <QAbstractTextDocumentLayout>
#include <QColor>
#include <QPainter>
#include <QString>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextLine>

ParagraphEditor::ParagraphEditor(QObject *parent, KoCanvasBase *canvas)
        : ParagraphBase(parent, canvas),
        m_activeRuler(noRuler),
        m_focusedRuler(noRuler),
        m_highlightedRuler(noRuler),
        m_smoothMovement(false)
{
    initializeRuler(m_rulers[textIndentRuler], i18n("First Line:"));
    connect(&m_rulers[textIndentRuler], SIGNAL(valueChanged(qreal)), this, SLOT(saveTextIndent()));

    initializeRuler(m_rulers[leftMarginRuler], i18n("Left:"));
    connect(&m_rulers[leftMarginRuler], SIGNAL(valueChanged(qreal)), this, SLOT(saveLeftMargin()));

    initializeRuler(m_rulers[rightMarginRuler], i18n("Right:"));
    connect(&m_rulers[rightMarginRuler], SIGNAL(valueChanged(qreal)), this, SLOT(saveRightMargin()));

    initializeRuler(m_rulers[topMarginRuler], i18n("Before:"), Ruler::drawSides);
    connect(&m_rulers[topMarginRuler], SIGNAL(valueChanged(qreal)), this, SLOT(saveTopMargin()));

    initializeRuler(m_rulers[bottomMarginRuler], i18n("After:"), Ruler::drawSides);
    connect(&m_rulers[bottomMarginRuler], SIGNAL(valueChanged(qreal)), this, SLOT(saveBottomMargin()));

    initializeRuler(m_rulers[lineSpacingRuler], i18n("Line Spacing:"));
    connect(&m_rulers[lineSpacingRuler], SIGNAL(valueChanged(qreal)), this, SLOT(saveLineSpacing()));
    m_rulers[lineSpacingRuler].setEnabled(false);
}

ParagraphEditor::~ParagraphEditor()
{}

// helper function to initialize rulers
void ParagraphEditor::initializeRuler(Ruler &ruler, const QString &name, int options)
{
    ruler.setName(name);
    ruler.setParent(this);
    ruler.setOptions(options);
    ruler.setUnit(canvas()->unit());
    ruler.setMinimumValue(0.0);
    connect(&ruler, SIGNAL(needsRepaint()), this, SLOT(scheduleRepaint()));
}

void ParagraphEditor::loadRulers()
{
    m_rulers[textIndentRuler].setValue(paragraphStyle()->leftMargin() + paragraphStyle()->textIndent());
    m_rulers[leftMarginRuler].setValue(paragraphStyle()->leftMargin());
    m_rulers[rightMarginRuler].setValue(paragraphStyle()->rightMargin());
    m_rulers[topMarginRuler].setValue(paragraphStyle()->topMargin());
    m_rulers[bottomMarginRuler].setValue(paragraphStyle()->bottomMargin());

    scheduleRepaint();
}

void ParagraphEditor::saveLeftMargin()
{
    paragraphStyle()->setLeftMargin(m_rulers[leftMarginRuler].value());
    applyStyle();
}

void ParagraphEditor::saveRightMargin()
{
    paragraphStyle()->setRightMargin(m_rulers[rightMarginRuler].value());

    applyStyle();
}

void ParagraphEditor::saveTopMargin()
{
    paragraphStyle()->setTopMargin(m_rulers[topMarginRuler].value());

    applyStyle();
}

void ParagraphEditor::saveBottomMargin()
{
    paragraphStyle()->setBottomMargin(m_rulers[bottomMarginRuler].value());

    applyStyle();
}

void ParagraphEditor::saveTextIndent()
{
    paragraphStyle()->setTextIndent(m_rulers[textIndentRuler].value() - m_rulers[leftMarginRuler].value());

    applyStyle();
}

void ParagraphEditor::saveLineSpacing()
{
    paragraphStyle()->setTextIndent(m_rulers[lineSpacingRuler].value() - m_rulers[lineSpacingRuler].value());
    applyStyle();
}

void ParagraphEditor::paintLabel(QPainter &painter, const KoViewConverter &converter) const
{
    Label label;
    QLineF unmapped;
    QColor color;

    if (hasActiveRuler()) {
        unmapped = m_activeRulerFragment->labelConnector();
        color = m_rulers[m_activeRuler].activeColor();
        label.setText(m_rulers[m_activeRuler].name() + ' ' + m_rulers[m_activeRuler].valueString());
    } else if (hasHighlightedRuler()) {
        unmapped = m_highlightedRulerFragment->labelConnector();
        color = m_rulers[m_highlightedRuler].highlightColor();
        label.setText(m_rulers[m_highlightedRuler].name() + ' ' + m_rulers[m_highlightedRuler].valueString());
    } else {
        return;
    }

    // paint connector
    painter.save();

    painter.setPen(color);
    QLineF connector(converter.documentToView(unmapped.p1()), converter.documentToView(unmapped.p2()));
    connector.setLength(10.0);
    painter.drawLine(connector);

    painter.restore();

    // paint label
    label.setColor(color);

    Qt::Alignment alignment;
    if (abs(connector.dx()) > abs(connector.dy())) {
        if (connector.dy() < 0.0) {
            alignment = Qt::AlignLeft | Qt::AlignVCenter;
        } else {
            alignment = Qt::AlignRight | Qt::AlignVCenter;
        }
    } else {
        if (connector.dy() >= 0.0) {
            alignment = Qt::AlignHCenter | Qt::AlignBottom;
        } else {
            alignment = Qt::AlignHCenter | Qt::AlignTop;
        }
    }
    label.setPosition(connector.p2(), alignment);

    label.paint(painter);
}

void ParagraphEditor::paint(QPainter &painter, const KoViewConverter &converter)
{
    m_needsRepaint = false;

    if (hasActiveTextBlock()) {
        painter.save();

        // transform painter from view coordinate system to document coordinate system
        QPointF trans = converter.documentToView(QPointF(1.0, 1.0));
        QMatrix matrix = QMatrix().translate(trans.x(), trans.y());
        painter.setMatrix(matrix * painter.matrix());
        KoShape::applyConversion(painter, converter);
        painter.setPen(Qt::darkGray);

        for (int ruler = 0; ruler != maxRuler; ++ruler) {
            m_rulers[ruler].paint(painter);
        }

        painter.restore();

        paintLabel(painter, converter);
    }
}

QRectF ParagraphEditor::dirtyRectangle(bool updateWholeRegion)
{
    if ( !updateWholeRegion && !needsRepaint()) {
       return QRectF();
    }

    // TODO: should add previous and current label positions to the
    // repaint area
    QRectF repaintRectangle = m_storedRepaintRectangle;
    m_storedRepaintRectangle = QRectF();
    foreach(const ParagraphFragment &fragment, fragments()) {
        KoShape *shape = fragment.shape();
        QRectF boundingRect(QPointF(0, 0), shape->size());

        // adjust for arrow heads and label
        // (although we can't be sure about the label)
        boundingRect.adjust(-50.0, -50.0, 50.0, 50.0);

        boundingRect = shape->absoluteTransformation(0).mapRect(boundingRect);

        m_storedRepaintRectangle |= boundingRect;
    }
    repaintRectangle |= m_storedRepaintRectangle;

    return repaintRectangle;
}

void ParagraphEditor::initRulerFragments(const ParagraphFragment *fragment, Ruler *rulers) const
{
    KoShape *shape = fragment->shape();
    KoTextShapeData *textShapeData = static_cast<KoTextShapeData*>(shape->userData());

    QRectF border = fragment->border();
    QRectF firstLine = fragment->firstLine();
    QRectF followingLines = fragment->followingLines();

    bool isSingleLine = fragment->isSingleLine();

    qreal shapeTop(textShapeData->documentOffset());
    qreal shapeBottom(shapeTop + shape->size().height());

    // matrix to map text to document coordinates
    QMatrix matrix = shape->absoluteTransformation(NULL);
    matrix.translate(0.0, -shapeTop);

    qreal rightTop = qMax(shapeTop, firstLine.top());
    qreal followingTop = qMax(shapeTop, followingLines.top());
    qreal followingBottom = qMin(shapeBottom, followingLines.bottom());

    // first line
    RulerFragment firstFragment;
    firstFragment.setVisible(shapeTop < firstLine.bottom());
    firstFragment.setBaseline(matrix.map(QLineF(border.left(), firstLine.top(), border.left(), firstLine.bottom())));
    rulers[textIndentRuler].addFragment(firstFragment);

    // following lines
    RulerFragment followingFragment;
    followingFragment.setVisible(shapeTop < followingLines.bottom() && shapeBottom > followingLines.top() && !isSingleLine);
    followingFragment.setBaseline(matrix.map(QLineF(border.left(), followingTop, border.left(), followingBottom)));
    rulers[leftMarginRuler].addFragment(followingFragment);

    // right margin
    RulerFragment rightFragment;
    rightFragment.setVisible(true);
    rightFragment.setBaseline(matrix.map(QLineF(border.right(), followingBottom, border.right(), rightTop)));
    rulers[rightMarginRuler].addFragment(rightFragment);

    // top margin
    RulerFragment topFragment;
    topFragment.setVisible(shapeTop <= firstLine.top());
    topFragment.setBaseline(matrix.map(QLineF(border.right(), border.top(), border.left(), border.top())));
    rulers[topMarginRuler].addFragment(topFragment);

    // bottom margin
    RulerFragment bottomFragment;
    bottomFragment.setVisible(shapeBottom >= followingLines.bottom());
    bottomFragment.setBaseline(matrix.map(QLineF(border.right(), followingLines.bottom(), border.left(), followingLines.bottom())));
    rulers[bottomMarginRuler].addFragment(bottomFragment);

    // line spacing
    RulerFragment lineFragment;
    lineFragment.setVisible(!isSingleLine && rightTop != followingTop);
    lineFragment.setBaseline(matrix.map(QLineF(firstLine.right(), firstLine.bottom(), border.left(), firstLine.bottom())));
    rulers[lineSpacingRuler].addFragment(lineFragment);
}

void ParagraphEditor::addFragments()
{
    m_rulers[textIndentRuler].clearFragments();
    m_rulers[leftMarginRuler].clearFragments();
    m_rulers[rightMarginRuler].clearFragments();
    m_rulers[topMarginRuler].clearFragments();
    m_rulers[bottomMarginRuler].clearFragments();
    m_rulers[lineSpacingRuler].clearFragments();

    ParagraphBase::addFragments();

    foreach(const ParagraphFragment &fragment, fragments()) {
        initRulerFragments(&fragment, m_rulers);
    }

    loadRulers();
}

void ParagraphEditor::applyStyle()
{
    QTextBlockFormat format;
    paragraphStyle()->applyStyle(format);

    cursor().mergeBlockFormat(format);

    static_cast<KoTextDocumentLayout*>(textBlock().document()->documentLayout())->layout();

    addFragments();

    scheduleRepaint();
}

bool ParagraphEditor::activateRulerAt(const QPointF &point)
{
    if (!hasActiveTextBlock()) {
        // can't activate a ruler without a textblock
        return false;
    }

    for (int ruler = 0; ruler != maxRuler; ++ruler) {
        RulerFragment *fragment = m_rulers[ruler].hitTest(point);
        if (fragment != NULL) {
            activateRuler(static_cast<RulerIndex>(ruler), fragment);
            return true;
        }
    }

    m_activeRuler = noRuler;
    m_activeRulerFragment = NULL;
    return false;
}

void ParagraphEditor::activateRuler(RulerIndex ruler, RulerFragment *fragment)
{
    m_activeRuler = ruler;
    m_activeRulerFragment = fragment;
    m_rulers[m_activeRuler].setActive(true);

    defocusRuler();
    dehighlightRuler();
}

void ParagraphEditor::deactivateRuler()
{
    if (!hasActiveRuler()) {
        return;
    }

    RulerIndex activeRuler = m_activeRuler;
    m_activeRuler = noRuler;
    m_activeRulerFragment = NULL;
    m_rulers[activeRuler].setActive(false);

    focusRuler(activeRuler);
}

void ParagraphEditor::resetActiveRuler()
{
    if (hasActiveRuler()) {
        m_rulers[m_activeRuler].resetValue();
    }
}

void ParagraphEditor::moveActiveRulerTo(const QPointF &point)
{
    m_activeRulerFragment->moveTo(point, smoothMovement());
}

void ParagraphEditor::focusRuler(RulerIndex ruler)
{
    if (m_focusedRuler == ruler) {
        return;
    }

    if (hasFocusedRuler()) {
        m_rulers[m_focusedRuler].setFocused(false);
    }

    m_focusedRuler = ruler;
    m_rulers[m_focusedRuler].setFocused(true);
}

void ParagraphEditor::focusFirstRuler()
{
    focusRuler((RulerIndex)0);
}

void ParagraphEditor::focusLastRuler()
{
    focusRuler((RulerIndex)((int)maxRuler-1));
}

bool ParagraphEditor::focusPreviousRuler()
{
    RulerIndex newRuler = m_focusedRuler;

    do {
        if (newRuler == (RulerIndex)0) {
            return false;
        }
        
        newRuler = (RulerIndex)((int)newRuler - 1);
    } while (!m_rulers[newRuler].isVisible() || !m_rulers[newRuler].isEnabled());

    focusRuler(newRuler);

    return true;
}

bool ParagraphEditor::focusNextRuler()
{
    RulerIndex newRuler = m_focusedRuler;

    do {
        newRuler = (RulerIndex)((int)newRuler + 1);

        if (newRuler == maxRuler) {
            return false;
        }

    } while (!m_rulers[newRuler].isVisible() || !m_rulers[newRuler].isEnabled());

    focusRuler(newRuler);

    return true;
}

void ParagraphEditor::defocusRuler()
{
    if (!hasFocusedRuler()) {
        return;
    }

    m_rulers[m_focusedRuler].setFocused(false);
    m_focusedRuler = noRuler;
}

void ParagraphEditor::highlightRulerAt(const QPointF &point)
{
    // check if we were already hovering over an element
    if (hasHighlightedRuler()) {
        // check if we are still over the same element
        if (m_highlightedRulerFragment->hitTest(point)) {
            return;
        } else {
            // stop hovering over the element
            dehighlightRuler();
        }
    }

    // check if we are hovering over a new control
    for (int ruler = 0; ruler != maxRuler; ++ruler) {
        RulerFragment *fragment = m_rulers[ruler].hitTest(point);
        if (fragment != NULL) {
            m_highlightedRuler = static_cast<RulerIndex>(ruler);
            m_highlightedRulerFragment = fragment;
            m_rulers[m_highlightedRuler].setHighlighted(true);
            break;
        }
    }
}

void ParagraphEditor::dehighlightRuler()
{
    if (hasHighlightedRuler()) {
        m_rulers[m_highlightedRuler].setHighlighted(false);
        m_highlightedRuler = noRuler;
        m_highlightedRulerFragment = NULL;
    }
}

void ParagraphEditor::applyParentStyleToActiveRuler()
{
    if (!hasActiveRuler()) {
        return;
    }

    if (m_activeRuler == textIndentRuler) {
        paragraphStyle()->remove(QTextFormat::TextIndent);
        m_rulers[m_activeRuler].setValue(paragraphStyle()->textIndent());
        saveTextIndent();
    } else if (m_activeRuler == leftMarginRuler) {
        paragraphStyle()->remove(QTextFormat::BlockLeftMargin);
        m_rulers[m_activeRuler].setValue(paragraphStyle()->leftMargin());
        saveLeftMargin();
    } else if (m_activeRuler == rightMarginRuler) {
        paragraphStyle()->remove(QTextFormat::BlockRightMargin);
        m_rulers[m_activeRuler].setValue(paragraphStyle()->rightMargin());
        saveRightMargin();
    } else if (m_activeRuler == topMarginRuler) {
        paragraphStyle()->remove(QTextFormat::BlockTopMargin);
        m_rulers[m_activeRuler].setValue(paragraphStyle()->topMargin());
        saveTopMargin();
    } else if (m_activeRuler == bottomMarginRuler) {
        paragraphStyle()->remove(QTextFormat::BlockBottomMargin);
        m_rulers[m_activeRuler].setValue(paragraphStyle()->bottomMargin());
        saveBottomMargin();
    }

}

