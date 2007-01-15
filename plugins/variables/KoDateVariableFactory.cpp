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

#include "KoDateVariableFactory.h"
#include "KoDateVariable.h"

#include <KoProperties.h>

#include <klocale.h>

KoDateVariableFactory::KoDateVariableFactory(QObject *parent)
    : KoInlineObjectFactory(parent, "date")
{
    KoInlineObjectTemplate var;
    var.id = "fixed";
    var.name = i18n("Fixed");
    KoProperties *props = new KoProperties();
    props->setProperty("id", KoDateVariable::Fixed);
    var.properties = props;
    addTemplate(var);
}

KoInlineObject *KoDateVariableFactory::createInlineObject(const KoProperties *properties) const {
    KoDateVariable *var = new KoDateVariable(static_cast<KoDateVariable::DateType> (properties->getProperty("id").toInt()));
    var->setProperties(properties);
    return var;
}
