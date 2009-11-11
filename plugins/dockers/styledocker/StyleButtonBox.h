/* This file is part of the KDE project
   Copyright (C) 2003 Tomislav Lukman <tomislav.lukman@ck.t-com.hr>
   Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>

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

#ifndef STYLEBUTTONBOX_H
#define STYLEBUTTONBOX_H

#include <QtGui/QWidget>

class QButtonGroup;

class StyleButtonBox : public QWidget
{
    Q_OBJECT

public:
    enum StyleButton {
        None     = 1,
        Solid    = 2,
        Gradient = 4,
        Pattern  = 8,
        EvenOdd  = 16,
        Winding  = 32
    };
    Q_DECLARE_FLAGS(StyleButtons, StyleButton)

    explicit StyleButtonBox(QWidget* parent = 0L);
    virtual ~StyleButtonBox();

public slots:
    /// shows specified buttons
    void showButtons(StyleButtons buttons);

signals:
    void buttonPressed(int buttonId);

private:
    class Private;
    Private * const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(StyleButtonBox::StyleButtons)

#endif // STYLEBUTTONBOX_H
