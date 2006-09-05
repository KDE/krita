/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#include "KoToolFactory.h"

KoToolFactory::KoToolFactory(QObject *parent, const QString &id, const QString &name)
: QObject(parent)
, m_name(name)
, m_id(id)
{
    m_priority=100;
}

KoToolFactory::~KoToolFactory()
{
}

const QString &KoToolFactory::toolId() const {
    return m_id;
}

int KoToolFactory::priority() const {
    return m_priority;
}

const QString& KoToolFactory::toolType() const {
    return m_toolType;
}

const QString& KoToolFactory::toolTip() const {
    return m_tooltip;
}

const QString& KoToolFactory::icon() const {
    return m_icon;
}

const QString &KoToolFactory::activationShapeId() const {
    return m_activationId;
}

void KoToolFactory::setActivationShapeID(const QString &activationShapeId) {
    m_activationId = activationShapeId;
}

void KoToolFactory::setToolTip(const QString & tooltip) {
    m_tooltip = tooltip;
}

void KoToolFactory::setToolType(const QString & toolType) {
    m_toolType = toolType;
}

void KoToolFactory::setIcon(const QString & icon) {
    m_icon = icon;
}

void KoToolFactory::setPriority(int newPriority) {
    m_priority = newPriority;
}

const KoID KoToolFactory::id() const {
    return KoID(m_id, m_name);
}

const QString& KoToolFactory::name() const {
    return m_name;
}

#include "KoToolFactory.moc"
