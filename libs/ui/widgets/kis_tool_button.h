/*
 *  Copyright (c) 2011 José Luis Vergara <pentalis@gmail.com>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
