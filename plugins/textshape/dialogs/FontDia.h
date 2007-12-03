/* This file is part of the KDE project
   Copyright (C)  2001,2002,2003 Montel Laurent <lmontel@mandrakesoft.com>
   Copyright (C)  2006-2007 Thomas Zander <zander@kde.org>

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

#ifndef FONTDIA_H
#define FONTDIA_H

#include <kdialog.h>
#include <KoCharacterStyle.h>

#include <QTextCharFormat>

class FontTab;
class CharacterHighlighting;
class FontDecorations;
class FontLayoutTab;
class LanguageTab;

class FontDia : public KDialog
{
    Q_OBJECT
public:

    /// If your application supports spell-checking, pass here the KSpell2 Loader
    /// so that the font dialog can show which languages are supported for spellchecking.
    explicit FontDia( const QTextCharFormat &format/* KSpell2::Loader::Ptr loader = KSpell2::Loader::Ptr()*/, QWidget* parent = 0);

    KoCharacterStyle style() const { return m_style; }

protected slots:
    void slotReset();
    void slotApply();
    void slotOk();

private:
    FontTab *fontTab;
    CharacterHighlighting *m_highlightingTab;
    FontDecorations *m_decorationTab;
    FontLayoutTab *m_layoutTab;
    LanguageTab *languageTab;

    KoCharacterStyle m_style;
};

#endif
