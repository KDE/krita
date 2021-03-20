/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SIMPLETOUCHAREA_H
#define SIMPLETOUCHAREA_H

#include <QQuickItem>

class SimpleTouchArea : public QQuickItem
{
    Q_OBJECT
public:
    explicit SimpleTouchArea(QQuickItem* parent = 0);
    virtual ~SimpleTouchArea();

Q_SIGNALS:
    void touched();

protected:
    virtual bool event(QEvent* event);
    virtual void touchEvent(QTouchEvent* event);
};

#endif // SIMPLETOUCHAREA_H
