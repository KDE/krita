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

#include <QWidget>

class KoParagraphStyle;
class KoUnit;

class ParagraphIndentSpacing : public QWidget
{
    Q_OBJECT
public:
    explicit ParagraphIndentSpacing(QWidget *parent);
    // open and display the style
    void setDisplay(KoParagraphStyle *style);
    void setUnit(const KoUnit &unit);

    // save widget state to style
    void save(KoParagraphStyle *style);

Q_SIGNALS:
    void parStyleChanged();

private Q_SLOTS:
    void lineSpacingChanged(int);
    void spacingValueChanged();
    void spacingPercentChanged();
    void useFontMetrices(bool);
    void autoTextIndentChanged(int state);
    void firstIndentValueChanged();
    void leftMarginValueChanged();
    void rightMarginValueChanged();
    void bottomMarginValueChanged();
    void topMarginValueChanged();
    void firstLineMarginChanged(qreal margin);
    void leftMarginChanged(qreal margin);
    void rightMarginChanged(qreal margin);

private:
    Ui::ParagraphIndentSpacing widget;

    KoParagraphStyle *m_style;
    bool m_fontMetricsChecked;
    bool m_rightMarginIngerited;
    bool m_leftMarginInherited;
    bool m_topMarginInherited;
    bool m_bottomMarginInherited;
    bool m_textIndentInherited;
    bool m_autoTextIndentInherited;
    bool m_spacingInherited;
};

#endif
