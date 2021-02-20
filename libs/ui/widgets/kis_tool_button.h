/*
 *  SPDX-FileCopyrightText: 2011 José Luis Vergara <pentalis@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TOOL_BUTTON_H
#define KIS_TOOL_BUTTON_H

#include <QToolButton>
#include <kritaui_export.h>

/** This class exists to work around a bug in QToolButton when in
  * MenuPopupButton mode. The correct (mouse) behavior is to display
  * the menu with the first click, and let the user choose an item with
  * the next click. The buggy behavior presented by the tablet is to
  * present the menu and immediately select an item when the user
  * performs a stylus tip click. This workaround solves this.
  */

class KRITAUI_EXPORT KisToolButton : public QToolButton
{
    Q_OBJECT
public:
    explicit KisToolButton(QWidget *parent = 0);

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

private:
    bool m_tabletContact;
};

#endif // KIS_TOOL_BUTTON_H
