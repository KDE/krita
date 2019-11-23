/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_POPUP_BUTTON_H_
#define _KIS_POPUP_BUTTON_H_

#include <QPushButton>

#include <kritawidgetutils_export.h>

/**
 * This class is a convenience class for a button that
 * when clicked displays a popup widget.
 */
class KRITAWIDGETUTILS_EXPORT KisPopupButton : public QPushButton
{

    Q_OBJECT

public:

    KisPopupButton(QWidget* parent);
    ~KisPopupButton() override;

    /**
     * Set the popup widget, the KisPopupButton becomes
     * the owner and parent of the widget.
     */
    void setPopupWidget(QWidget* widget);

    /**
     * This function allow to force the popup to be visible.
     * @param v set to true to force the popup to be visible, set to false
     *          to allow the popup to be hidden
     */
    void setAlwaysVisible(bool v);

    /**
     * Set the width of the popup widget.
     * @return new width of the popup widget
     */
    void setPopupWidgetWidth(int w);

    /**
     * @brief adjustPosition
     * adjusts the position of the popup widget based on the position
     * of this button and the size of the widget
     */
    void adjustPosition();

public Q_SLOTS:

    void showPopupWidget();

    void hidePopupWidget();

protected:
    void paintEvent(QPaintEvent* event) override;
    
    void paintPopupArrow();
private:
    struct Private;
    Private* const m_d;
};

#endif
