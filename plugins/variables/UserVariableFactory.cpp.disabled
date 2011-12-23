/* This file is part of the KDE project
 * Copyright (C) 2011 Sebastian Sauer <mail@dipe.org>
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

#include "UserVariableFactory.h"
#include "UserVariable.h"

#include <KoProperties.h>
#include <KoXmlNS.h>

#include <klocale.h>
#include <kdebug.h>

UserVariableFactory::UserVariableFactory()
    : KoInlineObjectFactoryBase("user", TextVariable)
{
    KoInlineObjectTemplate var1;
    var1.id = "userfieldget";
    var1.name = i18n("Custom");
    KoProperties *props = new KoProperties();
    props->setProperty("varproperty", KoInlineObject::UserGet);
    props->setProperty("varname", QString());
    var1.properties = props;
    addTemplate(var1);

    /*
    KoInlineObjectTemplate var2;
    var2.id = "userfieldinput";
    var2.name = i18n("User Input");
    props = new KProperties();
    props->setProperty("varproperty", KoInlineObject::UserField);
    var2.properties = props;
    addTemplate(var2);
    */

    QStringList elementNames;
    elementNames << "user-field-get" << "user-field-input";
    setOdfElementNames(KoXmlNS::text, elementNames);
}

KoInlineObject *UserVariableFactory::createInlineObject(const KoProperties *properties) const
{
    UserVariable *var = new UserVariable();
    if (properties)
        var->readProperties(properties);
    return var;
}
