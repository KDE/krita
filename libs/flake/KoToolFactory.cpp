/*
 *  Copyright (c) 2006 Thomas Zander <zander@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "KoToolFactory.h"

KoToolFactory::KoToolFactory(int id, const QString name)
: m_name(name)
, m_id(id)
{
    m_priority=100;
    m_activationId=-1;
}

KoToolFactory::~KoToolFactory()
{
}

int KoToolFactory::toolId() const {
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

const QPixmap& KoToolFactory::icon() const {
    return m_icon;
}

int KoToolFactory::activationShapeId() const {
    return m_activationId;
}

void KoToolFactory::setToolTip(const QString & tooltip) {
    m_tooltip = tooltip;
}

void KoToolFactory::setToolType(const QString & toolType) {
    m_toolType = toolType;
}

void KoToolFactory::setIcon(const QPixmap & icon) {
    m_icon = icon;
}

void KoToolFactory::setPriority(int newPriority) {
    m_priority = newPriority;
}

const KoID KoToolFactory::id() const {
    return KoID(QString::number(m_id), m_name);
}

const QString& KoToolFactory::name() const {
    return m_name;
}

#include "KoToolFactory.moc"
