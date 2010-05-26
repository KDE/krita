/* This file is part of the KDE project
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
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

#include "KoInlineObjectRegistry.h"
#include "KoInlineObjectFactoryBase.h"
#include "InsertVariableAction_p.h"

#include <KoCanvasBase.h>
#include <KoInlineObject.h>
#include <KoXmlReader.h>
#include <KoPluginLoader.h>

#include <KDebug>
#include <KGlobal>

class KoInlineObjectRegistry::Private
{
public:
    void insertFactory(KoInlineObjectFactoryBase *factory);
    void init(KoInlineObjectRegistry *q);

    QHash<QPair<QString, QString>, KoInlineObjectFactoryBase *> factories;
};

void KoInlineObjectRegistry::Private::init(KoInlineObjectRegistry *q)
{
    KoPluginLoader::PluginsConfig config;
    config.whiteList = "TextInlinePlugins";
    config.blacklist = "TextInlinePluginsDisabled";
    config.group = "koffice";
    KoPluginLoader::instance()->load(QString::fromLatin1("KOffice/Text-InlineObject"),
                                     QString::fromLatin1("[X-KoText-MinVersion] <= 0"), config);

    foreach (KoInlineObjectFactoryBase *factory, q->values()) {
        QString nameSpace = factory->odfNameSpace();
        if (nameSpace.isEmpty() || factory->odfElementNames().isEmpty()) {
            kDebug(32500) << "Variable factory" << factory->id() << " does not have odfNameSpace defined, ignoring";
        } else {
            foreach (const QString &elementName, factory->odfElementNames()) {
                factories.insert(QPair<QString, QString>(nameSpace, elementName), factory);

                kDebug(32500) << "Inserting variable factory" << factory->id() << " for"
                    << nameSpace << ":" << elementName;
            }
        }
    }
}

KoInlineObjectRegistry* KoInlineObjectRegistry::instance()
{
    K_GLOBAL_STATIC(KoInlineObjectRegistry, s_instance)
    if (!s_instance.exists()) {
        s_instance->d->init(s_instance);
    }
    return s_instance;
}

QList<QAction*> KoInlineObjectRegistry::createInsertVariableActions(KoCanvasBase *host) const
{
    QList<QAction*> answer;
    foreach(const QString & key, keys()) {
        KoInlineObjectFactoryBase *factory = value(key);
        if (factory->type() == KoInlineObjectFactoryBase::TextVariable) {
            foreach (const KoInlineObjectTemplate &templ, factory->templates()) {
                answer.append(new InsertVariableAction(host, factory, templ));
            }
        }
    }
    return answer;
}

KoInlineObject *KoInlineObjectRegistry::createFromOdf(const KoXmlElement &element, KoShapeLoadingContext &context) const
{
    kDebug(32500) << "Going to check for" << element.namespaceURI() << ":" << element.tagName();

    KoInlineObjectFactoryBase *factory = d->factories.value(
            QPair<QString, QString>(element.namespaceURI(), element.tagName()));
    if (factory == 0)
        return 0;

    KoInlineObject *object = factory->createInlineObject(0);
    if (object) {
        object->loadOdf(element, context);
    }

    return object;
}

KoInlineObjectRegistry::~KoInlineObjectRegistry()
{
    delete d;
}

KoInlineObjectRegistry::KoInlineObjectRegistry()
        : d(new Private())
{
}

#include <KoInlineObjectRegistry.moc>
