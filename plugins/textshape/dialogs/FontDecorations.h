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

#ifndef FONTDECORATIONS_H
#define FONTDECORATIONS_H

#include "ui_FontDecorations.h"

#include <KoCharacterStyle.h>

class FontDecorations : public QWidget
{
    Q_OBJECT

public:
    explicit FontDecorations( QWidget* parent=0);
    ~FontDecorations() {}

    void open(KoCharacterStyle *style);
    void save() const;

private slots:
    void clearTextColor();
    void clearBackgroundColor();
    void textColorChanged() { m_textColorReset = false; m_textColorChanged = true; }
    void backgroundColorChanged() { m_backgroundColorReset = false; m_backgroundColorChanged = true; }

private:
    Ui::FontDecorations widget;

    bool m_textColorChanged, m_textColorReset;
    bool m_backgroundColorChanged, m_backgroundColorReset;

    KoCharacterStyle *m_style;
};

#endif
