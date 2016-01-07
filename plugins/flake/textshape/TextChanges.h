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
#ifndef TextChanges_H
#define TextChanges_H

#include <QMap>

class TextChange;
class QString;

class TextChanges
{
public:
    TextChanges();
    ~TextChanges();
    void inserted(int position, const QString &text);
    void changed(int position, const QString &former, const QString &latter);

    /// return true if the current text and formatting for the parameter section is already in our database
    bool hasText(int position, int length) const;

    const TextChange *first() const
    {
        return m_root;
    }
    QMap<int, const TextChange *> changes() const;

private:
    QMap<int, TextChange *> m_index;
    TextChange *m_root;
};

#endif
