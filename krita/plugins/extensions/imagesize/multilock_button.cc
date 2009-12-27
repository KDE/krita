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
#include "multilock_button.h"

#include <QPixmap>
#include <QPainter>
#include <KIcon>
#include <QMouseEvent>

class MultiLockButton::Private {
public:
    Private()
        : lockedPixmap(KIcon("object-locked").pixmap(24,24)),
        unlockedPixmap(KIcon("object-unlocked").pixmap(24,24)),
        locked(true)
    {
    }
    QPixmap lockedPixmap, unlockedPixmap;
    bool locked;
};

MultiLockButton::MultiLockButton(QWidget *parent)
    : QAbstractButton(parent),
    d(new Private())
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
}

MultiLockButton::~MultiLockButton()
{
    delete d;
}

void MultiLockButton::mouseReleaseEvent (QMouseEvent *ev) {
    if (! isEnabled() || ev->button() != Qt::LeftButton)
        return;
    if (d->locked) {
        //TODO warn user
    }
    else
        lock();
}

void MultiLockButton::lock() {
    d->locked = true;
    update();
    emit lockStateChanged(d->locked);
}

void MultiLockButton::unlock() {
    d->locked = false;
    update();
    emit lockStateChanged(d->locked);
}

void MultiLockButton::slotSibilingChange(bool siblingLocked) {
    if (siblingLocked)
        unlock();
}

void MultiLockButton::nominateSiblings(MultiLockButton *sibling1, MultiLockButton *sibling2) {
    connect(this, SIGNAL(lockStateChanged(bool)), sibling1, SLOT(slotSibilingChange(bool)));
    connect(this, SIGNAL(lockStateChanged(bool)), sibling2, SLOT(slotSibilingChange(bool)));
}

void MultiLockButton::paintEvent (QPaintEvent *) {
    QPainter painter(this);
    painter.drawPixmap(0, 0, 24, 24, d->locked ? d->lockedPixmap : d->unlockedPixmap);
    painter.end();
}

QSize MultiLockButton::sizeHint () const {
    return QSize(24, 24);
}

bool MultiLockButton::isLocked() const
{
    return d->locked;
}

#include "multilock_button.moc"
