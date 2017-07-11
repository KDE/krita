/* This file is part of the KDE project
   Copyright (c) 2007 C. Boemann <cbo@boemann.dk>
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

#include "kritawidgets_export.h"
#include <KoColorDisplayRendererInterface.h>

class KoColor;
class KoColorSet;

/**
 * @short A colormanaged widget for choosing a color from a colorset
 *
 * KoColorSetWidget is a widget for choosing a color (colormanaged via pigment). It shows a color
 * set plus optionally a checkbox to filter away bad matching colors.
 */
class KRITAWIDGETS_EXPORT KoColorSetWidget : public QFrame
{

    Q_OBJECT

public:

    /**
     * Constructor for the widget, where color is initially blackpoint of sRGB
     *
     * @param parent parent QWidget
     */
    explicit KoColorSetWidget(QWidget *parent=0);

    /**
     * Destructor
     */
    ~KoColorSetWidget() override;

    /**
     * Sets the color set that this widget shows.
     * @param colorSet pointer to the color set
     */
    void setColorSet(KoColorSet *colorSet);
    
    /**
     * @brief setDisplayRenderer
     * Set the display renderer of this object.
     * @param displayRenderer
     */
    void setDisplayRenderer(const KoColorDisplayRendererInterface *displayRenderer);

    /**
     * Gets the current color set
     * @returns current color set,, 0 if none set
     */
    KoColorSet* colorSet();

protected:
    void resizeEvent(QResizeEvent *event) override; ///< reimplemented from QFrame

Q_SIGNALS:

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
    Q_PRIVATE_SLOT(d, void addRemoveColors())
    Q_PRIVATE_SLOT(d, void setColorFromString(QString s))

    class KoColorSetWidgetPrivate;
    KoColorSetWidgetPrivate * const d;
};

#endif
