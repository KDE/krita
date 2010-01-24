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

#include "ActionHelper.h"
#include <QAction>

ActionHelper::ActionHelper(QObject *parent, QAction *original, QAction *shadow, bool partOfGroup)
    : QObject(parent), m_original(original), m_shadow(shadow),
    m_blockSignals(false), m_partOfGroup(partOfGroup)
{
    connect(original, SIGNAL(toggled(bool)), this, SLOT(originalTriggered(bool)));
    connect(shadow, SIGNAL(triggered(bool)), this, SLOT(shadowTriggered(bool)));
}

void ActionHelper::originalTriggered(bool on) {
    if (m_blockSignals)
        return;
    if ((m_shadow->isCheckable() && m_shadow->isChecked() == on) || (!on && m_partOfGroup))
        return;
    m_blockSignals = true;
    m_shadow->trigger();
    m_blockSignals = false;
}

void ActionHelper::shadowTriggered(bool on) {
    if (m_blockSignals) return;
    if (m_original->isCheckable() && m_original->isChecked() == on)
        return;
    if (m_shadow->isCheckable() && !on && m_partOfGroup)
        return;
    m_blockSignals = true;
    m_original->trigger();
    m_blockSignals = false;
}

#include <ActionHelper.moc>

