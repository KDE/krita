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

#ifndef CHARACTERGENERAL_H
#define CHARACTERGENERAL_H

#include "ui_CharacterGeneral.h"

#include <QWidget>

class KoCharacterStyle;
class FontDecorations;
class CharacterHighlighting;
class FontTab;
class FontLayoutTab;
class LanguageTab;

class CharacterGeneral : public QWidget
{
    Q_OBJECT
public:
    explicit CharacterGeneral(QWidget *parent = 0, bool uniqueFormat = true);

    void setStyle(KoCharacterStyle *style);
    void hideStyleName(bool hide);

public slots:
    void save(KoCharacterStyle *style = 0);

    void switchToGeneralTab();

signals:
    void nameChanged(const QString &name);
    void styleAltered(const KoCharacterStyle *style);

private slots:
    void setName(const QString &name);
    void slotFontSelected(const QFont &);
    void slotBackgroundColorChanged(QColor);
    void slotTextColorChanged(QColor);
    void slotUnderlineChanged(KoCharacterStyle::LineType, KoCharacterStyle::LineStyle, QColor);
    void slotStrikethroughChanged(KoCharacterStyle::LineType, KoCharacterStyle::LineStyle, QColor);
    void slotCapitalizationChanged(QFont::Capitalization capitalisation);

private:
    Ui::CharacterGeneral widget;
    bool m_blockSignals;
    bool m_nameHidden;

    FontLayoutTab *m_layoutTab;
    FontDecorations *m_characterDecorations;
    CharacterHighlighting *m_characterHighlighting;
    FontTab *m_fontTab;
    LanguageTab *m_languageTab;

    KoCharacterStyle *m_style;
};

#endif
