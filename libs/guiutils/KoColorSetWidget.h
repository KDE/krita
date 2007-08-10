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

#include <QWidgetAction>
#include <QFrame>

#include <koguiutils_export.h>

class KoColor;
class KoColorSet;

/**
 * @short A colormanaged widget for chooosing a color from a colorset
 *
 * KoColorSetWidget is a widget for choosing a color (colormanaged via pigment). Normally it shows an
 * icon and above a small rect of the current color. When the user moves the pointer on top of the widget
 * a larger widget is popped up. This larger widget provides lots of functionallity to choose the color from
 * a colorset.
 */
class KOGUIUTILS_EXPORT KoColorSetWidget : public QFrame
{

    Q_OBJECT

public:

    /**
     * Constructor for the widget, where color is initially blackpoint of sRGB
     *
     * @param parent parent QWidget
     */
    KoColorSetWidget(QWidget *parent=0);

    /**
     * Destructor
     */
    virtual ~KoColorSetWidget();

public slots:

    /**
     * Sets the color of the opposite color. Typically this is the background if this widget controls the
     * text color. Or vice versa. The color is used to calculate the suggestions.
     * @param color the opposite color
     */
    void setOppositeColor(const KoColor &color);

    /**
     * Sets the color set that this widget shows.
     * @param colorSet pointer to the color set
     */
    void setColorSet(KoColorSet *colorSet);

signals:

    /**
     * Emitted every time the color changes (by calling setColor() or
     * by user interaction.
     * @param color the new color
     * @param final if the value is final (ie not produced by the pointer moving over around)
     */
    void colorChanged(KoColor &color, bool final);

protected:
    virtual void hideEvent(QHideEvent *); ///< reimplemented from QComboBox
    virtual void mousePressEvent(QMouseEvent *e); ///< reimplemented from QComboBox
    virtual void changeEvent(QEvent *e); ///< reimplemented from QComboBox

private:
    Q_PRIVATE_SLOT(d, void colorTriggered(KoColorPatch *))
    Q_PRIVATE_SLOT(d, void filter(int))

    class KoColorSetWidgetPrivate;
    KoColorSetWidgetPrivate * const d;
};

#endif
