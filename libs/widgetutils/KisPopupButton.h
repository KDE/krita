/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
