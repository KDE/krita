/* This file is part of the KDE libraries
   Copyright (C) 1999 Ilya Baran (ibaran@mit.edu)

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

#ifndef MATRIXDIALOG_H_INCLUDED
#define MATRIXDIALOG_H_INCLUDED

#include <QWidget>
#include <kdialogbase.h>
#include <QValidator>

#include "kformuladefs.h"

KFORMULA_NAMESPACE_BEGIN

/**
 * Dialog for entering matrix sizes.
 */
class MatrixDialog : public KDialogBase
{
    Q_OBJECT

public:

    int w;
    int h;

    MatrixDialog( QWidget *parent, int width = 3, int height = 3 );
    ~MatrixDialog() {}

protected slots:

    void setWidth(int value);
    void setHeight(int value);
};

KFORMULA_NAMESPACE_END

#endif // MATRIXDIALOG_H_INCLUDED
