/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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
#ifndef TEXTCHANGE_H
#define TEXTCHANGE_H

#include <QString>

class TextChange
{
public:
    TextChange();
    int formerLength() const;
    int length() const;
    int formerPosition() const;
    int position() const;

    TextChange *next()
    {
        return m_next;
    }
    const TextChange *next() const
    {
        return m_next;
    }
    TextChange *previous()
    {
        return m_previous;
    }
    const TextChange *previous() const
    {
        return m_previous;
    }

    QString before() const
    {
        return m_before;
    }
    QString after() const
    {
        return m_after;
    }

    void setPosition(int pos);
    void setOldText(const QString &old);
    void setNewText(const QString &current);
    void setPrevious(TextChange *item);
    void setNext(TextChange *item);
    void move(int length);

    void insertBefore(TextChange *node);
    void insertAfter(TextChange *node);

    void merge(TextChange *other);

private:
    QString m_before, m_after;
    int m_formerPosition, m_position;
    TextChange *m_previous, *m_next;
};

#endif
