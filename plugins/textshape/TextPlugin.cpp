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
#include "TextPlugin.h"
#include "TextToolFactory.h"
#include "ChangeTrackingToolFactory.h"
#include "TextShapeFactory.h"

#include <KoShapeRegistry.h>
#include <KoToolRegistry.h>

#include <kgenericfactory.h>

K_EXPORT_COMPONENT_FACTORY(textshape, KGenericFactory<TextPlugin>("TextShape"))

TextPlugin::TextPlugin(QObject * parent, const QStringList &)
        : QObject(parent)
{
    KoToolRegistry::instance()->add(new TextToolFactory());
    KoToolRegistry::instance()->add(new ChangeTrackingToolFactory());
    KoShapeRegistry::instance()->add(new TextShapeFactory());
}

#include <TextPlugin.moc>
