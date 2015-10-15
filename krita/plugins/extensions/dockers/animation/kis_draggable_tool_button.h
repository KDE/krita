/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_DRAGGABLE_TOOL_BUTTON_H
#define __KIS_DRAGGABLE_TOOL_BUTTON_H

#include <QToolButton>


class KisDraggableToolButton : public QToolButton
{
    Q_OBJECT
public:
    KisDraggableToolButton(QWidget *parent);
    ~KisDraggableToolButton();

    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);

    static int unitRadius();

    int calculateValue(const QPoint &diff);

Q_SIGNALS:
    void valueChanged(int value);

private:
    Qt::Orientation m_orientation;
    QPoint m_startPoint;
};

#endif /* __KIS_DRAGGABLE_TOOL_BUTTON_H */
