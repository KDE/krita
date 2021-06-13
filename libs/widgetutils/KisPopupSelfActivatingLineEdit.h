/*
 * SPDX-FileCopyrightText: 2021 Alvin Wong <alvin@alvinhc.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef KIS_POPUP_SELF_ACTIVATING_LINE_EDIT_H
#define KIS_POPUP_SELF_ACTIVATING_LINE_EDIT_H

#include <QLineEdit>

#include <kritawidgetutils_export.h>


// HACK: This QLineEdit calls `QWidget::activateWindow` on focus if it is
// housed inside a Qt::Popup, in order to work around a bug causing input
// methods to not work in it.
// See https://bugs.kde.org/show_bug.cgi?id=395598
class KRITAWIDGETUTILS_EXPORT KisPopupSelfActivatingLineEdit : public QLineEdit
{
    Q_OBJECT
    
public:
    KisPopupSelfActivatingLineEdit(QWidget *parent = nullptr);
    virtual ~KisPopupSelfActivatingLineEdit();

protected:
    void focusInEvent(QFocusEvent *e) override;
};

#endif // KIS_POPUP_SELF_ACTIVATING_LINE_EDIT_H
