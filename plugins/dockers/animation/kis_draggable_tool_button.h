/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_DRAGGABLE_TOOL_BUTTON_H
#define __KIS_DRAGGABLE_TOOL_BUTTON_H

#include <QToolButton>


class KisDraggableToolButton : public QToolButton
{
    Q_OBJECT
public:
    KisDraggableToolButton(QWidget *parent);
    ~KisDraggableToolButton() override;

    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;

    static int unitRadius();

    void beginDrag(const QPoint &pos);
    int continueDrag(const QPoint &pos);

Q_SIGNALS:
    void valueChanged(int value);

private:
    Qt::Orientation m_orientation;
    QPoint m_startPoint;
};

#endif /* __KIS_DRAGGABLE_TOOL_BUTTON_H */
