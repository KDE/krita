/* This file is part of the KDE project
   Copyright (c) 2007 Marijn Kruisselbrink <mkruisselbrink@kde.org>

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

#include "kritawidgets_export.h"
#include <QAbstractButton>

class QEvent;
class QPaintEvent;

/**
 * @short A custom title bar button for dock widgets.
 *
 * Used in KoDockWidgetTitleBar but can be also used for similar
 * purposes inside other parents.
 */
class KRITAWIDGETS_EXPORT KoDockWidgetTitleBarButton : public QAbstractButton
{
    Q_OBJECT

public:
    explicit KoDockWidgetTitleBarButton(QWidget *parent = 0);
    ~KoDockWidgetTitleBarButton() override;

    QSize sizeHint() const override; ///< reimplemented from QWidget
    QSize minimumSizeHint() const override; ///< reimplemented from QWidget

protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    class Private;
    Private * const d;
};

#endif // KODOCKWIDGETTITLEBARBUTTON_H_
