/* This file is part of the KDE project
 * Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#include "KoShapeFactory.h"

KoShapeFactory::KoShapeFactory(QObject *parent, const QString id, const QString name)
: QObject(parent)
, m_id(id)
, m_name(name)
{
}

const KoID KoShapeFactory::id() const {
    return KoID(m_id, m_name);
}

const QString & KoShapeFactory::toolTip() const {
    return m_tooltip;
}

const QPixmap & KoShapeFactory::icon() const {
    return m_icon;
}

const QString& KoShapeFactory::name() const {
    return m_name;
}

void KoShapeFactory::addTemplate(KoProperties * params) {
    m_templates.append(params);
}

void KoShapeFactory::setToolTip(const QString & tooltip) {
    m_tooltip = tooltip;
}

void KoShapeFactory::setIcon(const QPixmap & icon) {
    m_icon = icon;
}

const QString &KoShapeFactory::shapeId() const {
    return m_id;
}

#include "KoShapeFactory.moc"
