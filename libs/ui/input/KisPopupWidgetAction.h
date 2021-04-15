/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 * SPDX-FileCopyrightText: 2021 Emmet O 'Neill <emmetoneill.pdx@gmail.com>
 * SPDX-FileCopyrightText: 2021 Eoin O 'Neill <eoinoneill1991@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_POPUP_WIDGET_ACTION_H
#define KIS_POPUP_WIDGET_ACTION_H

#include "kis_abstract_input_action.h"

#include <QObject>
#include <QPointer>
#include <QWidget>
#include <QMainWindow>

#include "kis_global.h"
#include "kis_debug.h"
#include "kis_canvas2.h"
#include "KisPopupWidgetInterface.h"
class QMenu;


/**
 * \brief Get the current tool's popup widget and display it.
 */
class KisPopupWidgetAction : public QObject, public KisAbstractInputAction
{
    Q_OBJECT

public:
    explicit KisPopupWidgetAction();
    ~KisPopupWidgetAction() override;

    int priority() const override {return 1;}

    void begin(int, QEvent *) override;

private:
    bool m_requestedWithStylus;
};

#endif // KIS_POPUP_WIDGET_ACTION_H
