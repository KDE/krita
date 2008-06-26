/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (c) 2003 David Faure <faure@kde.org>
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

#include "ParagraphIndentSpacing.h"

#include <KoParagraphStyle.h>
#include <KDebug>

ParagraphIndentSpacing::ParagraphIndentSpacing(QWidget *parent)
    : QWidget(parent),
    m_fontMetricsChecked(false)
{
    widget.setupUi(this);

    // Keep order in sync with lineSpacingType() and display()
    widget.lineSpacing->addItem( i18nc( "Line spacing value", "Single" ) );
    widget.lineSpacing->addItem( i18nc( "Line spacing value", "1.5 Lines" ) );
    widget.lineSpacing->addItem( i18nc( "Line spacing value", "Double" ) );
    widget.lineSpacing->addItem( i18nc( "Line spacing type", "Proportional") ); // called Proportional like in OO
    widget.lineSpacing->addItem( i18nc( "Line spacing type", "Additional") ); // normal distance + absolute value
    widget.lineSpacing->addItem( i18nc( "Line spacing type", "Fixed") );

    connect(widget.lineSpacing, SIGNAL(currentIndexChanged(int)), this, SLOT(lineSpacingChanged(int)));
    connect(widget.useFont, SIGNAL(toggled (bool)), this, SLOT(useFontMetrices(bool)));
    connect(widget.autoTextIndent, SIGNAL(stateChanged (int)), this, SLOT(autoTextIndentChanged(int)));
    lineSpacingChanged(0);
}

void ParagraphIndentSpacing::autoTextIndentChanged(int state) {
    widget.first->setEnabled(state == Qt::Unchecked);
}

void ParagraphIndentSpacing::open(KoParagraphStyle *style) {
    m_style = style;
    widget.first->changeValue(style->textIndent());
    widget.left->changeValue(style->leftMargin());
    widget.right->changeValue(style->rightMargin());
    widget.before->changeValue(style->topMargin());
    widget.after->changeValue(style->bottomMargin());
    widget.autoTextIndent->setChecked(style->autoTextIndent());

    int index;
    if(style->lineHeightPercent() != 0) {
        int percent = style->lineHeightPercent();
        if(percent == 120)
            index = 0; // single
        else if(percent == 180)
            index = 1; // 1.5
        else if(percent == 240)
            index = 2; // double
        else
            index = 3; // proportional
    }
    else if(style->lineSpacing() > 0.0)
        index = 4; // Additional
    else
        index = 5;
    widget.lineSpacing->setCurrentIndex(index);
    widget.minimumLineSpacing->changeValue(m_style->minimumLineHeight());
    widget.useFont->setChecked(m_style->lineSpacingFromFont());
    m_fontMetricsChecked = m_style->lineSpacingFromFont();
}

void ParagraphIndentSpacing::lineSpacingChanged(int row) {
    bool percent = false, custom = false;
    double customValue = 0.0;
    switch(row) {
        case 3: // proportional
            percent = true;
            widget.proportional->setValue(m_style->lineHeightPercent());
            break;
        case 4: // additional
            custom = true;
            customValue = qMax(0.1, m_style->lineSpacing());
            break;
        case 5: // fixed
            custom = true;
            if(m_style->lineHeightAbsolute() == 0) // unset
                customValue = 12.0; // nice default value...
            else
                customValue = m_style->lineHeightAbsolute();
            break;
        default:; // other cases don't need the spinboxes
    }

    if(custom) {
        widget.custom->setEnabled(true);
        widget.spacingStack->setCurrentWidget(widget.unitsPage);
        widget.custom->changeValue(customValue);
    }
    else {
        widget.spacingStack->setCurrentWidget(widget.percentPage);
        widget.proportional->setEnabled(percent);
        if(! percent)
            widget.proportional->setValue(100);
    }

    widget.minimumLineSpacing->setEnabled(row != 5);
    widget.useFont->setEnabled(row != 5);
    widget.useFont->setChecked( row == 5 ? false : m_fontMetricsChecked);
}

void ParagraphIndentSpacing::save() {
    // general note; we have to unset values by setting it to zero instead of removing the item
    // since this dialog may be used on a copy style, which will be applied later. And removing
    // items doesn't work for that.
    m_style->setTextIndent( widget.first->value() );
    m_style->setLeftMargin( widget.left->value() );
    m_style->setRightMargin( widget.right->value() );
    m_style->setTopMargin( widget.before->value() );
    m_style->setBottomMargin( widget.after->value() );
    m_style->setAutoTextIndent( widget.autoTextIndent->isChecked() );
    m_style->setLineHeightAbsolute(0); // since it trumps percentage based line heights, unset it.
    switch(widget.lineSpacing->currentIndex()) {
        case 0: m_style->setLineHeightPercent(120); break;
        case 1: m_style->setLineHeightPercent(180); break;
        case 2: m_style->setLineHeightPercent(240); break;
        case 3: m_style->setLineHeightPercent(widget.proportional->value()); break;
        case 4: m_style->setLineHeightPercent(0);
                m_style->setLineSpacing(widget.custom->value());
                break;
        case 5: m_style->setLineHeightPercent(0);
                m_style->setLineSpacing(0);
                m_style->setLineHeightAbsolute(widget.custom->value());
                break;
    }
    if(widget.lineSpacing->currentIndex() != 5)
        m_style->setMinimumLineHeight(widget.minimumLineSpacing->value());
    m_style->setLineSpacingFromFont(widget.lineSpacing->currentIndex() != 5 && widget.useFont->isChecked());
}

void ParagraphIndentSpacing::setUnit(const KoUnit &unit) {
    widget.first->setUnit(unit);
    widget.left->setUnit(unit);
    widget.right->setUnit(unit);
    widget.before->setUnit(unit);
    widget.after->setUnit(unit);
    widget.custom->setUnit(unit);
    widget.minimumLineSpacing->setUnit(unit);
}

void ParagraphIndentSpacing::useFontMetrices(bool on) {
    if(widget.lineSpacing->currentIndex() != 5)
        m_fontMetricsChecked = on;
}

#include "ParagraphIndentSpacing.moc"
