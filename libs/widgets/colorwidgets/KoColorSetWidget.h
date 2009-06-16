/* This file is part of the KDE project
   Copyright (c) 2007 Casper Boemann <cbr@boemann.dk>
   Copyright (c) 2007 Fredy Yanardi <fyanardi@gmail.com>

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

#ifndef KOCOLORSETWIDGET_H_
#define KOCOLORSETWIDGET_H_

#include <QWidgetAction>
#include <QFrame>

#include "kowidgets_export.h"

class KoColor;
class KoColorSet;

/**
 * @short A colormanaged widget for choosing a color from a colorset
 *
 * KoColorSetWidget is a widget for choosing a color (colormanaged via pigment). It shows a a color
 * set plus optionally a checkbox to filter away bad matching colors.
 * Some ways to add and remove plus choose another colorset will be added in the future.
 */
class KOWIDGETS_EXPORT KoColorSetWidget : public QFrame
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

    /**
     * Add a color  to the list of recent colors. This method is useful when the user selects a color
     * from other interface or dialog.
     * @param color the color to be added to the list of recent colors.
     */
    void addRecentColor(const KoColor &color);

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

protected:
    virtual void resizeEvent(QResizeEvent *event); ///< reimplemented from QFrame

signals:

    /**
     * Emitted every time the color changes (by calling setColor() or
     * by user interaction.
     * @param color the new color
     * @param final if the value is final (ie not produced by the pointer moving over around)
     */
    void colorChanged(const KoColor &color, bool final);

    /**
     * Emitted every time the size of this widget changes because of new colorset with
     * different number of colors is loaded. This is useful for KoColorSetAction to update
     * correct size of the menu showing this widget.
     * @param size the new size
     */
    void widgetSizeChanged(const QSize &size);

private:
    Q_PRIVATE_SLOT(d, void colorTriggered(KoColorPatch *))
    Q_PRIVATE_SLOT(d, void filter(int))
    Q_PRIVATE_SLOT(d, void addRemoveColors())

    class KoColorSetWidgetPrivate;
    KoColorSetWidgetPrivate * const d;
};

#endif
