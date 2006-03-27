/* This file is part of the KDE project
   Copyright (C) 1999 by Dirk A. Mueller <dmuell@gmx.net>
   Copyright (C) 2002 Werner Trobin <trobin@kde.org>

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

#ifndef _COLORACTIONTEST_H
#define _COLORACTIONTEST_H

#include <q3mainwindow.h>

class KoColorPanel;

class TopLevel : public Q3MainWindow
{
    Q_OBJECT
public:
    TopLevel( QWidget* parent = 0, const char* name = 0 );

public slots:
    void insertRandomColor();
    void defaultColors();
    void clearColors();
    void slotColorSelected( const QColor& color );

private:
    KoColorPanel* panel;
};

#endif  // _COLORACTIONTEST_H
