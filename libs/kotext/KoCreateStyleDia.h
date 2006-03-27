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

#ifndef __KoCreateStyleDia__
#define __KoCreateStyleDia__

#include <kdialogbase.h>
#include <qstringlist.h>
#include <koffice_export.h>

class QLineEdit;
class KOTEXT_EXPORT KoCreateStyleDia : public KDialogBase
{
    Q_OBJECT
public:
    KoCreateStyleDia( const QStringList & _list, QWidget *parent, const char *name );
    QString nameOfNewStyle()const;

protected slots:
    virtual void slotOk();
    void nameChanged( const QString &text);

protected:
    QStringList styleList;
    QLineEdit *m_styleName;
};

#endif
