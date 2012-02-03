/* This file is part of the KDE project
 * Copyright (C) 2007 Pierre Ducroquet <pinaraf@gmail.com>
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

#include "InfoVariableFactory.h"

#include <KoProperties.h>
#include <KoXmlNS.h>
#include <kdebug.h>
#include <klocale.h>

#include <QStringList>

#include "InfoVariable.h"

InfoVariableFactory::InfoVariableFactory()
        : KoInlineObjectFactoryBase("info", TextVariable)
{
    KoInlineObjectTemplate var1;
    var1.id = "author";
    var1.name = i18n("Author Name");
    KoProperties *props = new KoProperties();
    props->setProperty("vartype", KoInlineObject::AuthorName);
    var1.properties = props;
    addTemplate(var1);

    KoInlineObjectTemplate var2;
    var2.id = "title";
    var2.name = i18n("Title");
    props = new KoProperties();
    props->setProperty("vartype", KoInlineObject::Title);
    var2.properties = props;
    addTemplate(var2);

    KoInlineObjectTemplate var3;
    var3.id = "subject";
    var3.name = i18n("Subject");
    props = new KoProperties();
    props->setProperty("vartype", KoInlineObject::Subject);
    var3.properties = props;
    addTemplate(var3);

    KoInlineObjectTemplate var4;
    var4.id = "file-name";
    var4.name = i18n("File Name");
    props = new KoProperties();
    props->setProperty("vartype", KoInlineObject::DocumentURL);
    var4.properties = props;
    addTemplate(var4);

    KoInlineObjectTemplate var5;
    var5.id = "keywords";
    var5.name = i18n("Keywords");
    props = new KoProperties();
    props->setProperty("vartype", KoInlineObject::Keywords);
    var5.properties = props;
    addTemplate(var5);

    KoInlineObjectTemplate var6;
    var6.id = "comments";
    var6.name = i18n("Comments");
    props = new KoProperties();
    props->setProperty("vartype", KoInlineObject::Comments);
    var6.properties = props;
    addTemplate(var6);

    QStringList elementNames(InfoVariable::tags());
    setOdfElementNames(KoXmlNS::text, elementNames);
}

KoInlineObject *InfoVariableFactory::createInlineObject(const KoProperties *properties) const
{
    InfoVariable *var = new InfoVariable();
    if (properties)
        var->readProperties(properties);
    return var;
}
