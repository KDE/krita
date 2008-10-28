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

#include "ParagraphEditor.h"
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
    initializeRuler(m_rulers[firstIndentRuler], i18n("First Line:"));
    initializeRuler(m_rulers[followingIndentRuler], i18n("Left:"));
    initializeRuler(m_rulers[rightMarginRuler], i18n("Right:"));
    initializeRuler(m_rulers[topMarginRuler], i18n("Before:"), Ruler::drawSides);
    initializeRuler(m_rulers[bottomMarginRuler], i18n("After:"), Ruler::drawSides);
    initializeRuler(m_rulers[lineSpacingRuler], i18n("Line Spacing:"));
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
    connect(&ruler, SIGNAL(valueChanged(qreal)), this, SLOT(updateLayout()));
}

void ParagraphEditor::loadRulers()
{
    m_rulers[firstIndentRuler].setValue(paragraphStyle()->leftMargin() + paragraphStyle()->textIndent());
    m_rulers[followingIndentRuler].setValue(paragraphStyle()->leftMargin());
    m_rulers[rightMarginRuler].setValue(paragraphStyle()->rightMargin());
    m_rulers[topMarginRuler].setValue(paragraphStyle()->topMargin());
    m_rulers[bottomMarginRuler].setValue(paragraphStyle()->bottomMargin());

    scheduleRepaint();
}


void ParagraphEditor::saveRulers()
{
    paragraphStyle()->setLeftMargin(m_rulers[followingIndentRuler].value());
    paragraphStyle()->setRightMargin(m_rulers[rightMarginRuler].value());
    paragraphStyle()->setTopMargin(m_rulers[topMarginRuler].value());
    paragraphStyle()->setBottomMargin(m_rulers[bottomMarginRuler].value());
    paragraphStyle()->setTextIndent(m_rulers[firstIndentRuler].value() - m_rulers[followingIndentRuler].value());

    QTextBlockFormat format;
    paragraphStyle()->applyStyle(format);

    cursor().mergeBlockFormat(format);
}

QString ParagraphEditor::styleName()
{
    KoParagraphStyle *style = paragraphStyle();
    while (style != NULL) {
        QString name = style->name();
        if (!name.isNull() && !name.isEmpty()) {
            return name;
        }
        style = style->parentStyle();
    }

    return QString(i18n("None"));
}

void ParagraphEditor::paintLabel(QPainter &painter, const KoViewConverter &converter) const
{
    RulerIndex ruler;
    RulerFragment *rulerFragment;
    QColor foregroundColor;

    if (hasActiveRuler()) {
        ruler = m_activeRuler;
        rulerFragment = m_activeRulerFragment;
        foregroundColor = m_rulers[ruler].activeColor();
    } else if (hasHighlightedRuler()) {
        ruler = m_highlightedRuler;
        rulerFragment = m_highlightedRulerFragment;
        foregroundColor = m_rulers[ruler].highlightColor();
    } else {
        return;
    }

    painter.save();

    QLineF unmapped(rulerFragment->labelConnector());
    QLineF connector(converter.documentToView(unmapped.p1()), converter.documentToView(unmapped.p2()));
    connector.setLength(10.0);

    QString text(m_rulers[ruler].name() + ' ' + m_rulers[ruler].valueString());

    painter.setBrush(Qt::white);
    painter.setPen(foregroundColor);

    QRectF label(connector.p2().x(), connector.p2().y(), 0.0, 0.0);
    label = painter.boundingRect(label, Qt::AlignCenter | Qt::AlignVCenter, text);
    label.adjust(-4.0, 0.0, 4.0, 0.0);

    // adjust label so that the connector ends on the edge of the label
    if (abs(connector.dx()) > abs(connector.dy())) {
        qreal halfWidth = connector.dy() < 0.0 ? label.width() / 2.0 : -label.width() / 2.0;
        label.adjust(halfWidth, 0.0, halfWidth, 0.0);
    } else {
        qreal halfHeight = connector.dy() >= 0.0 ? label.height() / 2.0 : -label.height() / 2.0;
        label.adjust(0.0, halfHeight, 0.0, halfHeight);
    }

    painter.drawLine(connector);
    painter.drawRoundRect(label, 720.0 / label.width(), 720.0 /  label.height());
    painter.drawText(label, Qt::AlignHCenter | Qt::AlignVCenter, text);

    painter.restore();
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

QRectF ParagraphEditor::dirtyRectangle()
{
    if (!needsRepaint()) {
        QRectF();
    }

    // TODO: should add previous and current label positions to the
    // repaint area
    QRectF repaintRectangle = m_storedRepaintRectangle;
    m_storedRepaintRectangle = QRectF();
    foreach(KoShape *shape, shapes()) {
        QRectF boundingRect(QPointF(0, 0), shape->size());

        if (shape->border()) {
            KoInsets insets;
            shape->border()->borderInsets(shape, insets);
            boundingRect.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
        }

        // adjust for arrow heads and label
        // (although we can't be sure about the label)
        boundingRect.adjust(-50.0, -50.0, 50.0, 50.0);

        boundingRect = shape->absoluteTransformation(0).mapRect(boundingRect);

        m_storedRepaintRectangle |= boundingRect;
    }
    repaintRectangle |= m_storedRepaintRectangle;

    return repaintRectangle;
}

void ParagraphEditor::addShapes()
{
    m_rulers[firstIndentRuler].clearFragments();
    m_rulers[followingIndentRuler].clearFragments();
    m_rulers[rightMarginRuler].clearFragments();
    m_rulers[topMarginRuler].clearFragments();
    m_rulers[bottomMarginRuler].clearFragments();
    m_rulers[lineSpacingRuler].clearFragments();

    ParagraphBase::addShapes();

    foreach(KoShape *shape, shapes()) {
        ParagraphFragment(m_rulers, shape, textBlock(), paragraphStyle());
    }

    loadRulers();
}

/* slot which is called when the value of one of the rulers changed
 * causes storing and reloading of the positions and values of the rulers
 * from the file */
void ParagraphEditor::updateLayout()
{
    saveRulers();

    static_cast<KoTextDocumentLayout*>(textBlock().document()->documentLayout())->layout();

    addShapes();

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

    if (m_activeRuler == firstIndentRuler) {
        paragraphStyle()->remove(QTextFormat::TextIndent);
        m_rulers[m_activeRuler].setValue(paragraphStyle()->textIndent());
    } else if (m_activeRuler == followingIndentRuler) {
        paragraphStyle()->remove(QTextFormat::BlockLeftMargin);
        m_rulers[m_activeRuler].setValue(paragraphStyle()->leftMargin());
    } else if (m_activeRuler == rightMarginRuler) {
        paragraphStyle()->remove(QTextFormat::BlockRightMargin);
        m_rulers[m_activeRuler].setValue(paragraphStyle()->rightMargin());
    } else if (m_activeRuler == topMarginRuler) {
        paragraphStyle()->remove(QTextFormat::BlockTopMargin);
        m_rulers[m_activeRuler].setValue(paragraphStyle()->topMargin());
    } else if (m_activeRuler == bottomMarginRuler) {
        paragraphStyle()->remove(QTextFormat::BlockBottomMargin);
        m_rulers[m_activeRuler].setValue(paragraphStyle()->bottomMargin());
    }

    updateLayout();
}

