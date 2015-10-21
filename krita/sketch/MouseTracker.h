/* This file is part of the KDE project
 * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
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

#ifndef MOUSETRACKER_H
#define MOUSETRACKER_H

#include <QObject>
#include <QPointF>

class QQuickItem;

/**
 * Helper class for tracking global mouse position from within QML.
 */
class MouseTracker : public QObject
{
    Q_OBJECT
public:
    explicit MouseTracker(QObject* parent = 0);
    virtual ~MouseTracker();

public Q_SLOTS:
    void addItem(QQuickItem* item, const QPointF& offset = QPointF());
    void removeItem(QQuickItem* item);

protected:
    virtual bool eventFilter(QObject* target, QEvent* event);

private:
private:
    class Private;
    Private* const d;
};

#endif // MOUSETRACKER_H
