/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#ifndef PARAGRAPHINDENTSPACING_H
#define PARAGRAPHINDENTSPACING_H

#include <ui_ParagraphIndentSpacing.h>

#include <KoUnit.h>

#include <QWidget>

class KoParagraphStyle;

class ParagraphIndentSpacing : public QWidget
{
    Q_OBJECT
public:
    ParagraphIndentSpacing(QWidget *parent);
    // open and display the style
    void setDisplay(KoParagraphStyle *style);
    void setUnit(const KoUnit &unit);

    // save widget state to style
    void save(KoParagraphStyle *style);

signals:
    void firstLineMarginChanged(qreal margin);
    void leftMarginChanged(qreal margin);
    void lineSpacingChanged(qreal fixedLineHeight, qreal lineSpacing, qreal minimumLineHeight, int percentLineSpacing, bool useFontProperties);
    void rightMarginChanged(qreal margin);

private slots:
    void lineSpacingChanged(int);
    void spacingValueChanged(qreal value);
    void spacingPercentChanged(int percent);
    void useFontMetrices(bool);
    void autoTextIndentChanged(int state);

private:
    Ui::ParagraphIndentSpacing widget;

    KoParagraphStyle *m_style;
    bool m_fontMetricsChecked;
};

#endif
