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

#include "KoInlineObjectRegistry.h"

#include <KoPluginLoader.h>

#include <kdebug.h>
#include <kstaticdeleter.h>

void KoInlineObjectRegistry::init() {
kDebug() << "XXXXXXXXXXXXXXX init!\n";
    KoPluginLoader::instance()->load( QString::fromLatin1("KOffice/Text-InlineObject"),
                                      QString::fromLatin1("[X-KoText-Version] == 1"));
}

KoInlineObjectRegistry *KoInlineObjectRegistry::s_instance = 0;
static KStaticDeleter<KoInlineObjectRegistry> staticShapeRegistryDeleter;

KoInlineObjectRegistry* KoInlineObjectRegistry::instance() {
    if(KoInlineObjectRegistry::s_instance == 0) {
        staticShapeRegistryDeleter.setObject(s_instance, new KoInlineObjectRegistry());
        KoInlineObjectRegistry::s_instance->init();
    }
    return KoInlineObjectRegistry::s_instance;
}

#include "KoInlineObjectRegistry.moc"
