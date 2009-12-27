/* This file is part of the KDE project
 * Copyright (C) 2009 Casper Boemann <cbo@boemann.dk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef MULTILOCKBUTTON_H
#define MULTILOCKBUTTON_H

#include <QAbstractButton>

/**
 * This button is a sort of radiobutton
 * Contrary to a normal radiobutton this shows a lock icon and no label.
 * It also differs from a radiobutton in that it is the locked state that is mutually exclusive.
 * It is also special in that it is designed for a group of 3 buttons
 */
class MultiLockButton : public QAbstractButton {
    Q_OBJECT
public:
    /// constructor
    MultiLockButton(QWidget *parent);
    virtual ~MultiLockButton();

    /// Returns state of lock
    bool isLocked() const;

    void nominateSiblings(MultiLockButton *sibling1, MultiLockButton *sibling2);

public slots:
    /**
     * Make the button locked.
     * There is no public unlock method. Unlocking must be done by locking a sibling
     * This also emits the lockStateChanged if the value has changed.
     */
    void lock();

    void slotSibilingChange(bool siblingLocked);

signals:
    /**
     * This signal is emitted every time the button changes value, either by user interaction or
     * by programetically setting it.
     */
    void lockStateChanged(bool keep);

protected:
    /// reimplemented
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void paintEvent (QPaintEvent *);
    virtual QSize sizeHint () const;

    void unlock();

private:
    class Private;
    Private * const d;
};

#endif
