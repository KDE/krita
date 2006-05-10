/* This file is part of the KDE project
   Copyright (C)  2001,2002,2003 Montel Laurent <lmontel@mandrakesoft.com>

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

#ifndef __kofonttab_h__
#define __kofonttab_h__

#include <kofonttabbase.h>
#include <QFont>

class KoFontTab : public KoFontTabBase
{
    Q_OBJECT

public:
    KoFontTab( uint fontListCriteria=0, QWidget* parent=0, const char* name=0, Qt::WFlags fl=0 );
    ~KoFontTab();

    QFont getSelection();

public slots:
    void setSelection( const QFont &font );

signals:
    void familyChanged();
    void boldChanged();
    void italicChanged();
    void sizeChanged();

private:
    QFont comparisonFont;

private slots:
    void slotFontChanged( const QFont &font );
};

#endif
