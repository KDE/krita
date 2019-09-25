/*
 * This file is part of the KDE project
 * Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (c) 2016 L. E. Segovia <leo.segovia@siggraph.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KISSWATCH_H
#define KISSWATCH_H

#include "kritapigment_export.h"
#include <QString>
#include "KoColor.h"

class KRITAPIGMENT_EXPORT KisSwatch
{
public:
    KisSwatch() = default;
    KisSwatch(const KoColor &color, const QString &name = QString());

public:
    QString name() const { return m_name; }
    void setName(const QString &name);

    QString id() const { return m_id; }
    void setId(const QString &id);

    KoColor color() const { return m_color; }
    void setColor(const KoColor &color);

    bool spotColor() const { return m_spotColor; }
    void setSpotColor(bool spotColor);

    bool isValid() const { return m_valid; }

public:
    bool operator==(const KisSwatch& rhs) const {
        return m_color == rhs.m_color && m_name == rhs.m_name;
    }

private:
    KoColor m_color;
    QString m_name;
    QString m_id;
    bool m_spotColor {false};
    bool m_valid {false};
};

#endif // KISSWATCH_H
