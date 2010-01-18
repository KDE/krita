/* This file is part of the KDE project
   Copyright (c) 2007 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>

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
   Boston, MA 02110-1301, USA.
*/
#ifndef KODOCKWIDGETTITLEBARBUTTON_H_
#define KODOCKWIDGETTITLEBARBUTTON_H_

#include "kowidgets_export.h"
#include <QtGui/QAbstractButton>

class QEvent;
class QPaintEvent;

/**
 * @short A custom title bar button for dock widgets.
 *
 * Used in KoDockWidgetTitleBarButton but can be also used for similar
 * purposes inside other parents.
 */
class KOWIDGETS_EXPORT KoDockWidgetTitleBarButton : public QAbstractButton
{
public:
    KoDockWidgetTitleBarButton(QWidget *parent = 0);
    ~KoDockWidgetTitleBarButton();

    QSize sizeHint() const; ///< reimplemented from QWidget
    QSize minimumSizeHint() const; ///< reimplemented from QWidget

protected:
    virtual void enterEvent(QEvent *event);
    virtual void leaveEvent(QEvent *event);
    virtual void paintEvent(QPaintEvent *event);

private:
    class Private;
    Private * const d;
};

#endif // KODOCKWIDGETTITLEBARBUTTON_H_
