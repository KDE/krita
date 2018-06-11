/* This file is part of the KDE project
 * Copyright (c) 2007 C. Boemann <cbo@boemann.dk>
 * Copyright (C) 2007 Fredy Yanardi <fyanardi@gmail.com>
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

#ifndef KOCOLORPOPUPACTION_H
#define KOCOLORPOPUPACTION_H

#include <QAction>

#include "kritawidgets_export.h"

class KoColor;

/**
 * KoColorPopupAction makes use of KoColorSetWidget to show a widget for for choosing a color (colormanaged via pigment).
 * @see KoColorPopupAction
 */

class KRITAWIDGETS_EXPORT KoColorPopupAction : public QAction
{
    Q_OBJECT

public:
    /**
      * Constructs a KoColorPopupAction with the specified parent.
      *
      * @param parent The parent for this action.
      */
    explicit KoColorPopupAction(QObject *parent = 0);

    /**
     * Destructor
     */
    ~KoColorPopupAction() override;

public Q_SLOTS:
    /// Sets a new color to be displayed
    void setCurrentColor( const QColor &color );

    /// Sets a new color to be displayed
    void setCurrentColor( const KoColor &color );

    /// Returns the current color
    QColor currentColor() const;

    /// Returns the current color as a KoColor
    KoColor currentKoColor() const;

    /// update the icon - only needed if you resize the iconsize in the widget that shows the action
    void updateIcon();

Q_SIGNALS:
    /**
     * Emitted every time the color changes (by calling setColor() or
     * by user interaction.
     * @param color the new color
     */
    void colorChanged(const KoColor &color);

private Q_SLOTS:
    void emitColorChanged();
    void colorWasSelected(const KoColor &color, bool final);
    void colorWasEdited( const QColor &color );
    void opacityWasChanged( int opacity );
    void slotTriggered(bool);

private:
    class KoColorPopupActionPrivate;
    KoColorPopupActionPrivate * const d;
};

#endif

