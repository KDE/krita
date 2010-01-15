/* This file is part of the KDE project
   Copyright (C)  2001,2002,2003 Montel Laurent <lmontel@mandrakesoft.com>
   Copyright (C)  2006 Thomas Zander <zander@kde.org>
   Copyright (C)  2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef CHARACTERHIGHLIGHTING_H
#define CHARACTERHIGHLIGHTING_H

#include <ui_CharacterHighlighting.h>

#include "KoCharacterStyle.h"

class QColor;

class CharacterHighlighting : public QWidget
{
    Q_OBJECT

public:
    explicit CharacterHighlighting(bool uniqueFormat, QWidget* parent = 0);
    ~CharacterHighlighting() {}

    void setDisplay(KoCharacterStyle *style);
    void save(KoCharacterStyle *style);

signals:
    void underlineChanged(KoCharacterStyle::LineType, KoCharacterStyle::LineStyle, QColor);
    void strikethroughChanged(KoCharacterStyle::LineType, KoCharacterStyle::LineStyle, QColor);
    void capitalizationChanged(QFont::Capitalization);

private slots:
    void underlineTypeChanged(int item);
    void underlineStyleChanged(int item);
    void underlineColorChanged(QColor color);
    void strikethroughTypeChanged(int item);
    void strikethroughStyleChanged(int item);
    void strikethroughColorChanged(QColor color);
    void capitalisationChanged();

private:
    KoCharacterStyle::LineType indexToLineType(int index);
    KoCharacterStyle::LineStyle indexToLineStyle(int index);
    int lineTypeToIndex(KoCharacterStyle::LineType type);
    int lineStyleToIndex(KoCharacterStyle::LineStyle type);

    Ui::CharacterHighlighting widget;

    bool m_uniqueFormat;
};

#endif
