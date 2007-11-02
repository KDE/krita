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
#ifndef KODOCKWIDGET_H_
#define KODOCKWIDGET_H_

#include <QtGui/QDockWidget>

class QAbstractButton;

class KoDockWidget : public QDockWidget {
public:
    KoDockWidget(const QString & title, QWidget * parent = 0, Qt::WindowFlags flags = 0);
};

class KoDockWidgetTitleBar : public QWidget {
    Q_OBJECT
public:
    KoDockWidgetTitleBar(QDockWidget* dockWidget);
    virtual ~KoDockWidgetTitleBar();
    
    virtual QSize minimumSizeHint() const; ///< reimplemented from QWidget
    virtual QSize sizeHint () const; ///< reimplemented from QWidget
protected:
    virtual void paintEvent(QPaintEvent* event); ///< reimplemented from QWidget
    virtual void resizeEvent(QResizeEvent* event); ///< reimplemented from QWidget
private:
    Q_PRIVATE_SLOT(d, void toggleFloating())
    Q_PRIVATE_SLOT(d, void toggleCollapsed())
    Q_PRIVATE_SLOT(d, void featuresChanged(QDockWidget::DockWidgetFeatures))

    class Private;
    Private * const d;
};

#endif // KODOCKWIDGET_H_