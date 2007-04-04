/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   Copyright (C) 2007 Thomas Zander <zander@kde.org>

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

#ifndef KOCHARSELECTDIA_H
#define KOCHARSELECTDIA_H

#include <kdialog.h>
#include <kofficeui_export.h>

class QWidget;
class QGridLayout;
class QPushButton;
class KCharSelect;
class KButtonBox;

/**
 * A dialog for selecting a character.
 */
class KOFFICEUI_EXPORT KoCharSelectDia : public KDialog
{
    Q_OBJECT

public:

    // constructor
    KoCharSelectDia( QWidget *parent, const char *name, const QChar &_chr,
                     const QString &_font, bool _modal=true );

    //constructor when you want to insert multi char
    KoCharSelectDia( QWidget *parent, const char *name, const QString &_font,
                     const QChar &_chr, bool _modal=true );
    ~KoCharSelectDia();
    /**
     * Shows the selection dialog and returns true if user pressed ok, after filling the font and character parameters.
     * @param font will be filled when the user pressed Ok with the selected font.
     * @param character will be filled when the user pressed Ok with the selected character.
     * @param parent the parent widget this dialog will be associated with.
     */
    KOFFICEUI_EXPORT static bool selectChar( QString &font, QChar &character, QWidget* parent = 0, const char* name = 0);

signals:
    /**
     * Emitted when the user presses the 'insert' button.
     * @param character the character that the user selected
     * @param font the font name that was selected when the user inserted the character.
     */
    void insertChar(QChar character ,const QString &font);

private:
    void initDialog(const QChar &_chr, const QString &_font);
    QChar chr() const;
    QString font() const;
    void closeDialog();

private slots:
    void slotUser1();
    void slotDoubleClicked();

private:
    class Private;
    Private * const d;
};

#endif
