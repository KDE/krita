/*
 *  Copyright (c) 2019 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KISMOUSECLICKEATER_H
#define KISMOUSECLICKEATER_H

#include <QtGlobal>
#include <QObject>
#include <QElapsedTimer>


class KisMouseClickEater : public QObject
{
public:
    KisMouseClickEater(Qt::MouseButtons buttons,
                       int clicksToEat = 1,
                       QObject *parent = 0);

    ~KisMouseClickEater();

    void reset();
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Qt::MouseButtons m_buttons = Qt::NoButton;
    int m_clicksToEat = 1;
    int m_clicksHappened = 0;
    bool m_blockTimedRelease = false;
    QElapsedTimer m_timeSinceReset;
};

#endif // KISMOUSECLICKEATER_H
