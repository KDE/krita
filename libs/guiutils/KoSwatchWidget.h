/* This file is part of the KDE project
   Copyright (c) 2007 Casper Boemann <cbr@boemann.dk>

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

#ifndef KOSWATCHWIDGET_H_
#define KOSWATCHWIDGET_H_

#include <QPushButton>

#include <koguiutils_export.h>

class KoColor;

/**
 * @short A colormanaged widget for chooosing a color from a swatch
 *
 * KoSwatchWidget is a widget for choosing a color (colormanaged via pigment). Normally it shows an
 * icon and above a small rect of the current color. When the user moves the pointer on top of the widget
 * a larger widget is popped up. This larger widget provides lots of functionallity to choose the color from
 * a swatch.
 */
class KOGUIUTILS_EXPORT KoSwatchWidget : public QPushButton
{

    Q_OBJECT

public:

    /**
     * Constructor for the widget, where color is initially blackpoint of sRGB
     *
     * @param parent parent QWidget
     */
    KoSwatchWidget(QWidget *parent=0);

    /**
     * Destructor
     */
    virtual ~KoSwatchWidget();

public slots:

signals:

    /**
     * Emitted every time the color changes (by calling setColor() or
     * by user interaction.
     * @param color the new color
     * @param final if the value is final (ie not produced by the pointer moving over around)
     */
    void colorChanged(KoColor &color, bool final);

protected:
    virtual void enterEvent(QEvent *); ///< reimplemented from QComboBox
    virtual void hideEvent(QHideEvent *); ///< reimplemented from QComboBox
    virtual void changeEvent(QEvent *e); ///< reimplemented from QComboBox

private:
//    Q_PRIVATE_SLOT(d, void sliderValueChanged(int value))

    class KoSwatchWidgetPrivate;
    KoSwatchWidgetPrivate * const d;
};

#endif
