/*
 *  SPDX-FileCopyrightText: 2021 Emmet O 'Neill <emmetoneill.pdx@gmail.com>
 *  SPDX-FileCopyrightText: 2021 Eoin O 'Neill <eoinoneill1991@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISPOPUPWIDGETINTERFACE_H
#define KISPOPUPWIDGETINTERFACE_H

#include <QWidget>
#include <QGridLayout>

/**
 * @brief The PopupWidgetInterface abstract class defines
 * the basic interface that will be used by all popup widgets.
 *
 * Classes that implement this interface should use `Q_INTERFACES(KisPopupWidgetInterface)`!
 * This is needed in order to include signals in the interface.
 */
class KisPopupWidgetInterface {
public:
    virtual ~KisPopupWidgetInterface() {}

    /**
     * @brief Called when and where you want a widget to popup.
     */
    virtual void popup(const QPoint& position) = 0;

    /**
     * @brief Returns whether the widget is active (on screen) or not.
     */
    virtual bool onScreen() = 0;

    /**
     * @brief Called when you want to dismiss a popup widget.
     */
    virtual void dismiss() = 0;

Q_SIGNALS:
    /**
     * @brief Emitted when a popup widget believes that its job is finished.
     */
    virtual void finished() = 0;
};

Q_DECLARE_INTERFACE(KisPopupWidgetInterface, "KisPopupWidgetInterface")

#endif // KISPOPUPWIDGETINTERFACE_H
