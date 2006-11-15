/* This file is part of the KDE project
 * Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
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

#include <kdebug.h>
#include <kstaticdeleter.h>

#include <KoPluginLoader.h>
#include <KoShapeRegistry.h>
#include <KoPathShapeFactory.h>

#include <QString>

KoShapeRegistry::KoShapeRegistry()
{
}

void KoShapeRegistry::init() {
    KoPluginLoader::instance()->load( QString::fromLatin1("KOffice/Flake"),
                                      QString::fromLatin1("[X-Flake-Version] == 1"));
    KoPluginLoader::instance()->load(QString::fromLatin1("KOffice/Shape"),
                                     QString::fromLatin1("[X-Flake-Version] == 1"));

    // Also add our hard-coded basic shape
    add( new KoPathShapeFactory(this, QStringList()) );
}


KoShapeRegistry::~KoShapeRegistry()
{
}

KoShapeRegistry *KoShapeRegistry::m_singleton = 0;
static KStaticDeleter<KoShapeRegistry> staticShapeRegistryDeleter;

KoShapeRegistry* KoShapeRegistry::instance()
{
    if(KoShapeRegistry::m_singleton == 0)
    {
        staticShapeRegistryDeleter.setObject(m_singleton, new KoShapeRegistry());
        KoShapeRegistry::m_singleton->init();
    }
    return KoShapeRegistry::m_singleton;
}

#include "KoShapeRegistry.moc"
