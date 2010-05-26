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

#include "PageVariableFactory.h"
#include "PageVariable.h"

#include <KoProperties.h>
#include <KoXmlNS.h>

#include <klocale.h>
#include <kdebug.h>

PageVariableFactory::PageVariableFactory(QObject *parent)
        : KoInlineObjectFactoryBase(parent, "page")
{
    KoInlineObjectTemplate var1;
    var1.id = "pagecount";
    var1.name = i18n("Page Count");
    KoProperties *props = new KoProperties();
    props->setProperty("vartype", 1);
    var1.properties = props;
    addTemplate(var1);

    KoInlineObjectTemplate var2;
    var2.id = "pagenumber";
    var2.name = i18n("Page Number");
    props = new KoProperties();
    props->setProperty("vartype", 2);
    var2.properties = props;
    addTemplate(var2);

    //KoInlineObjectTemplate var3;
    //var3.id = "pagecontinuation";
    //var3.name = i18n("Page Continuation");
    //props = new KoProperties();
    //props->setProperty("vartype", 3);
    //var3.properties = props;
    //addTemplate(var3);

    QStringList elementNames;
    elementNames << "page-count" << "page-number" << "page-continuation-string";
    setOdfElementNames(KoXmlNS::text, elementNames);
}

KoInlineObject *PageVariableFactory::createInlineObject(const KoProperties *properties) const
{
    PageVariable *var = new PageVariable();
    if (properties)
        var->readProperties(properties);
    return var;
}
