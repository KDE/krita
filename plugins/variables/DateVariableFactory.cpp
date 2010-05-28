/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#include "DateVariableFactory.h"
#include "DateVariable.h"

#include <KoProperties.h>
#include <KoXmlNS.h>

#include <klocale.h>

DateVariableFactory::DateVariableFactory(QObject *parent)
        : KoInlineObjectFactoryBase(parent, "date", TextVariable)
{
    KoInlineObjectTemplate var;
    var.id = "fixed";
    var.name = i18nc("date that can not be changed later", "Date (Fixed)");
    KoProperties *props = new KoProperties();
    props->setProperty("id", DateVariable::Fixed);
    props->setProperty("definition", "dd/MM/yy");
    var.properties = props;
    addTemplate(var);

    QStringList elementNames;
    elementNames << "date" << "time";
    setOdfElementNames(KoXmlNS::text, elementNames);
}

KoInlineObject *DateVariableFactory::createInlineObject(const KoProperties *properties) const
{
    DateVariable::DateType dt = DateVariable::Fixed;
    if (properties)
        dt = static_cast<DateVariable::DateType>(properties->intProperty("id", dt));

    DateVariable *var = new DateVariable(dt);
    if (properties)
        var->readProperties(properties);
    return var;
}
