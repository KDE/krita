/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
