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

#ifndef SHADOWCONFIGWIDGET_H
#define SHADOWCONFIGWIDGET_H

#include "kowidgets_export.h"
#include <QtGui/QWidget>

class KoUnit;
class KoColor;

/// A widget for configuring the shadow of a shape
class KOWIDGETS_EXPORT KoShadowConfigWidget : public QWidget
{
    Q_OBJECT
public:
    KoShadowConfigWidget( QWidget * parent );
    ~KoShadowConfigWidget();

    /// Sets the shadow color
    void setShadowColor( const QColor &color );

    /// Returns the shadow color
    QColor shadowColor() const;

    /// Sets the shadow offset
    void setShadowOffset( const QPointF &offset );

    /// Returns the shadow offset
    QPointF shadowOffset() const;

    /// Sets if the shadow is visible
    void setShadowVisible( bool visible );

    /// Returns if shadow is visible
    bool shadowVisible() const;

public slots:
    void setUnit( const KoUnit &unit );

signals:
    /// Is emitted whenever the shadow color has changed
    void shadowColorChanged( const KoColor &color );

    /// Is emitted whenever the shadow offset has changed
    void shadowOffsetChanged( const QPointF &offset );

    /// Is emitted whenever the shadow visibility has changed
    void shadowVisibilityChanged( bool visible );

private slots:
    void visibilityChanged();
    void offsetChanged();
private:
    class Private;
    Private * const d;
};

#endif // SHADOWCONFIGWIDGET_H
