/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KOCOLORCOMBOBOX_H
#define KOCOLORCOMBOBOX_H

#include "koguiutils_export.h"
#include <QtGui/QComboBox>

/**
 * A custom combobox widget for selecting colors.
 */
class KOGUIUTILS_EXPORT KoColorComboBox : public QComboBox
{
    Q_OBJECT

public:
    KoColorComboBox( QWidget * parent = 0 );
    virtual ~KoColorComboBox();

signals:
    /// Is emitted when the color was changed by the user
    void colorChanged( const QColor &color );
    
    /// Is emitted when the user has clicked on the color rect
    void colorApplied( const QColor &color );

public slots:
    /// Sets a new color to be displayed
    void setColor( const QColor &color );

    /// Returns the current color
    QColor color() const;

protected:
    /// reimplemented
    virtual void paintEvent( QPaintEvent * );
    /// reimplemented
    virtual void showPopup();
    /// reimplemented
    virtual void mousePressEvent( QMouseEvent * );

private slots:
    void colorHasChanged( const QColor &color );
    void opacityHasChanged( int opacity );

private:
    class Private;
    Private * const d;
};

#endif // KOCOLORCOMBOBOX_H
