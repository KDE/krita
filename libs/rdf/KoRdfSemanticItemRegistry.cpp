/* This file is part of the Calligra project, made with-in the KDE community

   Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>
   Copyright (C) 2013 Friedrich W. H. Kossebau <kossebau@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "KoRdfSemanticItemRegistry.h"

#include <KoPluginLoader.h>

#include <kdebug.h>
#include <kglobal.h>

class Q_DECL_HIDDEN KoRdfSemanticItemRegistry::Private
{
public:
    ~Private();
    void init();
};


KoRdfSemanticItemRegistry::Private::~Private()
{
}

void KoRdfSemanticItemRegistry::Private::init()
{
    KoPluginLoader::PluginsConfig config;
    config.whiteList = "SemanticItemPlugins";
    config.blacklist = "SemanticItemPluginsDisabled";
    config.group = "calligra";
    KoPluginLoader::instance()->load(QString::fromLatin1("Calligra/SemanticItem"),
                                     QString::fromLatin1("[X-Calligra-PluginVersion] == 28"), config);
}

KoRdfSemanticItemRegistry* KoRdfSemanticItemRegistry::instance()
{
    K_GLOBAL_STATIC(KoRdfSemanticItemRegistry, s_instance)
    if (!s_instance.exists()) {
        s_instance->d->init();
    }
    return s_instance;
}

QStringList KoRdfSemanticItemRegistry::classNames() const
{
    return keys();
}

QString KoRdfSemanticItemRegistry::classDisplayName(const QString& className) const
{
    const KoRdfSemanticItemFactoryBase *factory = value(className);
    return factory ? factory->classDisplayName() : QString();
}


hKoRdfSemanticItem KoRdfSemanticItemRegistry::createSemanticItem(const QString &semanticClass, const KoDocumentRdf *docRdf, QObject *parent) const
{
    KoRdfSemanticItemFactoryBase *factory = value(semanticClass);
    if (factory) {
        return factory->createSemanticItem(docRdf, parent);
    }
    return hKoRdfSemanticItem(0);
}

hKoRdfSemanticItem KoRdfSemanticItemRegistry::createSemanticItemFromMimeData(const QMimeData *mimeData, KoCanvasBase *host, const KoDocumentRdf *docRdf, QObject *parent) const
{
    foreach (const QString &key, keys()) {
        KoRdfSemanticItemFactoryBase *factory = value(key);
        if (factory->canCreateSemanticItemFromMimeData(mimeData)) {
            return factory->createSemanticItemFromMimeData(mimeData, host, docRdf, parent);
        }
    }
    return hKoRdfSemanticItem(0);
}

bool KoRdfSemanticItemRegistry::canCreateSemanticItemFromMimeData(const QMimeData *mimeData) const
{
    foreach (const QString &key, keys()) {
        KoRdfSemanticItemFactoryBase *factory = value(key);
        if (factory->canCreateSemanticItemFromMimeData(mimeData)) {
            return true;
        }
    }
    return false;
}

void KoRdfSemanticItemRegistry::updateSemanticItems(QList<hKoRdfSemanticItem> &semanticItems, const KoDocumentRdf *docRdf, const QString &className, QSharedPointer<Soprano::Model> m) const
{
    KoRdfSemanticItemFactoryBase *factory = value(className);
    if (factory) {
        factory->updateSemanticItems(semanticItems, docRdf, m);
    }
}

KoRdfSemanticItemRegistry::~KoRdfSemanticItemRegistry()
{
    delete d;
}

KoRdfSemanticItemRegistry::KoRdfSemanticItemRegistry()
  : d(new Private())
{
}
