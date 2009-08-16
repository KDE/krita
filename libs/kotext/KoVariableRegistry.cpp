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

#include "KoVariableRegistry.h"
#include "KoVariableFactory.h"
#include "KoVariable.h"
#include "InsertVariableAction_p.h"

#include <KoXmlReader.h>
#include <KoCanvasBase.h>
#include <KoPluginLoader.h>
#include <kglobal.h>
#include <kdebug.h>

class KoVariableRegistry::Singleton
{
public:
    Singleton()
            : initDone(false) {}
    ~Singleton() {}

    KoVariableRegistry q;
    bool initDone;
};

K_GLOBAL_STATIC(KoVariableRegistry::Singleton, singleton)

class KoVariableRegistry::Private
{
public:
    QHash<QPair<QString, QString>, KoVariableFactory *> factories;
};

KoVariableRegistry::KoVariableRegistry()
        : d(new Private())
{
}

KoVariableRegistry::~KoVariableRegistry()
{
    delete(d);
}

void KoVariableRegistry::init()
{
    KoPluginLoader::PluginsConfig config;
    config.whiteList = "TextVariablePlugins";
    config.blacklist = "TextVariablePluginsDisabled";
    config.group = "koffice";
    KoPluginLoader::instance()->load(QString::fromLatin1("KOffice/Text-Variable"),
                                     QString::fromLatin1("[X-KoText-MinVersion] <= 0"), config);

    QList<KoVariableFactory*> factories = values();
    kDebug(32500) << "factories count" << factories.size();
    foreach(KoVariableFactory *factory, factories) {
        kDebug(32500) << factory->id();
        QString nameSpace = factory->odfNameSpace();
        if (nameSpace.isEmpty() || factory->odfElementNames().isEmpty()) {
            kDebug(32500) << "Variable factory" << factory->id() << " does not have odfNameSpace defined, ignoring";
        } else {
            foreach(const QString & elementName, factory->odfElementNames()) {
                d->factories.insert(QPair<QString, QString>(nameSpace, elementName), factory);

                kDebug(32500) << "Inserting variable factory" << factory->id() << " for"
                << nameSpace << ":" << elementName;
            }
        }
    }

}

KoVariableRegistry *KoVariableRegistry::instance()
{
    KoVariableRegistry *registry = &(singleton->q);
    if (! singleton->initDone) {
        singleton->initDone = true;
        registry->init();
    }
    return registry;
}

KoVariable *KoVariableRegistry::createFromOdf(const KoXmlElement & e, KoShapeLoadingContext & context) const
{
    kDebug(32500) << "Going to check for" << e.namespaceURI() << ":" << e.tagName();

    KoVariable *variable = 0;

    KoVariableFactory *factory = d->factories.value(QPair<QString, QString>(e.namespaceURI(), e.tagName()));

    if (factory) {
        variable = factory->createVariable();
        if (variable) {
            variable->loadOdf(e, context);
        }
    }

    return variable;
}

QList<QAction*> KoVariableRegistry::createInsertVariableActions(KoCanvasBase *host) const
{
    QList<QAction*> answer;
    foreach(KoVariableFactory *factory, values()) {
        foreach(const KoVariableTemplate & templ, factory->templates()) {
            answer.append(new InsertVariableAction(host, factory, templ));
        }
    }
    return answer;
}

