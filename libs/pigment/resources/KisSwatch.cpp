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

#include "KisSwatch.h"

KisSwatch::KisSwatch(const KoColor &color, const QString &name)
    : m_color(color)
    , m_name(name)
    , m_valid(true)
{ }

void KisSwatch::setName(const QString &name)
{
    m_name = name;
    m_valid = true;
}

void KisSwatch::setId(const QString &id)
{
    m_id = id;
    m_valid = true;
}

void KisSwatch::setColor(const KoColor &color)
{
    m_color = color;
    m_valid = true;
}

void KisSwatch::setSpotColor(bool spotColor)
{
    m_spotColor = spotColor;
    m_valid = true;
}
