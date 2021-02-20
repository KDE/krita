/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 C. Boemann <cbo@boemann.dk>
 * SPDX-FileCopyrightText: 2007 Fredy Yanardi <fyanardi@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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

private:
    class KoColorPopupActionPrivate;
    KoColorPopupActionPrivate * const d;
};

#endif

