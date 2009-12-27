/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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
#include "KoInlineObjectFactory.h"
#include "InsertVariableAction_p.h"

#include <KoCanvasBase.h>
#include <KoPluginLoader.h>

#include <KDebug>
#include <KGlobal>

void KoInlineObjectRegistry::init()
{
    KoPluginLoader::PluginsConfig config;
    config.whiteList = "TextInlinePlugins";
    config.blacklist = "TextInlinePluginsDisabled";
    config.group = "koffice";
    KoPluginLoader::instance()->load(QString::fromLatin1("KOffice/Text-InlineObject"),
                                     QString::fromLatin1("[X-KoText-MinVersion] <= 0"), config);
}

KoInlineObjectRegistry* KoInlineObjectRegistry::instance()
{
    K_GLOBAL_STATIC(KoInlineObjectRegistry, s_instance)
    if (!s_instance.exists()) {
        s_instance->init();
    }
    return s_instance;
}

QList<QAction*> KoInlineObjectRegistry::createInsertVariableActions(KoCanvasBase *host) const
{
    QList<QAction*> answer;
    foreach(const QString & key, keys()) {
        KoInlineObjectFactory *factory = value(key);
        if (factory->type() == KoInlineObjectFactory::TextVariable) {
            foreach(const KoInlineObjectTemplate & templ, factory->templates()) {
                //answer.append(new InsertVariableAction(host, factory, templ));
            }
        }
    }
    return answer;
}

#include <KoInlineObjectRegistry.moc>
