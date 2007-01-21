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

#include "KoTextBlockData.h"
#include "KoTextBlockBorderData.h"

KoTextBlockData::KoTextBlockData()
    : m_counterPos(0.0, 0.0),
    m_border(0)
{
    m_counterWidth = -1.0;
}

KoTextBlockData::~KoTextBlockData() {
    if(m_border && m_border->removeUser() == 0)
        delete m_border;
}

bool KoTextBlockData::hasCounterData() const {
    return m_counterWidth >= 0 && !m_counterText.isNull();
}

double KoTextBlockData::counterWidth() const {
    return qMax(0., m_counterWidth);
}

void KoTextBlockData::setBorder(KoTextBlockBorderData *border) {
    if(m_border && m_border->removeUser() == 0)
        delete m_border;
    m_border = border;
    if(m_border)
        m_border->addUser();
}
