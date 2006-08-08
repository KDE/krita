/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; version 2.
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

#include <QVariant>
#include <QVector>

struct Property {
    Property() {}
    Property(int key, const QVariant &value) {
        this->key = key;
        this->value = value;
    }
    inline bool operator==(const Property &prop) const {
        return prop.key == key && prop.value == value;
    }
    inline bool operator!=(const Property &prop) const {
        return prop.key != key || prop.value != value;
    }

    int key;
    QVariant value;
};

class StylePrivate {
public:
    StylePrivate();
    ~StylePrivate();

    void add(int key, const QVariant &value);
    void remove(int key);
    const QVariant *get(int key) const;
    bool contains(int key) const;
    void copyMissing(const StylePrivate *other);
    void removeDuplicates(const StylePrivate *other);

private:
    QVector<Property> m_properties;
};

