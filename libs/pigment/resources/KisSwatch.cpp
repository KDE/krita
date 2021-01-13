/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2005 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2016 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
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

KisSwatch KisSwatch::fromByteArray(QByteArray &data)
{

}
