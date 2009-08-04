/* This file is part of the KDE project
 * Copyright (C) 2009 Elvis Stansvik <elvstone@gmail.com>
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

#include "KoTableFormat_p.h"

#include <QVariant>

KoTableFormatPrivate::KoTableFormatPrivate()
{
}

KoTableFormatPrivate::KoTableFormatPrivate(const KoTableFormatPrivate &rhs) :
    QSharedData(rhs), m_stylesPrivate(rhs.m_stylesPrivate)
{
}

QVariant KoTableFormatPrivate::property(int propertyId) const
{
    return m_stylesPrivate.value(propertyId);
}

void KoTableFormatPrivate::setProperty(int propertyId, const QVariant &value)
{
    m_stylesPrivate.add(propertyId, value);
}

bool KoTableFormatPrivate::hasProperty(int propertyId) const
{
    return m_stylesPrivate.contains(propertyId);
}

void KoTableFormatPrivate::clearProperty(int propertyId)
{
    m_stylesPrivate.remove(propertyId);
}

