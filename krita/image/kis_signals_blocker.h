/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_SIGNALS_BLOCKER_H
#define __KIS_SIGNALS_BLOCKER_H

/**
 * Block QObject's signals in a safe and sane way.
 *
 * Avoid using direct calls to QObject::blockSignals(bool),
 * because:
 *
 * 1) They are not safe. One beautifully sunny day someone (it might
 *    easily be you youself) will forget about these call and will put
 *    a 'return' statement somewhere among the lines. Surely this is
 *    not what you expect to happen.
 *
 * 2) Two lines of blocking for every line of access can easily make
 *    the code unreadable.
 */

class KisSignalsBlocker
{
public:
    KisSignalsBlocker(QObject *object)
        : m_object(object)
    {
        m_object->blockSignals(true);
    }

    ~KisSignalsBlocker()
    {
        m_object->blockSignals(false);
    }

private:
    Q_DISABLE_COPY(KisSignalsBlocker);

private:
    QObject *m_object;
};

#endif /* __KIS_SIGNALS_BLOCKER_H */
