/* This file is part of the KDE project
   Copyright (C)  2001,2002,2003 Montel Laurent <lmontel@mandrakesoft.com>
   Copyright (C)  2006-2007 Thomas Zander <zander@kde.org>
   Copyright (C)  2008 Girish Ramakrishnan <girish@forwardbias.in>
   Copyright (C)  2008 Pierre Stirnweiss <pstirnweiss@googlemail.com>

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

#include <QTextCursor>


class CharacterGeneral;

class FontDia : public KDialog
{
    Q_OBJECT
public:
    explicit FontDia(QTextCursor *cursor, QWidget* parent = 0);

signals:
    /// emitted when a series of commands is started that together need to become 1 undo action.
    void startMacro(const QString &name);
    /// emitted when a series of commands has ended that together should be 1 undo action.
    void stopMacro();

protected slots:
    void slotReset();
    void slotApply();
    void slotOk();

private:
    void initTabs();

    CharacterGeneral *m_characterGeneral;

    QTextCursor* m_cursor;
    QTextCharFormat m_initialFormat;

    bool m_uniqueFormat;
};

#endif
