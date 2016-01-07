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
#include <kfontchooser.h>

class QColor;

class CharacterHighlighting : public QWidget
{
    Q_OBJECT

public:
    explicit CharacterHighlighting(bool uniqueFormat, QWidget *parent = 0);
    ~CharacterHighlighting() {}

    void setDisplay(KoCharacterStyle *style);
    void save(KoCharacterStyle *style);

    QStringList capitalizationList();
    QStringList fontLayoutPositionList();

Q_SIGNALS:
    void underlineChanged(KoCharacterStyle::LineType, KoCharacterStyle::LineStyle, QColor);
    void strikethroughChanged(KoCharacterStyle::LineType, KoCharacterStyle::LineStyle, QColor);
    void capitalizationChanged(QFont::Capitalization);
    void fontChanged(const QFont &font);
    void textColorChanged(QColor);
    void backgroundColorChanged(QColor);
    void charStyleChanged();

private Q_SLOTS:
    void underlineTypeChanged(int item);
    void underlineStyleChanged(int item);
    void underlineColorChanged(QColor color);
    void strikethroughTypeChanged(int item);
    void strikethroughStyleChanged(int item);
    void strikethroughColorChanged(QColor color);
    void capitalisationChanged(int item);
    void positionChanged(int item);
    void textToggled(bool state);
    void backgroundToggled(bool state);
    void clearTextColor();
    void clearBackgroundColor();
    void textColorChanged();
    void backgroundColorChanged();

private:
    KoCharacterStyle::LineType indexToLineType(int index);
    KoCharacterStyle::LineStyle indexToLineStyle(int index);
    int lineTypeToIndex(KoCharacterStyle::LineType type);
    int lineStyleToIndex(KoCharacterStyle::LineStyle type);

    Ui::CharacterHighlighting widget;

    KFontChooser *m_fontChooser;

    bool m_uniqueFormat;
    bool m_underlineInherited;
    bool m_strikeoutInherited;
    bool m_mixedCaseInherited;
    bool m_smallCapsInherited;
    bool m_allUpperCaseInherited;
    bool m_allLowerCaseInherited;
    bool m_capitalizInherited;
    bool m_positionInherited;
    bool m_hyphenateInherited;
    bool m_textColorChanged;
    bool m_textColorReset;
    bool m_backgroundColorChanged;
    bool m_backgroundColorReset;
};

#endif
