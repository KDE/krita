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

#include "KoNamedVariable.h"
#include "KoInlineTextObjectManager.h"
#include <KoProperties.h>

KoNamedVariable::KoNamedVariable(Property key, const QString &name)
        : KoVariable(true),
        m_name(name),
        m_key(key)
{
}

void KoNamedVariable::propertyChanged(Property property, const QVariant &value)
{
    if (property == m_key)
        setValue(qvariant_cast<QString>(value));
}

void KoNamedVariable::setup()
{
    setValue(manager()->stringProperty(m_key));
}

bool KoNamedVariable::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    Q_UNUSED(element);
    Q_UNUSED(context);
    // TODO
    return false;
}

void KoNamedVariable::saveOdf(KoShapeSavingContext &context)
{
    Q_UNUSED(context);
    // TODO
}
