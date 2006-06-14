/* This file is part of the KDE project
   Copyright (C)  2002 Montel Laurent <lmontel@mandrakesoft.com>

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

#ifndef __KoCommentDia__
#define __KoCommentDia__

#include <kdialog.h>
#include <koffice_export.h>
class QPushButton;
class Q3MultiLineEdit;

class KOTEXT_EXPORT KoCommentDia : public KDialog
{
    Q_OBJECT
public:
    KoCommentDia( QWidget *parent, const QString &_note=QString::null, const QString & _authorName=QString::null, const QString &_createNote=QString::null, const char *name=0L );
    QString commentText();

private slots:
    void slotAddAuthorName();
    void slotTextChanged( );

protected:
    Q3MultiLineEdit *m_multiLine;
    QString authorName;
    QPushButton *pbAddAuthorName;
};

#endif
